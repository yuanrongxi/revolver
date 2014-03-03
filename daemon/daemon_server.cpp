#include "base_reactor_instance.h"
#include "core_daemon_msg.h"
#include "core_packet.h"
#include "core_connection.h"
#include "daemon_server.h"
#include "core_local_info.h"
#include "core_connection_manager.h"
#include "core_event_message.h"
#include "daemon_log.h"
#include "daemon_frame.h"
#include "base_timer_value.h"
#include "core_server_type.h"
#include "base64.h"

#define MIN_BIND_PORT 2500
#define MAX_BIND_PORT 500
#define HEARTBEAT_DELAY 10000

#define DAEMON_DC_PORT		2200

BEGIN_MSGCALL_MAP(CDaemonServer)
	ON_MSG_CON_INT(DAEMON_REGISTER_REQ, &CDaemonServer::on_register_req)
	ON_MSG_ADDR_INT(DAEMON_STUN_PING, &CDaemonServer::on_daemon_ping)
	ON_MSG_CON_INT(TCP_CLOSE_EVENT, &CDaemonServer::on_disconnect)
END_MSGCALL_MAP()

CDaemonServer::CDaemonServer()
{
	time_id_	= 0;
	dc_id_		= 0;
}

CDaemonServer::~CDaemonServer()
{
	cancel_timer();
	clear_timer_events();
}

//类似STUN的消息，可以判断服务器的使用的外网地址，在core框架中可以选择性的开启
int32_t CDaemonServer::on_daemon_ping(CBasePacket *packet, uint32_t sid, const Inet_Addr &remote_addr)
{
	Daemon_Stun_Ping *ping = (Daemon_Stun_Ping *)packet;
	
	INIT_CORE_REQUEST(msg, DAEMON_STUN_PONG);

	Daemon_Stun_Pong pong;
	pong.wan_addr_ = remote_addr;
	msg.set_body(pong);

	CONN_MANAGER()->send_udp(msg, remote_addr);

	DAEMON_INFO("on daemon ping from " << remote_addr);

	return 0;
}

int32_t CDaemonServer::on_register_req(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	Daemon_Register_Req* req = (Daemon_Register_Req *)packet;

	DAEMON_INFO("register req = " << *req);
	if(req->stype_ <= eDaemon_Server)
	{
		DAEMON_WARNING("client not create server!!!");
		return 0;
	}

	Inet_Addr remote_addr = connection->get_remote_addr();

	ServerElementMap::iterator it = element_map_.find(req->sid_);
	if(it != element_map_.end() && !it->second.is_null() 
		&& (it->second.tel_addr_.get_ip() == remote_addr.get_ip() || it->second.cnc_addr_.get_ip() == remote_addr.get_ip()))
	{
		DAEMON_WARNING("error sid!! sid = " << req->sid_);
		return 0;
	}

	if(req->cnc_addr_.get_port() != 0 || req->tel_addr_.get_port() != 0) //DAEMON关闭重连
	{
		CDaemonElement element;

		element.set_server_info(req->stype_, req->sid_, req->net_type_, req->tel_addr_, req->cnc_addr_);
		element.null_ = true;
		element_map_[req->sid_] = element;
	}

	//调用获取服务单元的物理机器信息，主要获取IP地址信息、地理位置信息、运营商信息等
	if(phy_db_ != NULL)
		phy_db_->get_physical_server(sid, req->stype_, remote_addr.ip_to_string(), connection);

	return 0;
}

int32_t CDaemonServer::on_node_state(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	DAEMON_INFO("recv node state from sid = " << sid);

	Daemon_Node_State* req = (Daemon_Node_State *)packet;

	ServerElementMap::iterator it = element_map_.find(sid);
	if(it != element_map_.end() && dc_id_ > 0)
	{
		NodeStateJson json;
		json.sid = sid;
		json.info = Base64::encode(req->node_state_);

		string json_param;
		json.Serialize(json_param);

		//TODO:实时处理服务单元的报告信息，可以存在后端的数据库中，以便分析
	}
	return 0;
}

void CDaemonServer::on_physical_server_event(const ServerInfoJson& server_json, CConnection* connection)
{
	if(connection != NULL)
	{
		process_server_info(server_json, connection);
	}
	else
	{
		DAEMON_WARNING("on_physical_server_event, conn = NULL!!!");
	}
}

void CDaemonServer::process_server_info(const ServerInfoJson& json_obj, CConnection* connection)
{
	//结果校验
	if(json_obj.result != 0)
	{
		DAEMON_WARNING("query server info failed! stype = " << GetServerName(json_obj.type) << ", peer addr = " << connection->get_remote_addr());
		element_map_.erase(json_obj.sid);

		return;
	}

	Inet_Addr tel_addr, cnc_addr;
	
	ServerElementMap::iterator it = element_map_.find(json_obj.sid);
	if(it != element_map_.end() && it->second.is_null() && json_obj.type == it->second.server_type_) //判断是否是重连
	{
		it->second.null_ = false;

		it->second.tel_addr_.set_ip(json_obj.ip_ct);
		it->second.cnc_addr_.set_ip(json_obj.ip_un);
		it->second.net_type_ = json_obj.net_type;
		it->second.server_type_ = json_obj.type;
		
		tel_addr = it->second.tel_addr_;
		cnc_addr = it->second.cnc_addr_;
	}
	else //刚启动的SERVER
	{
		CDaemonElement element;
		uint16_t port = get_port(json_obj.type);

		tel_addr.set_ip(json_obj.ip_ct);
		tel_addr.set_port(port);
		cnc_addr.set_ip(json_obj.ip_un);
		cnc_addr.set_port(port);

		element.set_server_info(json_obj.type, json_obj.sid, json_obj.net_type, tel_addr, cnc_addr);
		element_map_[json_obj.sid] = element;
	}

	//建立节点关联关系
	CONN_MANAGER()->add_server(json_obj.sid, json_obj.type, json_obj.net_type, tel_addr, cnc_addr);
	connection->set_server_id(json_obj.sid);
	CONN_MANAGER()->on_add_connection(connection);
	//发送响应
	send_register_res(json_obj.type, json_obj.sid, json_obj.net_type, tel_addr, cnc_addr, connection);
	//发送通告
	for(ServerElementMap::iterator eit = element_map_.begin(); eit != element_map_.end(); ++ eit)
	{
		if(json_obj.sid != eit->second.server_id_ && !eit->second.is_null()) //只发除自己以外激活的服务器
		{
			eit->second.start_notify(json_obj.sid, json_obj.type, json_obj.net_type, tel_addr, cnc_addr);
		}
	}
	
	DAEMON_INFO("on add server, sid = " << json_obj.sid << ", stype = " <<GetServerName(json_obj.type) << ", net type = " 
		<< json_obj.net_type << ", tel_addr = " << tel_addr << ",cnc_addr = " << cnc_addr);

}

int32_t CDaemonServer::on_disconnect(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	DAEMON_WARNING("daemon client disconnected! sid = " << sid);
	//通知数据读取接口取消本连接正在获取的过程
	if(phy_db_ != NULL)
		phy_db_->on_connection_disconnected(connection);


	ServerElementMap::iterator it = element_map_.find(sid);
	if(it == element_map_.end())
	{
		DAEMON_WARNING("error sid!! sid = " << sid);
		return 0;
	}

	//发送通告
	for(ServerElementMap::iterator eit = element_map_.begin(); eit != element_map_.end(); ++ eit)
	{
		if(it->second.server_id_ != eit->second.server_id_ && !eit->second.is_null()) //只发除自己以外激活的服务器
		{
			eit->second.stop_notify(it->second.server_id_, it->second.server_type_);
		}
	}

	it->second.clear_info();

	DAEMON_INFO("del server, server id = " << sid << ", server type = " << GetServerName(it->second.server_type_) 
		<< ", net_type = " << it->second.net_type_ << ", tel_addr = " << it->second.get_tel_addr() << ", cnc_addr = " << it->second.get_cnc_addr());

	return 0;
}

void CDaemonServer::send_register_res(uint8_t type, uint32_t sid, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr, CConnection* connection)
{
	INIT_CORE_REQUEST(packet, DAEMON_REGISTER_RES);

	Daemon_Register_Res res;
	res.stype_ = type;
	res.sid_ = sid;
	res.net_type_ = net_type;
	res.tel_addr_ = tel_addr;
	res.cnc_addr_ = cnc_addr;
	
	packet.set_body(res);

	DAEMON_INFO("send register res, sid = " << sid << ", stype = " << GetServerName(type) << ", net_type = "<< net_type << ", tel_addr = " << tel_addr << ", cnc_addr = " << cnc_addr);

	connection->send(packet);
}

uint32_t CDaemonServer::set_timer()
{
	if(time_id_ <= 0)
		time_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, HEARTBEAT_DELAY);

	return time_id_;
}

void CDaemonServer::cancel_timer()
{
	if(time_id_ > 0)
	{
		const void *act = NULL;
		REACTOR_INSTANCE()->cancel_timer(time_id_, &act);

		time_id_ = 0;
	}
}

int32_t CDaemonServer::handle_timeout(const void *act, uint32_t timer_id)
{
	if(time_id_ == timer_id)
	{
		time_id_ = 0;
		heartbeat();
	}

	return 0;
}

void CDaemonServer::heartbeat()
{
	//daemond的服务心跳
}

void CDaemonServer::init(IPhysicalServer* db)
{
	phy_db_ = db;
	set_timer();
}

void CDaemonServer::destroy()
{

	cancel_timer();
}


uint16_t CDaemonServer::get_port(uint8_t server_type)
{
	uint16_t port = 0;

	switch(server_type)
	{
	//TODO:根据服务类型指定对应绑定端口，
	case eDaemon_Server:
		port = DAEMON_DC_PORT;
		break;

	//默认生成动态端口
	default:
		port = rand() % MAX_BIND_PORT + MIN_BIND_PORT;
	}

	return port;
}


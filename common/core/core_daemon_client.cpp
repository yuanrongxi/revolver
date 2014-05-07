#include "core/core_packet.h"
#include "core/core_event_message.h"
#include "core/core_daemon_msg.h"
#include "core/core_daemon_event.h"
#include "core/core_log_macro.h"
#include "core/core_connection_manager.h"
#include "core/core_local_info.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_message_processor.h"
#include "core/core_daemon_client.h"

//DAEOMON SERVER地址，可以用域名
#define DAEMON_SERVER_CNC_IP	"daemon.revolver.com"
#define DAEMON_SERVER_TEL_IP	"daemon.revolver.com"
#define DAEMON_SERVER_PORT		7600
#define DAEMON_SERVER_ID		1

#define DAEMON_CONNECT_DELAY	12000
#define DAEMON_RECONNECT_TICK	2000

#define DAEMON_UDP_PORT			20000

BASE_NAMESPACE_BEGIN_DECL

//消息映射安装
BEGIN_MSGCALL_MAP(CDaemonClient)
	ON_MSG_ADDR_INT(DAEMON_STUN_PONG, &CDaemonClient::on_daemon_pong)
	ON_MSG_ID_INT(DAEMON_ADD_SERVER, &CDaemonClient::on_add_server)
	ON_MSG_ID_INT(DAEMON_DEL_SERVER, &CDaemonClient::on_del_server)
	ON_MSG_ID_INT(DAEMON_REGISTER_RES, &CDaemonClient::on_register_res)
	ON_MSG_CON_INT(TCP_CONNECT_EVENT, &CDaemonClient::connect_event)
	ON_MSG_CON_INT(TCP_CLOSE_EVENT, &CDaemonClient::disconnect_event)
END_MSGCALL_MAP()

CDaemonClient::CDaemonClient() : timer_id_(0), daemon_event_(NULL),
	daemon_sid_(DAEMON_SERVER_ID), daemon_stype_(eDaemon_Server),
	daemon_tel_addr_(DAEMON_SERVER_TEL_IP, DAEMON_SERVER_PORT),
	daemon_cnc_addr_(DAEMON_SERVER_CNC_IP, DAEMON_SERVER_PORT)
{
	daemon_event_ = NULL;
	config_ = NULL;

	state_ = eDaemon_Idle;
	wan_ = false;
	init_flag_ = false;
	reconnect_count_ = 0;
}

CDaemonClient::~CDaemonClient()
{
	init_flag_ = false;

	focus_.clear();
	daemon_event_ = NULL;
	config_ = NULL;
	reconnect_count_ = 0;

	clear_timer_events();
}

void CDaemonClient::init()
{
	//注册消息处理句柄映射
	INIT_MSG_PROCESSOR1(this);
	//安装消息体映射
	LOAD_MESSAGEMAP_DECL(DAEMON);

	init_flag_ = true;
}

void CDaemonClient::attach_event(IDaemonEvent * daemon_event, IDaemonConfig* config)
{
	daemon_event_ = daemon_event;
	config_ = config;
}

void CDaemonClient::add_focus(uint8_t type)
{
	FocusSet::iterator it = focus_.find(type);
	if(it == focus_.end())
	{
		focus_.insert(FocusSet::value_type(type, type));
	}
}

bool CDaemonClient::is_focus(uint8_t type)
{
	FocusSet::iterator it = focus_.find(type);
	if(it != focus_.end())
		return true;
	else
		return false;
}

void CDaemonClient::start(bool wan)
{
	//未初始化
	if(!init_flag_)
	{
		CORE_WARNING("daemon client not init!!");
		return ;
	}

	if(state_ == eStunning || state_ == eDaemon_Connected || 
		state_ == eDaemon_Connecting || state_ == eDaemon_Finished)
	{
		return ;
	}

	//读取配置，主要是SERVER ID
	if(config_ != NULL)
	{
		config_->read();
	}

	wan_ = wan;

	CONN_MANAGER()->add_server(daemon_sid_, daemon_stype_, 0x00ff, daemon_tel_addr_, daemon_cnc_addr_);

	cancel_timer();

	if(wan)
	{
		if(!daemon_udp_.is_open())
		{
			Inet_Addr daemon_udp_addr(INADDR_ANY, DAEMON_UDP_PORT + rand() % DAEMON_UDP_PORT);
			daemon_udp_.open(daemon_udp_addr);

			daemon_udp_.attach(MSG_PROCESSOR());
		}

		//发送一个STUN探测报文探测自己的WAN IP
		send_daemon_stun();
	}
	else
	{
		star_daemon_tcp();
	}
}

void CDaemonClient::send_daemon_stun()
{
	INIT_CORE_REQUEST(packet, DAEMON_STUN_PING);

	Daemon_Stun_Ping ping;
	ping.optype_ = 0;
	packet.set_body(ping);

	daemon_udp_.send(packet, daemon_tel_addr_);

	timer_id_ = set_timer(DAEMON_RECONNECT_TICK);

	state_ = eStunning;

	CORE_INFO("Daemon client stunning..., daemon addr = " << daemon_tel_addr_);
}

void CDaemonClient::star_daemon_tcp()
{
	INIT_CORE_REQUEST(packet, DAEMON_REGISTER_REQ);

	Daemon_Register_Req register_req;
	register_req.stype_ = SERVER_TYPE;
	register_req.sid_ = SERVER_ID;
	register_req.net_type_ = SERVER_NET_TYPE;
	register_req.tel_addr_ = TEL_IPADDR;
	register_req.cnc_addr_ = CNC_IPADDR;
	packet.set_body(register_req);

	SendDispathByID(packet, daemon_sid_);
	 
	timer_id_ = set_timer(DAEMON_CONNECT_DELAY);

	state_ = eDaemon_Connecting;

	CORE_INFO("start Dameon client, daemon register = " << register_req);
}

void CDaemonClient::stop()
{
	state_ = eDaemon_Finished;

	daemon_udp_.close();

	cancel_timer();
}

int32_t CDaemonClient::handle_timeout(const void *act, uint32_t timer_id)
{
	if(reconnect_count_ > 1000) //持续4小时左右
	{
		CORE_WARNING("daemon client start timeout!");
		return 0;
	}

	timer_id_ = 0;

	if(state_ != eDaemon_Connected)
	{
		CORE_WARNING("Timeout!!! state = " << (uint16_t) state_);
		state_ = eDaemon_Disconnect;

		start(wan_);
		reconnect_count_ ++;
	}

	return 0;
}

uint32_t CDaemonClient::set_timer(uint32_t delay)
{
	return REACTOR_INSTANCE()->set_timer(this, NULL, delay);
}

void CDaemonClient::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);

		timer_id_ = 0;
	}
}

int32_t CDaemonClient::on_daemon_pong(CBasePacket* packet, uint32_t sid, const Inet_Addr& remote_addr)
{
	if(state_ == eStunning)
	{
		Daemon_Stun_Pong* pong = (Daemon_Stun_Pong *)packet;
		CORE_INFO("on daemon pong sid = " << sid <<", wan addr = " <<  pong->wan_addr_ << ", daemon addr = " << remote_addr);

		TEL_IPADDR = pong->wan_addr_;
		TEL_IPADDR.set_port(SERVER_PORT);

		cancel_timer();

		//启动TCP
		star_daemon_tcp();
	}

	return 0;
}

int32_t CDaemonClient::on_register_res(CBasePacket* packet, uint32_t sid)
{
	state_ = eDaemon_Connected;
	cancel_timer();

	Daemon_Register_Res *res = (Daemon_Register_Res *)packet;
	if(daemon_event_ != NULL)
	{
		CORE_INFO("on register response, local sid = " << res->sid_ << ", stype = " 
			<< GetServerName(res->stype_) << ", tel addr = " <<  res->tel_addr_ << ", cnc addr = " << res->cnc_addr_);
		
		SERVER_NET_TYPE = res->net_type_;

		daemon_event_->on_register(res->sid_, res->stype_, res->net_type_, res->tel_addr_, res->cnc_addr_);

		//刷新保存配置
		if(config_ != NULL)
		{
			config_->write();
		}
	}

	reconnect_count_ = 0;

	return 0;
}

int32_t CDaemonClient::on_add_server(CBasePacket* packet, uint32_t sid)
{
	Daemon_Add_Server *notify = (Daemon_Add_Server*)packet;
	if(daemon_event_ != NULL)
	{
		if(is_focus(notify->stype_))
		{
			CONN_MANAGER()->add_server(notify->sid_, notify->stype_, notify->net_type_, notify->tel_addr_, notify->cnc_addr_);
			daemon_event_->on_add_server(notify->sid_, notify->stype_, notify->net_type_, notify->tel_addr_, notify->cnc_addr_);

			CORE_INFO("on_add_server, local sid = " << notify->sid_ << ", stype = " 
				<< GetServerName(notify->stype_) << ", tel addr = " <<  notify->tel_addr_
				<< ", cnc addr = " << notify->cnc_addr_);
		}
 	}

	return 0;
}

int32_t CDaemonClient::on_del_server(CBasePacket *packet, uint32_t sid)
{
	Daemon_Del_Server* notify = (Daemon_Del_Server *)packet;
	if(daemon_event_ != NULL)
	{
		if(is_focus(notify->stype_))
		{
			CONN_MANAGER()->del_server(notify->sid_, notify->stype_);
			daemon_event_->on_del_server(notify->sid_, notify->stype_);
			CORE_INFO("on_del_server, sid = " << notify->sid_ << ", stype = " << GetServerName(notify->stype_));
		}
	}
	return 0;
}

int32_t CDaemonClient::connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	if(connection->get_server_id() == daemon_sid_ && connection->get_server_type() == daemon_stype_)
	{
		CORE_INFO("connected Daemon Server!");

		return 0;
	}

	return -1;
}

int32_t CDaemonClient::disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	if(connection->get_server_id() == daemon_sid_ && connection->get_server_type() == daemon_stype_)
	{
		state_ = eDaemon_Disconnect;
		cancel_timer();
		timer_id_ = set_timer(DAEMON_RECONNECT_TICK); //2秒后再次发起连接

		CORE_WARNING("disconnect Daemon Server!");

		return 0;
	}

	return -1;
}

int32_t CDaemonClient::send_node_state(const string& node_info)
{
	int32_t ret= -1;
	if(state_ == eDaemon_Connected)
	{
		INIT_CORE_REQUEST(packet, DAEMON_NODE_STATE);

		Daemon_Node_State req;
		req.node_state_ = node_info;
		packet.set_body(req);

		SendDispathByID(packet, daemon_sid_);

		ret = 0;
	}

	return ret;
}

BASE_NAMESPACE_END_DECL

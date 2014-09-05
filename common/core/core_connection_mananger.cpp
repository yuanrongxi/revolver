#include "core/core_packet.h"
#include "revolver/base_guard.h"
#include "core/core_log_macro.h"
#include "core/core_connection_manager.h"
#include "core/core_udp_handler.h"
#include "core/core_local_info.h"
#include "core/core_tcp_disconnect_notify.h"

BASE_NAMESPACE_BEGIN_DECL

Connection_Pool	CONNECTION_POOL;

Connection_Manager::Connection_Manager()
{
	udp_handler_ = NULL;
	notify_ = NULL;
}

Connection_Manager::~Connection_Manager()
{
	nodes_.clear();
}

void Connection_Manager::attach_server_notify(ICoreServerNotify *notify)
{
	notify_ = notify;
}

void Connection_Manager::attach_udp(CoreUDPHandler* handler)
{
	udp_handler_ = handler;
}

void Connection_Manager::close_all()
{
	vector<CConnection *> conn_list;
	for(Server_Node_Map::iterator it = nodes_.begin(); it != nodes_.end(); it ++)
	{
		if(it->second.conn != NULL)
		{
			conn_list.push_back(it->second.conn);
		}
	}

	for(size_t i = 0; i < conn_list.size(); i ++)
	{
		conn_list[i]->extern_close();
	}
}

void Connection_Manager::add_server(uint32_t server_id, uint8_t server_type, uint16_t net_type,
									const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr)
{
	CORE_INFO("add server, sid = " << server_id << ", server_type = " << GetServerName(server_type));

	Server_Node_Map::iterator it = nodes_.find(server_id);
	if(it == nodes_.end())
	{
		Server_Node_t node;
		node.server_id = server_id;
		node.server_type = server_type;
		node.tel_addr = tel_addr;
		node.net_type = net_type;
		node.cnc_addr = cnc_addr;
		node.daemon_del_ = false;
		
		nodes_[server_id] = node;
	}
	else
	{
		it->second.server_id = server_id;
		it->second.server_type = server_type;
		it->second.net_type = net_type;
		it->second.tel_addr = tel_addr;
		it->second.cnc_addr = cnc_addr;
		it->second.daemon_del_ = false;
	}
}

void Connection_Manager::del_server(uint32_t server_id, uint8_t server_type)
{
	CORE_INFO("delete server, sid = " << server_id << ", server_type = " << GetServerName(server_type));
	Server_Node_Map::iterator it = nodes_.find(server_id);
	if(it != nodes_.end()) //只删除连接断开的
	{
		if(it->second.conn == NULL)
		{
			//可以做个上层接口，通知节点真正的死去
			if(notify_ != NULL)
			{
				notify_->on_server_dead(it->second.server_id, it->second.server_type);
			}

			nodes_.erase(it);
		}
		else
		{
			it->second.daemon_del_ = true;
		}
	}
}

bool Connection_Manager::on_add_connection(CConnection* conn)
{
	CORE_INFO("on add connection, sid = " << conn->get_server_id() << ", server type = " << GetServerName(conn->get_server_type()));
	if(conn->get_server_id() == 0)
	{
		return false;
	}

	if(conn->get_server_type() >= eDaemon_Server)
	{
		Server_Node_Map::iterator it = nodes_.find(conn->get_server_id());
		if(it != nodes_.end())
		{
			it->second.conn = conn;
			it->second.index = conn->get_index();
		
			for(StreamList::iterator list_it = it->second.strms.begin(); list_it != it->second.strms.end(); ++list_it)
			{
				if(!list_it->empty())
					conn->send(*list_it);
			}

			it->second.strms.clear();
		}
		else
		{
			Server_Node_t node;
			node.server_id = conn->get_server_id();
			node.server_type = conn->get_server_type();
			node.daemon_del_ = true;

			nodes_[conn->get_server_id()] = node;
			nodes_[conn->get_server_id()].index = conn->get_index();
			nodes_[conn->get_server_id()].conn = conn;
		}

		CORE_WARNING("on add connection, sid = " << conn->get_server_id() << ", stype = " << GetServerName(conn->get_server_type()));
		return true;
	}

	return false;
}

void Connection_Manager::on_del_connection(CConnection* conn)
{
	if(conn->get_server_type() >= eDaemon_Server)
	{

		//BASE_GUARD(BaseThreadMutex, cf_mon, mutex_);
		CORE_WARNING("on del connection, sid = " << conn->get_server_id() << ", stype = " << GetServerName(conn->get_server_type()));

		Server_Node_Map::iterator it = nodes_.find(conn->get_server_id());
		if(it != nodes_.end())
		{
			it->second.conn = NULL;
			it->second.index = 0;

			it->second.strms.clear();

			if(it->second.daemon_del_)
			{
				if(notify_ != NULL)
				{
					notify_->on_server_dead(it->second.server_id, it->second.server_type);
				}

				nodes_.erase(it);
			}
		}
	}
}

void Connection_Manager::get_address_pair(const Server_Node_t& node, Inet_Addr& src_addr, Inet_Addr& dst_addr)
{
	uint16_t local_type = (SERVER_NET_TYPE & 0x00ff);
	uint16_t remote_type = (node.net_type & 0x00ff);

	if(local_type == 0x00ff) //本地是双线,
	{
		if(remote_type == 0x0001)
		{
			src_addr = CNC_IPADDR;
			dst_addr = node.cnc_addr;
		}
		else
		{
			src_addr = TEL_IPADDR;
			dst_addr = node.tel_addr;
		}
	}
	else if(remote_type == 0x00ff) //远端是双线
	{
		if(local_type == 0x0001)
		{
			dst_addr = node.cnc_addr;
		}
		else
		{
			dst_addr = node.tel_addr;
		}
	}
	else
	{
		src_addr = (local_type == 0x0001) ? CNC_IPADDR : TEL_IPADDR;
		dst_addr = (remote_type == 0x0001) ? node.cnc_addr : node.tel_addr;
	}
}

void Connection_Manager::connecting_by_id(const string& data, uint32_t sid)
{
	Server_Node_Map::iterator it = nodes_.find(sid);
	if(it != nodes_.end())
	{
		CConnection* conn = NULL;
		if(it->second.conn == NULL) //建立一条新的TCP服务器连接 
		{
			conn = CONNECTION_POOL.pop_obj();

			conn->set_index(0);
			conn->set_server_type(it->second.server_type);
			conn->set_server_id(it->second.server_id);			
			
			//选取连接地址，目标地址为电信用电信网卡进行连接，依次类推
			Inet_Addr src_addr, dst_addr;
			get_address_pair(it->second, src_addr, dst_addr);

			src_addr.set_port(0);
			//随机一个端口作为客户机端口
			if(src_addr.get_ip() != INADDR_ANY)
			{
				src_addr.set_port(15000 + rand() % 15000);
			}

			CORE_INFO("connecting " << dst_addr << ", by local addr = " << src_addr << ", dst sid = " << sid);

			if(conn->connect(src_addr, dst_addr) != 0)
			{
				conn->close();
				CONNECTION_POOL.push_obj(conn);
				CORE_DEBUG("push conn = " << this);
				CORE_ERROR("connect " << dst_addr << "failed!!");
				return ;
			}

			it->second.conn = conn;
		}
		else
		{
			conn = it->second.conn;
		}

		if(conn->get_state() != CConnection::CONN_CONNECTED) //处于连接状态，缓冲到LIST中
			it->second.strms.push_back(data);
		else
			conn->send(data);
	}
	else
	{
		CORE_ERROR("sid = " << sid << " error!!");
	}
}

CConnection* Connection_Manager::get_connection(uint32_t sid)
{
	CConnection* ret = NULL;

	Server_Node_Map::iterator it = nodes_.find(sid);
	if(it != nodes_.end() && it->second.conn != NULL 
		&& it->second.conn->get_state() == CConnection::CONN_CONNECTED) //只获取连接完成的CONNECTION,否则返回1个NULL
	{
		ret = it->second.conn;
	}

	return ret;
}

void Connection_Manager::send_dispatch_by_id(CCorePacket& packet, uint32_t sid)
{
	CConnection* conn = get_connection(sid);
	if(conn == NULL)
	{
		GAIN_BINSTREAM(strm);

		string data;
		*strm << packet;
		strm->bin_to_string(data);
		connecting_by_id(data, sid);

		RETURN_BINSTREAM(strm);
	}
	else
	{
		conn->send(packet);
	}
}

void Connection_Manager::send_dispatch_by_id(const string& data, uint32_t sid)
{
	CConnection* conn = get_connection(sid);
	if(conn == NULL)
		connecting_by_id(data, sid);
	else
		conn->send(data);
}

const Inet_Addr& Connection_Manager::get_udp_remote_addr(const Server_Node_t& node)
{
	if(CHECK_CNC_NETTYPE(node.net_type))
		return node.cnc_addr;
	else
		return node.tel_addr;
}

void Connection_Manager::send_dispatch_by_udp(CCorePacket& packet, uint32_t sid)
{
	Server_Node_Map::iterator it = nodes_.find(sid);
	if(it != nodes_.end())
	{
		send_udp(packet, get_udp_remote_addr(it->second));
	}
}

void Connection_Manager::send_dispatch_by_udp(const string& data, uint32_t sid)
{
	Server_Node_Map::iterator it = nodes_.find(sid);
	if(it != nodes_.end())
	{
		send_udp(data, get_udp_remote_addr(it->second));
	}
}

void Connection_Manager::send_tcp(CCorePacket& packet, CConnection* conn)
{
	//报文编码
	if(conn->get_state() == CConnection::CONN_CONNECTED)
	{
		conn->send(packet);
	}
}

void Connection_Manager::send_tcp(const string& data, CConnection* conn)
{
	//报文编码
	if(conn->get_state() == CConnection::CONN_CONNECTED)
	{
		conn->send(data);
	}
}

void Connection_Manager::send_udp(CCorePacket& packet, const Inet_Addr& remote_addr)
{
	//获取已经连接的UDP HANDLER，将报文发送给对应地址
	if(udp_handler_ != NULL)
	{
		udp_handler_->send(packet, remote_addr);
	}
}

void Connection_Manager::send_udp(const string& data, const Inet_Addr& remote_addr)
{
	//获取已经连接的UDP HANDLER，将报文发送给对应地址
	if(udp_handler_ != NULL)
	{
		udp_handler_->send(data, remote_addr);
	}
}

Server_Node_t Connection_Manager::find_server_info(uint32_t sid)
{
	Server_Node_t node;

	Server_Node_Map::iterator it = nodes_.find(sid);
	if(it != nodes_.end())
	{
		node = it->second;
	}

	return node;
}

BASE_NAMESPACE_END_DECL

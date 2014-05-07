/*************************************************************************************
*filename: core_connection_manager.h
*
*to do:		服务器节点管理，客户端连接不体现在这个里面
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_CONNECTION_MANAGER_H
#define __CORE_CONNECTION_MANAGER_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "core/core_connection.h"
#include "revolver/object_pool.h"
#include "revolver/base_thread_mutex.h"
#include "revolver/base_singleton.h"
#include "core/core_server_type.h"
#include "core/core_packet.h"

#include <list>
#include <vector>
#include <map>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL
class CoreUDPHandler;
class ICoreServerNotify;

typedef ObjectMutexPool<CConnection, BaseThreadMutex, CONNECTION_POOL_SIZE>	Connection_Pool;
extern Connection_Pool	CONNECTION_POOL;

//报文在连接过程中的缓冲队列
typedef list<string>		StreamList;

typedef struct Server_Node_t
{
	CConnection*	conn;
	uint32_t		server_id;
	uint8_t			server_type;
	uint16_t		net_type;
	uint32_t		index;
	bool			daemon_del_;	//DAEMON认为死去了的节点
	Inet_Addr		tel_addr;		//电信地址
	Inet_Addr		cnc_addr;		//网通地址
	StreamList		strms;
public:
	Server_Node_t()
	{
		conn = NULL;
		server_id = 0;
		server_type = eClient;
		daemon_del_ = true;
		net_type = 0x0000;
		index = 0;
	};
}Server_Node_t;

typedef map<uint32_t, Server_Node_t> Server_Node_Map;

class Connection_Manager
{
public:
	Connection_Manager();
	~Connection_Manager();
	
	void close_all();
	
	void attach_udp(CoreUDPHandler* handler);
	void attach_server_notify(ICoreServerNotify *notify);

	void add_server(uint32_t server_id, uint8_t server_type, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr);
	void del_server(uint32_t server_id, uint8_t server_type);

	//连接增删事件
	bool on_add_connection(CConnection* conn);
	void on_del_connection(CConnection* conn);

	//通过TCP ID发送数据给其他服务器
	void send_dispatch_by_id(CCorePacket& packet, uint32_t sid);
	void send_dispatch_by_id(const string& data, uint32_t sid);

	//通过UDP ID发送数据给其的服务器
	void send_dispatch_by_udp(CCorePacket& packet, uint32_t sid);
	void send_dispatch_by_udp(const string& data, uint32_t sid);

	//通过UDP Address发送数据给其他节点
	void send_udp(CCorePacket& packet, const Inet_Addr& remote_addr);
	void send_udp(const string& data, const Inet_Addr& remote_addr);

	//通过TCP Connection发送数据给其他节点
	void send_tcp(CCorePacket& packet, CConnection* conn);
	void send_tcp(const string& data, CConnection* conn);
	//获取服务器的信息
	Server_Node_t find_server_info(uint32_t sid);
	void get_address_pair(const Server_Node_t& node, Inet_Addr& src_addr, Inet_Addr& dst_addr);

protected:
	void connecting_by_id(const string& data, uint32_t sid);
	CConnection* get_connection(uint32_t sid);

	const Inet_Addr& get_udp_remote_addr(const Server_Node_t& node);

protected:
	//BaseThreadMutex mutex_;
	Server_Node_Map nodes_;
	CoreUDPHandler* udp_handler_;
	ICoreServerNotify *notify_;
};

#define CREATE_CONN_MANAGER		CSingleton<Connection_Manager>::instance
#define CONN_MANAGER			CSingleton<Connection_Manager>::instance
#define DESTROY_CONN_MANAGER	CSingleton<Connection_Manager>::destroy

#define AttachServerNotify(notify)\
	CONN_MANAGER()->attach_server_notify(notify)\

#define SendDispathByID(packet, id)\
	CONN_MANAGER()->send_dispatch_by_id(packet, id)

#define SendDispathByUDP(packet, id)\
	CONN_MANAGER()->send_dispatch_by_udp(packet, id)

#define SendUDP(packet, addr)\
	CONN_MANAGER()->send_udp(packet, addr)

#define SendTCP(packet, conn)\
	CONN_MANAGER()->send_tcp(packet, conn)

#define GetServerByID(id)\
	CONN_MANAGER()->find_server_info(id)

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/

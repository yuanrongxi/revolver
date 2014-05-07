/*************************************************************************************
*filename:	rudp_interface.h
*
*to do:		定义RUDP的调用接口，实现socket、bind、connect、send、recv等函数接口
			管理
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __RUDP_INTERFACE_H_
#define __RUDP_INTERFACE_H_

#include "revolver/base_typedef.h"
#include "revolver/base_namespace.h"
#include "revolver/base_singleton.h"
#include "revolver/object_pool.h"
#include "revolver/base_inet_addr.h"
#include "revolver/base_event_handler.h"
#include "rudp/rudp_half_connection.h"
#include "rudp/rudp_event_handler.h"

#include <vector>
#include <set>

using namespace std;

#define INVALID_ADAPTER_INDEX	0xff

BASE_NAMESPACE_BEGIN_DECL
class RUDPSocket;
class IRUDPAdapter;

typedef vector<IRUDPAdapter *>		RUDPAdapterArray;


typedef vector<RUDPSocket *>								RUDPSocketArray;
typedef ObjectPool<RUDPSocket, RUDP_SOCKET_POOL_SIZE>		RUDPSocketPool;
typedef	set<int32_t>										RUDPHandleSet;	

class RUDPObject : public CEventHandler
{
public:
	RUDPObject();
	virtual ~RUDPObject();

	void				init();
	void				destroy();
	//应用层接口
	int32_t				create_socket();
	void				close_socket(int32_t rudp_id);
	//强制关闭，不做任何等待
	void				force_close_socket(int32_t rudp_id);
	int32_t				setoption(int32_t rudp_id, int32_t op_type, int32_t op_value);
	int32_t				bind(int32_t rudp_id, const Inet_Addr& local_addr);
	int32_t				connect(int32_t rudp_id, const Inet_Addr& remote_addr);
	int32_t				send(int32_t rudp_id, const uint8_t* data, int32_t data_size);
	int32_t				recv(int32_t rudp_id, uint8_t* data, int32_t data_size);
	int32_t				get_error(int32_t rudp_id);
	int32_t				get_local_addr(int32_t rudp_id, Inet_Addr& addr);
	int32_t				get_peer_addr(int32_t rudp_id, Inet_Addr& addr);

	uint32_t			recv_bandwidth(int32_t rudp_id);
	uint32_t			send_bandwidth(int32_t rudp_id);

	int32_t				send_cache_size(int32_t rudp_id);
	int32_t				recv_cache_size(int32_t rudp_id);

	void				bind_event_handle(int32_t rudp_id, RUDPEventHandler* event_handle);
	void				register_event(int32_t rudp_id, int32_t marks);
public:
	RUDPSocket*			get_socket(int32_t rudp_id);
	void				free_sockets(int32_t rudp_id);
	void				delete_peer_index(int32_t peer_id, const Inet_Addr& );
	//处理来自网络的UDP报文
	void				process(IRUDPAdapter* adapter, BinStream& strm, const Inet_Addr& remote_addr);
	//向外发送UDP报文
	void				send_udp(uint8_t index, BinStream& strm, const Inet_Addr& remote_addr);

	void				attach(IRUDPAdapter* adapter);
	void				unattach(IRUDPAdapter* adapter);

	void				attach_listener(RUDPEventHandler* handle){listener_ = handle;};

public:
	int32_t				handle_timeout(const void *act, uint32_t timer_id);

protected:
	void				alloc_sockets();
	void				send_syn2(uint32_t remote_rudp_id, uint8_t result, uint16_t check_sum, const Inet_Addr& remote_addr, IRUDPAdapter* adapter);

	void				set_timer();
	void				cancel_timer();
	void				heartbeat();

	RUDPSocket*			find_by_peer_id(int32_t peer_rudp_id, const Inet_Addr& peer_addr);

protected:
	RUDPSocketArray		socket_array_;
	RUDPSocketPool		socket_pool_;
	RUDPHandleSet		free_socket_ids_;
	RUDPHandleSet		used_socket_ids_;

	//存储半开连接的索引
	RUDPHandleMap		peer_socket_ids_;

	RUDPAdapterArray	adapter_array_;

	RUDPEventHandler*	listener_;

	uint32_t			timer_id_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CREATE_RUDP			CSingleton<RUDPObject>::instance
#define RUDP				CSingleton<RUDPObject>::instance
#define DESTROY_RUDP		CSingleton<RUDPObject>::destroy

#define init_rudp_socket() \
	CREATE_RUDP()

#define destroy_rudp_socket() \
	DESTROY_RUDP()

//封装宏接口
#define rudp_socket()	\
	RUDP()->create_socket()

#define rudp_close(rudp_id) \
	RUDP()->close_socket(rudp_id)

#define rudp_force_close(rudp_id) \
	RUDP()->force_close_socket(rudp_id)

#define rdup_bind(rudp_id, local_addr) \
	RUDP()->bind(rudp_id, local_addr)

#define rudp_connect(rudp_id, remote_addr) \
	RUDP()->connect(rudp_id, remote_addr)

#define rudp_send(rudp_id, data, data_size) \
	RUDP()->send(rudp_id, data, data_size)

#define rudp_recv(rudp_id, data, data_size) \
	RUDP()->recv(rudp_id, data, data_size)

#define rudp_setoption(rudp_id, op_type, op_value) \
	RUDP()->setoption(rudp_id, op_type, op_value)

#define rudp_error(rudp_id) \
	RUDP()->get_error(rudp_id)

#define rudp_recv_bandwidth(rudp_id) \
	RUDP()->recv_bandwidth(rudp_id)

#define rudp_send_bandwidth(rudp_id) \
	RUDP()->send_bandwidth(rudp_id)

#define rudp_send_cache_size(rudp_id) \
	RUDP()->send_cache_size(rudp_id)

#define rudp_recv_cache_size(rudp_id) \
	RUDP()->recv_cache_size(rudp_id)

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


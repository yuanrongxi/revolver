#ifndef __DC_SHORT_CONN_H_
#define __DC_SHORT_CONN_H_

#include "revolver/base_thread_mutex.h"
#include "core/core_connection.h"
#include "revolver/object_pool.h"

BASE_NAMESPACE_BEGIN_DECL

class ShortConnection;

class IShortProcessor
{
public:
	IShortProcessor(){};
	~IShortProcessor(){};

	virtual int32_t		on_short_connected(ShortConnection *conn) = 0;
	virtual int32_t		on_short_disconnected(ShortConnection *conn) = 0;
	virtual int32_t		on_message(CCorePacket &packet, BinStream& istrm, ShortConnection* conn) = 0;
};

class ShortConnection : public CEventHandler
{
public:
	enum 
	{
		SHORT_CONN_IDLE,			//平静状态,有可能是未连接
		SHORT_CONN_CONNECTING,		//连接状态
		SHORT_CONN_CONNECTED,		//连接状态
	};

public:
	ShortConnection();
	~ShortConnection();

	void				set_proc(IShortProcessor *proc) {msg_proc_ = proc;};
	void				reset();
	CSockStream&		get_sock_stream();

	uint16_t			get_state() const {return state_;};
	void				set_state(uint16_t state) {state_ = state;};

	BASE_HANDLER		get_handle() const;
	void				set_handle(BASE_HANDLER handle);

	uint32_t			get_conn_id() const {return conn_id_;};
	void				set_conn_id(uint32_t conn_id) {conn_id_ = conn_id;};

	void				set_remote_addr(const Inet_Addr& remote_addr){remote_addr_ = remote_addr;};
	const Inet_Addr&	get_remote_addr() const {return remote_addr_;};

	int32_t				connect(const Inet_Addr& remote_addr);
	int32_t				connect(const Inet_Addr& src_addr, const Inet_Addr& dst_addr);
	void				close();
	void				extern_close();
	int32_t				send(CCorePacket &packet);

public:
	//事件接口
	int32_t				handle_input(BASE_HANDLER handle);
	int32_t				handle_output(BASE_HANDLER handle);
	int32_t				handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	int32_t				handle_exception(BASE_HANDLER handle);
	int32_t				handle_timeout(const void *act, uint32_t timer_id);

private:
	uint32_t			set_timer(uint32_t timer_type, uint32_t delay = 60000); 
	void				cancel_timer();
	void				release_timer_act(const void* act);

	int32_t				heartbeat();
	void				check_connecting_state();
	int32_t				process(CCorePacket &packet, BinStream& istrm);
	void				send_ping();

protected:
	uint16_t			state_;
	uint32_t			timer_id_;

	CSockStream			sock_stream_;

	SBuffer				sbuffer_;			//发送BUFFER
	RBuffer				rbuffer_;			//接收BUFFER

	BinStream			istrm_;

	uint32_t			timer_count_;		//定时器计数
	Inet_Addr			remote_addr_;
	IShortProcessor*	msg_proc_;

	uint32_t			conn_id_;
};

typedef ObjectMutexPool<ShortConnection, BaseThreadMutex, CONNECTION_POOL_SIZE>	ShortConn_Pool;
extern ShortConn_Pool	SHORTCONN_POOL;

BASE_NAMESPACE_END_DECL

#endif

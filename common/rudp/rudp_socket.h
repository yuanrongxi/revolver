/*************************************************************************************
*filename:	rudp_socket.h
*
*to do:		定义RUDP SOCKET对象，实现RUDP的连接、状态转换、链路异常检测等等
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_SOCKET_H_
#define __RUDP_SOCKET_H_

#include "rudp/rudp_ccc.h"
#include "rudp/rudp_recv_buffer.h"
#include "rudp/rudp_send_buffer.h"
#include "revolver/base_inet_addr.h"
#include "revolver/base_event_handler.h"

BASE_NAMESPACE_BEGIN_DECL
class RUDPEventHandler;

#define RUDP_VERSION	1
enum RUDPOptionType
{
	//心跳的时间周期，以毫秒为单位
	RUDP_KEEPLIVE			= 1,
	//NAGLE算法开关
	RUDP_NAGLE,
	//接受缓冲区大小
	RUDP_RECV_BUFF_SIZE,
	//发送缓冲区大小
	RUDP_SEND_BUFF_SIZE,
	//RUDP连接超时断开的KEEPLIVE周期数
	RUDP_TIMEOUT_COUNT,
};

enum RUDPSessionState
{
	RUDP_IDLE				= 1,
	RUDP_CONNECTING			= 2,
	RUDP_CONNECTED			= 3,
	RUDP_FIN_STATE			= 4,
	RUDP_FIN2_STATE			= 5,
	RUDP_CLOSE				= 6

};

enum RUDPErrorCode
{
	RUDP_SUCCESS			= 0,
	RUDP_BIND_FAIL			= 1,
	RUDP_CONNECT_FAIL		= 2,
	RUDP_SEND_ERROR			= 3,
	RUDP_SEND_EAGIN			= 4,
};

class RUDPSocket : public IRUDPNetChannel,
				   public CEventHandler
{
public:
	RUDPSocket();
	virtual ~RUDPSocket();

	//属性方法
public:
	int32_t				get_rudp_id() const {return rudp_id_;};
	void				set_rudp_id(int32_t rudp_id) {rudp_id_ = rudp_id;};
	
	int32_t				get_remote_rudp_id() const {return remote_rudp_id_;};
	void				set_remote_rudp_id(int32_t rudp_id){remote_rudp_id_ = rudp_id;};

	uint8_t				get_local_index() const {return local_index_;};
	void				set_local_index(uint8_t index) {local_index_ = index;};

	uint32_t			get_check_sum() const {return check_sum_;};
	void				set_check_sum(uint16_t sum) {check_sum_ = sum;};

	const Inet_Addr&	get_peer_addr() const {return remote_addr_;};

	RUDPEventHandler*	get_event_handler() {return event_handler_;};
	void				set_event_handler(RUDPEventHandler* event_handler) {event_handler_ = event_handler;};

	uint16_t			get_state() const {return state_;};
	void				set_state(uint16_t state);

	int32_t				get_error() const {return error_code_;};
	
	//检查是否可读写
	void				register_event(uint32_t marks);

	uint32_t			recv_bandwidth() {return recv_buffer_.get_bandwidth();};
	uint32_t			send_bandwidth() {return send_buffer_.get_bandwidth();};

	int32_t				get_send_cache_size() {return send_buffer_.get_buffer_data_size();};
	int32_t				get_recv_cache_size() {return recv_buffer_.get_buffer_data_size();};
	//外部控制方法
public:
	int32_t				open(int32_t rudp_id_);
	void				close();
	void				force_close();
	void				reset();

	int32_t				setoption(int32_t op_type, int32_t op_value);
	int32_t				bind(uint8_t index, uint8_t title);
	int32_t				connect(const Inet_Addr& remote_addr);

	int32_t				send(const uint8_t* data, int32_t data_size);
	int32_t				recv(uint8_t* data, int32_t data_size);

public:
	//BUFFER报文发送接口
	virtual void		send_ack(uint64_t ack_seq_id);
	virtual void		send_nack(uint64_t base_seq_id, const LossIDArray& ids);
	virtual	void		send_data(uint64_t ack_seq_id, uint64_t cur_seq_id, const uint8_t* data, uint16_t data_size, uint64_t now_ts);

public:
	//BUFFER状态接口,BUFFER可以进行数据发送通告
	virtual void		on_write();
	//buffer可以进行数据读取
	virtual void		on_read();
	//错误通告
	virtual void		on_exception();

public:
	void				heartbeat();
	void				process(uint8_t msg_id, uint16_t check_sum, BinStream& strm, const Inet_Addr& remote_addr);
	//控制消息处理函数
	void				process_syn(RUDPSynPacket& syn, const Inet_Addr& remote_addr);

	int32_t				handle_timeout(const void *act, uint32_t timer_id);		

protected:
	void				set_timer(uint32_t delay);
	void				cancel_timer();

	void				process_syn2(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_syn_ack(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_fin(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_fin2(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_keeplive(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_keeplive_ack(BinStream& strm, const Inet_Addr& remote_addr);
	//数据消息处理函数
	void				process_data(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_data_ack(BinStream& strm, const Inet_Addr& remote_addr);
	void				process_data_nack(BinStream& strm, const Inet_Addr& remote_addr);

	//发送函数
	void				send_syn();
	void				send_syn2(uint8_t result, uint64_t remote_ts);
	void				send_syn_ack(uint8_t result , uint64_t remote_ts);
	void				send_fin();
	void				send_fin2();
	void				send_keeplive(uint64_t now_ts);
	void				send_keeplive_ack(uint64_t ts);

private:
	//连接信息
	int32_t				rudp_id_;
	uint8_t				local_index_;
	uint8_t				local_title_;

	uint16_t			check_sum_;
	//远端地址
	int32_t				remote_rudp_id_;
	Inet_Addr			remote_addr_;
	//缓冲区
	RUDPCCCObject		ccc_;
	RUDPSendBuffer		send_buffer_;
	RUDPRecvBuffer		recv_buffer_;

	uint64_t			heart_ts_;
	//KEEP LIVE周期,每个计数5MS
	uint16_t			keeplive_intnal_;
	uint32_t			keeplive_count_;
	//超时的KEEP LIVE周期数
	uint16_t			timeout_count_;

	//回调接口
	RUDPEventHandler*	event_handler_;
	//RUDP的状态
	uint16_t			state_;
	//当前错误码
	int32_t				error_code_;
	//定时器ID
	uint32_t			timer_id_;

	uint8_t				send_count_;

	BinStream			strm_;
};

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/


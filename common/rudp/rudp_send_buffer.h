/*************************************************************************************
*filename:	rudp_send_buffer.h
*
*to do:		定义RUDP的发送缓冲BUFFER和发送窗口
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_SEND_BUFFER_H_
#define __RUDP_SEND_BUFFER_H_

#include "rudp/rudp_channel_interface.h"
#include "rudp/rudp_segment.h"

#include <map>
#include <list>

BASE_NAMESPACE_BEGIN_DECL

typedef map<uint64_t, RUDPSendSegment*> SendWindowMap;
typedef list<RUDPSendSegment*> SendDataList;

class RUDPCCCObject;

class RUDPSendBuffer
{
public:
	RUDPSendBuffer();
	virtual ~RUDPSendBuffer();
	
	void				reset();
	//发送数据接口
	int32_t				send(const uint8_t* data, int32_t data_size);
	//ACK处理
	void				on_ack(uint64_t ack_seq);
	//NACK处理
	void				on_nack(uint64_t base_seq, const LossIDArray& loss_ids);
	//定时器接口
	void				on_timer(uint64_t now_ts);

	void				check_buffer();

public:
	uint64_t			get_buffer_seq() {return buffer_seq_;};
	//设置NAGLE算法	
	void				set_nagle(bool nagle = true){nagle_ = nagle;};
	bool				get_nagle() const {return nagle_;};
	//设置发送缓冲区的大小
	void				set_buffer_size(int32_t buffer_size){buffer_size_ = buffer_size;};
	int32_t				get_buffer_size() const {return buffer_size_;};

	void				set_net_channel(IRUDPNetChannel *channel) {net_channel_ = channel;};
	void				set_ccc(RUDPCCCObject* ccc) {ccc_ = ccc;};

	uint32_t			get_bandwidth();
	int32_t				get_buffer_data_size() const {return buffer_data_size_;};

	void				clear_loss();
protected:
	//试图发送
	void				attempt_send(uint64_t now_timer);

protected:
	IRUDPNetChannel*	net_channel_;

	//正在发送的数据片
	SendWindowMap		send_window_;
	//正在发送的报文的丢包集合
	LossIDSet			loss_set_;
	//等待发送的数据片
	SendDataList		send_data_;

	//发送缓冲区的大小
	int32_t				buffer_size_;
	//当前缓冲数据的大小
	int32_t				buffer_data_size_;
	//当前BUFFER中最大的SEQ
	uint64_t			buffer_seq_;
	//当前WINDOW中最大的SEQ
	uint64_t			cwnd_max_seq_;
	//接收端最大的SEQ
	uint64_t			dest_max_seq_;
	//速度控制器
	RUDPCCCObject*		ccc_;
	//是否启动NAGLE算法
	bool				nagle_;

	uint32_t			bandwidth_;
	uint64_t			bandwidth_ts_;
};

BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/



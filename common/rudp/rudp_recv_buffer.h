/*************************************************************************************
*filename:	rudp_recv_buffer.h
*
*to do:		定义RUDP接收窗口Buffer
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_RECV_BUFFER_H_
#define __RUDP_RECV_BUFFER_H_

#include "rudp/rudp_channel_interface.h"
#include "rudp/rudp_segment.h"

#include <map>
#include <list>

BASE_NAMESPACE_BEGIN_DECL

typedef map<uint64_t, RUDPRecvSegment*> RecvWindowMap;
typedef list<RUDPRecvSegment*>	RecvDataList;

typedef map<uint64_t, uint64_t>	LossIDTSMap;

class RUDPRecvBuffer
{
public:
	RUDPRecvBuffer();
	virtual~RUDPRecvBuffer();

	void				reset();
	//来自网络中的数据
	int32_t				on_data(uint64_t seq, const uint8_t* data, int32_t data_size);
	//定时
	void				on_timer(uint64_t now_timer, uint32_t rtc);

	//读取BUFFER中的数据
	int32_t				read(uint8_t* data, int32_t data_size);

	uint64_t			get_ack_id() const { return first_seq_;};

	uint32_t			get_bandwidth();

	void				set_net_channel(IRUDPNetChannel* channel) {net_channel_ = channel;};
	void				set_send_last_ack_ts(uint64_t ts) {last_ack_ts_ = ts;  recv_new_packet_ = false;};
	//只有启动收到对方握手时调用一次！！
	void				set_first_seq(uint64_t first_seq) {first_seq_ = first_seq - 1; max_seq_ = first_seq_;};

	void				check_buffer();
	int32_t				get_buffer_data_size();

protected:
	void				check_recv_window();
	bool				check_loss(uint64_t now_timer, uint32_t rtc);

protected:
	IRUDPNetChannel*	net_channel_;

	//接收窗口
	RecvWindowMap		recv_window_;
	//已完成的连续数据片
	RecvDataList		recv_data_;
	//丢包序列
	LossIDTSMap			loss_map_;

	//当前BUFFER中最大连续数据片的SEQ
	uint64_t			first_seq_;
	//当期BUFFER中受到的最大的数据片ID
	uint64_t			max_seq_;
	//最后一次发送ACK的时刻
	uint64_t			last_ack_ts_;
	//在上次发送ACK到现在，受到新的连续报文的标志	
	bool				recv_new_packet_;

	uint32_t			bandwidth_;
	uint64_t			bandwidth_ts_;
};

BASE_NAMESPACE_END_DECL

#endif   
/************************************************************************************/



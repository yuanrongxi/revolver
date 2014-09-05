/*************************************************************************************
*filename:	rudp_segment.h
*
*to do:		定义RUDP的分片对象及对象池，RUDP最大分片位1280字节
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_SEGMENT_H_
#define __RUDP_SEGMENT_H_

#include "revolver/base_typedef.h"
#include "revolver/object_pool.h"

#include <string>
#include <set>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

#define MAX_SEGMENT_SIZE	1408

typedef set<uint64_t>		LossIDSet;

//发送数据片
typedef struct tagRUDPSendSegment
{
	uint64_t	seq_;
	uint64_t	push_ts_;			//进入发送队列的时刻
	uint64_t	last_send_ts_;		//最后一次发送的时刻
	uint16_t	send_count_;		//发送的次数
	uint8_t		data_[MAX_SEGMENT_SIZE];	
	uint16_t	data_size_;			

	tagRUDPSendSegment()
	{
		reset();
	};

	void reset()
	{
		seq_ = 0;
		push_ts_ = 0;
		last_send_ts_ = 0;
		send_count_ = 0;
		data_size_ = 0;
	};
}RUDPSendSegment;
//接收数据片
typedef struct tagRUDPRecvSegment
{
	uint64_t	seq_;
	uint8_t		data_[MAX_SEGMENT_SIZE];
	uint16_t	data_size_;	

	tagRUDPRecvSegment()
	{
		seq_ = 0;
		data_size_ = 0;
	};

	void reset()
	{
		seq_ = 0;
		data_size_ = 0;
	};
}RUDPRecvSegment;

extern ObjectPool<RUDPSendSegment, RUDP_SEGMENT_POOL_SIZE>	SENDSEGPOOL;
extern ObjectPool<RUDPRecvSegment, RUDP_SEGMENT_POOL_SIZE>	RECVSEGPOOL;

#define GAIN_SEND_SEG(seg) \
	RUDPSendSegment* seg = SENDSEGPOOL.pop_obj();\
	seg->reset()

#define RETURN_SEND_SEG(seg) \
	if(seg != NULL)\
	SENDSEGPOOL.push_obj(seg)

#define GAIN_RECV_SEG(seg) \
	RUDPRecvSegment* seg = RECVSEGPOOL.pop_obj(); \
	seg->reset()

#define RETURN_RECV_SEG(seg) \
	if(seg != NULL)\
	RECVSEGPOOL.push_obj(seg)

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


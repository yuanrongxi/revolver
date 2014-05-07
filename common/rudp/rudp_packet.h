/*************************************************************************************
*filename:	rudp_packet.h
*
*to do:		RUDP协议定义
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_PACKET_H_
#define __RUDP_PACKET_H_

#include "revolver/base_packet.h"
#include "revolver/base_bin_stream.h"
#include "revolver/object_pool.h"

#include <vector>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

#define PARSE_RUDP_MESSAGE(strm, T, body, info) \
	T body;\
	try{\
	strm >> body;\
}\
	catch(...){\
	RUDP_ERROR(info);\
	return; \
}

//连接保持协议
#define RUDP_SYN				0x10		//主动发起连接
#define RUDP_SYN2				0x11		//发起连接返回包
#define RUDP_SYN_ACK			0x02		//SYN2的ACK
#define RUDP_FIN				0x13		//主动发起关闭
#define RUDP_FIN2				0x14		//关闭返回包
#define RUDP_KEEPALIVE			0x15		//心跳包
#define RUDP_KEEPALIVE_ACK		0x16		//心跳返回包

//数据协议
#define RUDP_DATA				0x20		//可靠数据
#define RUDP_DATA_ACK			0x23		//可靠数据确认
#define RUDP_DATA_NACK			0X24		//丢包确认

//网络错误码
#define ERROR_SYN_STATE			0x01
#define SYSTEM_SYN_ERROR		0X02

class RUDPHeadPacket : public CBasePacket
{
public:
	RUDPHeadPacket(){};
	virtual ~RUDPHeadPacket(){};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << msg_id_ << remote_rudp_id_ << check_sum_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> msg_id_ >> remote_rudp_id_ >> check_sum_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "msg id = " << ", remote rudp id = " <<remote_rudp_id_ << "local sum = " << check_sum_ << std::endl;
	};

public:
	uint8_t		msg_id_;
	uint16_t	check_sum_;
	int32_t		remote_rudp_id_; //-1表示非法
};

class RUDPSynPacket : public CBasePacket
{
public:
	RUDPSynPacket(){};
	virtual ~RUDPSynPacket(){};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << version_ << max_segment_size_ << local_rudp_id_ << start_seq_ << local_ts_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> version_ >> max_segment_size_ >> local_rudp_id_ >> start_seq_ >> local_ts_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "version = " << version_ << ", max_segment_size = " << max_segment_size_ \
			<< ", local rudp id = " << local_rudp_id_ << ", start seq = " << local_rudp_id_\
			<< ", local ts = " << local_ts_ << std::endl;
	}

public:
	uint8_t			version_;				//版本
	uint16_t		max_segment_size_;		//最大块尺寸
	int32_t			local_rudp_id_;			//本地RUDP 索引ID,类似socket id
	uint64_t		start_seq_;				//起始序号
	uint64_t		local_ts_;				//本地时间戳
};	


class RUDPSyn2Packet : public CBasePacket
{
public:
	RUDPSyn2Packet()
	{
	};

	virtual ~RUDPSyn2Packet()
	{
	};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << version_ << syn2_result_ << max_segment_size_ << local_rudp_id_ << start_seq_ << local_ts_ << remote_ts_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> version_>> syn2_result_ >> max_segment_size_ >> local_rudp_id_ >> start_seq_ >> local_ts_ >> remote_ts_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "version = " << version_ << ", max_segment_size = " << max_segment_size_ \
			<< ", syn2_result_ = " << syn2_result_ \
			<< ", local rudp id = " << local_rudp_id_ << ", start seq = " << local_rudp_id_ \
			<< ", local ts = " << local_ts_ << ", remote ts = " << remote_ts_ << std::endl;
	}
public:
	uint8_t			version_;			//版本
	uint8_t			syn2_result_;		//握手结果
	uint16_t		max_segment_size_;	//本地最大块尺寸
	int32_t			local_rudp_id_;		//本地rudp 索引ID
	uint64_t		start_seq_;			//本地起始序号
	uint64_t		local_ts_;			//本地时间戳
	uint64_t		remote_ts_;			//远端时间戳, RUDPSynPacket.local_ts_
};

class RUDPSyn2AckPacket : public CBasePacket
{
public:
	RUDPSyn2AckPacket()
	{
	};

	virtual ~RUDPSyn2AckPacket()
	{
	}

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << result_ << remote_ts_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> result_ >> remote_ts_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "result = " << (uint16_t)result_  << ", remote ts = " << remote_ts_ << std::endl;
	}

public:
	uint8_t			result_;
	uint64_t		remote_ts_;	//远端时间戳，RUDPSyn2Packet.local_ts_
};

class RDUPKeepLive : public CBasePacket
{
public:
	RDUPKeepLive(){};
	virtual ~RDUPKeepLive(){};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << timestamp_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> timestamp_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "timestamp = " << timestamp_ << std::endl;
	};

public:
	uint64_t	timestamp_;		//时间戳，主要用于计算RTT
};

class RUDPData : public CBasePacket
{
public:
	RUDPData() {};
	virtual ~RUDPData(){};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << ack_seq_id_ << cur_seq_id_;// << data_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> ack_seq_id_ >> cur_seq_id_;// >> data_;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "ack seq id = " << ack_seq_id_ << ",cur seq id = " << cur_seq_id_ << std::endl;
	};

public:
	//当前接受队列中最大SEQ
	uint64_t	ack_seq_id_;
	//当前数据块的SEQ
	uint64_t	cur_seq_id_;

	//string		data_;
};

class RUDPDataAck : public CBasePacket
{
public:
	RUDPDataAck(){};
	virtual ~RUDPDataAck(){};

protected:
	virtual void Pack(BinStream& strm) const
	{
		strm << ack_seq_id_;
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> ack_seq_id_ ;
	};

	virtual void Print(std::ostream& os)const
	{
		os << "ack seq id = " << ack_seq_id_ << std::endl;
	};

public:
	//当前接受队列中最大SEQ
	uint64_t	ack_seq_id_;
};

typedef vector<uint16_t>	LossIDArray;

class RUDPDataNack : public CBasePacket
{
public:
	RUDPDataNack() {};
	virtual ~RUDPDataNack() {};

protected:
		virtual void Pack(BinStream& strm) const
	{
		strm << base_seq_;

		uint16_t array_size = loss_ids_.size();
		strm << array_size;
		for(uint16_t i = 0; i < array_size; ++i)
		{
			strm << loss_ids_[i];
		}
	};

	virtual void UnPack(BinStream& strm)
	{
		strm >> base_seq_ ;

		uint16_t array_size;
		strm >> array_size;
		loss_ids_.resize(array_size);
		for(uint16_t i = 0; i < array_size; ++i)
		{
			strm >> loss_ids_[i];
		}
	};

	virtual void Print(std::ostream& os)const
	{
		os << "base seq id = " << base_seq_ << "loss ids =";
		for(uint16_t i = 0; i < loss_ids_.size(); ++i)
		{
			os << " " << loss_ids_[i];
		}
	};

public:
	uint64_t		base_seq_;	//丢包起始ID
	LossIDArray		loss_ids_; //丢包序列，相对base seq的距离。可以通过loss ids[i]和base_seq相加得到丢包的序号
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/




#include "core/core_packet.h"
#include "revolver/base_timer_value.h"

//×î´ó10M
#define MAX_PACKET_BODY_SIZE	1048576

BASE_NAMESPACE_BEGIN_DECL
ObjectPool<BinStream, STREAM_POOL_SIZE>	STREAMPOOL;


void CCorePacket::set_body(CBasePacket& packet)
{
	body_ptr_ = &packet;
}

void CCorePacket::Pack(BinStream& strm) const
{
	strm << server_id_ << server_type_ << msg_id_ << msg_type_;
	if(body_ptr_ != NULL){
		strm << *body_ptr_;
	}
}

void CCorePacket::UnPack(BinStream& strm)
{
	strm >> server_id_ >> server_type_ >> msg_id_ >> msg_type_;
	if(body_ptr_ != NULL){
		strm >> *body_ptr_;
	}
}


BASE_NAMESPACE_END_DECL

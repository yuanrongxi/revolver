#include "core_packet.h"
#include "base_timer_value.h"

//×î´ó10M
#define MAX_PACKET_BODY_SIZE	1048576

BASE_NAMESPACE_BEGIN_DECL
ObjectPool<BinStream, STREAM_POOL_SIZE>	STREAMPOOL;


void CCorePacket::set_data(CBasePacket& packet, bool zlib)
{
	GAIN_BINSTREAM(strm);

	*strm << packet;
	strm->bin_to_string(data_);

	RETURN_BINSTREAM(strm);
}

void CCorePacket::get_data(CBasePacket& packet)
{
	GAIN_BINSTREAM(strm);

	*strm = data_;
	*strm >> packet;

	RETURN_BINSTREAM(strm);
}

void CCorePacket::Pack(BinStream& strm) const
{
	strm << server_id_ << server_type_ << msg_id_ << msg_type_ << data_;
}

void CCorePacket::UnPack(BinStream& strm)
{
	strm >> server_id_ >> server_type_ >> msg_id_ >> msg_type_ >> data_;
}


BASE_NAMESPACE_END_DECL

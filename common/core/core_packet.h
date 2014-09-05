/*************************************************************************************
*filename:	core_packet.h
*
*to do:		定义底层消息基础类,主要包含基本的头信息、消息ID等
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_PACKET_H
#define __CORE_PACKET_H

#include "revolver/base_packet.h"
#include "revolver/base_bin_stream.h"
#include "revolver/object_pool.h"

#include <string>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

extern ObjectPool<BinStream, STREAM_POOL_SIZE>	STREAMPOOL;

#define GAIN_BINSTREAM(x) \
	BinStream* x = STREAMPOOL.pop_obj();\
	x->rewind(true)


#define RETURN_BINSTREAM(x) \
	if(x != NULL) \
	{\
		x->rewind(true);\
		x->reduce();\
		STREAMPOOL.push_obj(x);\
	}

typedef enum PacketClass
{
	CORE_HANDSHAKE,
	CORE_REQUEST,
	CORE_PING,
	CORE_MEDIA_SHELL,
	CORE_ZLIB,
	CORE_PONG
}PacketClass;
//定义净荷相对大的报文，主要描述
class CCorePacket : public CBasePacket
{
public:
	CCorePacket()
	{
		server_id_		= 0;
		server_type_	= 0;
		msg_id_			= 0;
		msg_type_		= CORE_REQUEST;
		body_ptr_		= NULL;
	};

	~CCorePacket()
	{

	}

	virtual void release_self()
	{
		delete this;
	}

	CCorePacket& operator=(const CCorePacket& packet)
	{
		server_id_		= packet.server_id_;
		server_type_	= packet.server_type_;
		msg_id_			= packet.msg_id_;
		msg_type_		= packet.msg_type_;
		body_ptr_		= packet.body_ptr_;

		return *this;
	}

	void		set_body(CBasePacket& packet);

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const;
	
	//解码函数
	virtual void	UnPack(BinStream& strm);

	virtual void	Print(std::ostream& os) const
	{
		os << "Core Packet, {"\
			<< "server_id = " << server_id_ \
			<<", server_type = " << (uint16_t)server_type_ \
			<<", msg_id_ = " << msg_id_ \
			<< ", msg_type = " << (uint16_t) msg_type_\
			<<"}" << std::endl;
	}
	
public:
	uint32_t		server_id_;		//服务器ID
	uint8_t			server_type_;	//服务器类型,0表示客户端
	uint32_t		msg_id_;		//消息ID
	uint8_t			msg_type_;		//消息类型，例如独立的PING PONG消息，握手消息，应用层消息等
	CBasePacket*	body_ptr_;		//消息内容
};

class HandShakeBody : public CBasePacket
{
public:
	HandShakeBody()
	{

	};

	~HandShakeBody()
	{

	};

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << digest_data;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> digest_data;
	};

public:
	string digest_data;	//摘要信息
};

#define INIT_CORE_REQUEST(p, MSGID)\
	CCorePacket p;\
	p.msg_id_ = MSGID;\
	p.msg_type_ = CORE_REQUEST;\
	p.server_id_ = SERVER_ID;\
	p.server_type_ = SERVER_TYPE

#define INIT_CORE_PING(p)\
	CCorePacket p;\
	p.msg_id_ = 0;\
	p.msg_type_ = CORE_PING;\
	p.server_id_ = SERVER_ID;\
	p.server_type_ = SERVER_TYPE

#define INIT_CORE_PONG(p)\
	CCorePacket p;\
	p.msg_id_ = 0;\
	p.msg_type_ = CORE_PONG;\
	p.server_id_ = SERVER_ID;\
	p.server_type_ = SERVER_TYPE

#define INIT_CORE_HANDSHAKE(p)\
	CCorePacket p;\
	p.msg_id_ = 0;\
	p.msg_type_ = CORE_HANDSHAKE;\
	p.server_id_ = SERVER_ID;\
	p.server_type_ = SERVER_TYPE

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


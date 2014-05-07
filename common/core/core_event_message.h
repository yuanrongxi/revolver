/*************************************************************************************
*filename:	core_event_message.h
*
*to do:		定义TCP断开和连接通知消息
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __CORE_EVENT_MESSAGE_H
#define __CORE_EVENT_MESSAGE_H

#include "revolver/base_packet.h"
#include "core/core_message_map_decl.h"

BASE_NAMESPACE_BEGIN_DECL

#define TCP_CONNECT_EVENT 0x00000000

class TCP_Connect_Message : public CBasePacket
{
public:
	TCP_Connect_Message()
	{
	};

	~TCP_Connect_Message()
	{
	};

	virtual void release_self()
	{
		delete this;
	}

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
	};
};

#define TCP_CLOSE_EVENT	0xffffffff
class TCP_Close_Message : public CBasePacket
{
public:
	TCP_Close_Message(){};
	virtual ~TCP_Close_Message(){};

	virtual void release_self()
	{
		delete this;
	}

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
	};
};


//消息体映射关系
MESSAGEMAP_DECL_BEGIN(TCP_EVENT)
MESSAGEMAP_REGISTER(TCP_CONNECT_EVENT, new TCP_Connect_Message())
MESSAGEMAP_REGISTER(TCP_CLOSE_EVENT, new TCP_Close_Message())
MESSAGEMAP_DECL_END()

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/

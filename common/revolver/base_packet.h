/*************************************************************************************
*filename:	base_packet.h
*
*to do:		定义网络协议基类，如果需要定义一个协议，只需要继承此类，并实现PACK和UNPACK
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_PACKET_H
#define __BASE_PACKET_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "revolver/base_bin_stream.h"

#include <iostream>


BASE_NAMESPACE_BEGIN_DECL

class CBasePacket
{
public:
	CBasePacket()
	{
	};

	virtual ~CBasePacket()
	{
	};

	friend BinStream& operator<<(BinStream& strm, const CBasePacket& packet)
	{
		packet.Pack(strm);
		return strm;
	};

	friend BinStream& operator>>(BinStream& strm, CBasePacket& packet)
	{	
		packet.UnPack(strm);
		return strm;
	};

	friend std::ostream& operator<<(std::ostream& os, const CBasePacket& packet)
	{
		packet.Print(os);
		return os;
	}

	virtual void release_self()
	{
		delete this;
	};

protected:
	virtual void Pack(BinStream& strm) const  = 0;
	virtual void UnPack(BinStream& strm) = 0;
	virtual void Print(std::ostream& /*os*/)const {};
};

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/

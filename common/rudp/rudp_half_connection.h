/*************************************************************************************
*filename:	rudp_half_connection.h
*
*to do:		定义RUDP半开连接数据接口
*Create on: 2013-04
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __RUDP_HALF_CONNECTION_H_
#define __RUDP_HALF_CONNECTION_H_

#include "revolver/base_typedef.h"
#include "revolver/base_namespace.h"
#include "revolver/base_inet_addr.h"

#include <map>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL
	typedef struct tagRUDPPeerInfo
{
	int32_t		rudp_id;
	Inet_Addr	addr;

	tagRUDPPeerInfo(int32_t id, const Inet_Addr& peer_addr) : rudp_id(id), addr(peer_addr)
	{
	};

	bool	operator==(const tagRUDPPeerInfo& info) const
	{
		if(rudp_id == info.rudp_id && addr == info.addr)
			return true;
		else
			return false;
	};

	bool	operator!=(const tagRUDPPeerInfo& info) const
	{
		if(rudp_id != info.rudp_id || addr == info.addr)
			return true;
		else
			return false;
	};

	bool	operator>(const tagRUDPPeerInfo& info) const
	{
		if(rudp_id > info.rudp_id)
			return true;
		else if(rudp_id < info.rudp_id)
			return false;
		else if(addr > info.addr)
			return true;
		else 
			return false;
	};

	bool	operator<(const tagRUDPPeerInfo& info) const
	{
		if(rudp_id < info.rudp_id)
			return true;
		else if(rudp_id > info.rudp_id)
			return false;
		else if(addr < info.addr)
			return true;
		else 
			return false;
	};

}RUDPPeerInfo;

typedef map<tagRUDPPeerInfo, int32_t>	RUDPHandleMap;	

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


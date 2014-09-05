/*************************************************************************************
*filename:	core_reciver_interface.h
*
*to do:		定义一个基本的网络数据接收接口
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __CORE_RECIVER_INTERFACE_H
#define __CORE_RECIVER_INTERFACE_H

#include "revolver/base_typedef.h"
#include "revolver/base_namespace.h"

#include <vector>
using namespace std;


BASE_NAMESPACE_BEGIN_DECL
class CConnection;
class Inet_Addr;

class IReciver
{
public:
	virtual int32_t	reciver(BinStream& bin_strm, const Inet_Addr& remote_addr){return -1;};
	virtual int32_t	reciver(BinStream& bin_strm, CConnection* conn){return -1;};
	virtual int32_t	reciver_media(BinStream& bin_strm,  const Inet_Addr& remote_addr){return -1;};
	virtual int32_t	reciver_media(BinStream& bin_strm, CConnection* conn){return -1;};
	virtual int32_t on_connect(uint32_t server_id, CConnection* conn){return -1;};
	virtual int32_t on_disconnect(uint32_t server_id, CConnection* conn){return -1;};
};

typedef vector<IReciver*>	UDPReciverArray;

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/

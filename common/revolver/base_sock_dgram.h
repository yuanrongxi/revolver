/*************************************************************************************
*filename:	base_sock_dgram.h
*
*to do:		∂®“ÂUDP SOCKET¿‡
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_SOCK_DGRAM_H
#define __BASE_SOCK_DGRAM_H

#include "revolver/base_socket.h"

BASE_NAMESPACE_BEGIN_DECL

class CSockDgram : public CBaseSocket
{
public:
	CSockDgram();
	virtual ~CSockDgram();

	int32_t open(const Inet_Addr& local_addr, bool nonblocking = false,  bool resue = true, bool client = false);

private:
	int32_t	bind(Inet_Addr& local_addr);

private:
	Inet_Addr	local_addr_;
};

BASE_NAMESPACE_END_DECL
#endif

/************************************************************************************/

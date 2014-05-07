/*************************************************************************************
*filename:	base_sock_connector.h
*
*to do:		定义TCP SOCKET的连接发起类，主要用于TCP CLIENT
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_SOCK_CONNNECTOR_H
#define __BASE_SOCK_CONNNECTOR_H

#include "revolver/base_sock_stream.h"

BASE_NAMESPACE_BEGIN_DECL

class CSockConnector
{
public:
	CSockConnector();
	~CSockConnector();

	int32_t connect(CSockStream& sock_stream, const Inet_Addr& remote_addr);

private:
	Inet_Addr remote_addr_;
};

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/

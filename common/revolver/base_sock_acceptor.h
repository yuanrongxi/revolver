/*************************************************************************************
*filename:	base_sock_acceptor.h
*
*to do:		定义TCP SOCKET的连接接受类
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_SOCK_ACCEPTOR_H
#define __BASE_SOCK_ACCEPTOR_H

#include "revolver/base_sock_stream.h"

BASE_NAMESPACE_BEGIN_DECL

#define LISTEN_MAX_NUMBER 40960

class CSockAcceptor
{
public:
	CSockAcceptor();
	~CSockAcceptor();

	BASE_HANDLER get_socket_handler() const;

	int32_t	open(const Inet_Addr& addr, bool nonblocking = false, int32_t listen_max = LISTEN_MAX_NUMBER);
	int32_t close();

	int32_t accept(CSockStream& new_stream, Inet_Addr &remote_addr, bool nonblocking = false);

	int32_t	get_local_addr(Inet_Addr& local_addr) const;

private:
	CSockStream	sock_stream_;
};

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/

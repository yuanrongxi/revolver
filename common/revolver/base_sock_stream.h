/*************************************************************************************
*filename:	base_sock_stream.h
*
*to do:		定义TCP SOCKET类
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __BASE_SOCK_STREAM_H
#define __BASE_SOCK_STREAM_H

#include "revolver/base_socket.h"

BASE_NAMESPACE_BEGIN_DECL
class CSockStream : public CBaseSocket
{
public:
	CSockStream();
	virtual ~CSockStream();

	int32_t			open(const Inet_Addr& local_addr, bool nonblocking = false, bool resue = true, bool client = false);
	//获得远端的对应IP地址,一般TCP有效
	int32_t			get_remote_addr (Inet_Addr &remote_addr) const;
	//绑定地址
	int32_t			bind(Inet_Addr& local_addr);
private:
	Inet_Addr		local_addr_;
};
BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/



/*************************************************************************************
*filename:	dc_tcp_listener.h
*
*to do:		定义专门为DCENTER的TCP监听服务
*Create on: 2013-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __DC_TCP_LISTENER_H_
#define __DC_TCP_LISTENER_H_

#include "revolver/base_namespace.h"
#include "revolver/base_sock_acceptor.h"
#include "revolver/base_event_handler.h"

BASE_NAMESPACE_BEGIN_DECL
class IShortProcessor;

class DCTCPListener : public CEventHandler
{
public:
	DCTCPListener();
	~DCTCPListener();

	void			set_short_proc(IShortProcessor* proc) {proc_ = proc;};
	bool			is_open() const;

	BASE_HANDLER	get_handle() const;

	int32_t			open(const Inet_Addr& local_addr);
	int32_t			close();


	int32_t			handle_input(BASE_HANDLER handle);
	int32_t			handle_output(BASE_HANDLER handle);
	int32_t			handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	int32_t			handle_exception(BASE_HANDLER handle);

protected:
	CSockAcceptor	acceptor_;
	IShortProcessor* proc_;
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


/*************************************************************************************
*filename:	core_udp_hander.h
*
*to do:		UDPhandler类，主要是实现UDP的监听、数据收发、处理控制等
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __CORE_UDP_HANDLER_H
#define __CORE_UDP_HANDLER_H

#include "revolver/base_event_handler.h"
#include "revolver/base_sock_dgram.h"
#include "revolver/base_bin_stream.h"
#include "core/core_reciver_interface.h"

BASE_NAMESPACE_BEGIN_DECL

class CBasePacket;
class CCorePacket;
class CoreExpPacket;

class CoreUDPHandler : public CEventHandler
{
public:
	CoreUDPHandler();
	virtual ~CoreUDPHandler();
	
	bool			is_open() const;

	void			attach(IReciver* reciver);

	int32_t			open(const Inet_Addr& local_addr);
	int32_t			close();
	int32_t			send(BinStream& bin_strm, const Inet_Addr& remote_addr);
	int32_t			send(const CCorePacket& pakcet, const Inet_Addr& remote_addr);
	int32_t			send(const string& data, const Inet_Addr& remote_addr);

	BASE_HANDLER	get_handle() const;

	virtual int32_t	handle_input(BASE_HANDLER handle);
	virtual int32_t handle_output(BASE_HANDLER handle);
	virtual int32_t handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	virtual int32_t handle_exception(BASE_HANDLER handle);

private:
	int32_t			process(IReciver* reciver, uint8_t packet_type, BinStream& bin_strm, const Inet_Addr& remote_addr);

private:
	BinStream		bin_strm_;		
	CSockDgram		sock_dgram_;
	UDPReciverArray	recivers_;
};


BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


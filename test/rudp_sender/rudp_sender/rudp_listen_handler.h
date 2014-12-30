#ifndef __RUDP_LISTENER_HANDLER_H_
#define __RUDP_LISTENER_HANDLER_H_

#include "rudp/rudp_interface.h"
#include "revolver/base_singleton.h"

using namespace BASE;

//用于接收和处理来自网络的RUDP连接事件，只需要实现rudp_accept_event
class RUDPListenHandler : public RUDPEventHandler
{
public:
	RUDPListenHandler();
	~RUDPListenHandler();

	virtual	int32_t rudp_accept_event(int32_t rudp_id);
	virtual	int32_t	rudp_input_event(int32_t rudp_id);
	virtual int32_t rudp_output_event(int32_t rudp_id);
	virtual	int32_t rudp_close_event(int32_t rudp_id);
	virtual int32_t rudp_exception_event(int32_t rudp_id);
};

#define CREATE_RUDP_LISTEN	CSingleton<RUDPListenHandler>::instance
#define RUDP_LISTEN			CSingleton<RUDPListenHandler>::instance
#define DESTROY_RUDP_LISTEN	CSingleton<RUDPListenHandler>::destroy

#endif

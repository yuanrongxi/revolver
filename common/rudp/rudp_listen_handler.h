#ifndef __REVOLVER_RUDP_LISTEN_HANDLER_H__
#define __REVOLVER_RUDP_LISTEN_HANDLER_H__

#include "rudp/rudp_interface.h"

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



#endif // __REVOLVER_RUDP_LISTEN_HANDLER_H__

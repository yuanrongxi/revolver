#ifndef __REVOLVER_RUDP_LISTEN_HANDLER_H__
#define __REVOLVER_RUDP_LISTEN_HANDLER_H__

#include "rudp/rudp_interface.h"
#include "rudp/rudp_adapter.h"
#include "rudp/rudp_handler.h"

BASE_NAMESPACE_BEGIN_DECL

//用于接收和处理来自网络的RUDP连接事件，只需要实现rudp_accept_event
class RUDPListenerHandler : public RUDPEventHandler
{
public:
    RUDPListenerHandler();
    ~RUDPListenerHandler();

    virtual	int32_t rudp_accept_event(int32_t rudp_id);
    virtual	int32_t	rudp_input_event(int32_t rudp_id);
    virtual int32_t rudp_output_event(int32_t rudp_id);
    virtual	int32_t rudp_close_event(int32_t rudp_id);
    virtual int32_t rudp_exception_event(int32_t rudp_id);
};

#define CREATE_RUDP_LISTEN_HANDLER	CSingleton<RUDPListenerHandler>::instance
#define RUDP_LISTEN_HANDLER			CSingleton<RUDPListenerHandler>::instance
#define DESTROY_RUDP_LISTEN_HANDLER	CSingleton<RUDPListenerHandler>::destroy

class RUDPListener {
public:
    RUDPListener() : _adapter(&_rudp_handler) {}
    virtual ~RUDPListener() {}

    int32_t open(const Inet_Addr& local_addr);
    int32_t close();

protected:
    RudpAdapter _adapter;
    RudpHandler _rudp_handler;
};

int32_t create_rudp_listener();
void destroy_rudp_listener();

BASE_NAMESPACE_END_DECL

#endif // __REVOLVER_RUDP_LISTEN_HANDLER_H__

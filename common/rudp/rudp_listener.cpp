#include "rudp/rudp_listener.h"
#include "rudp/rudp_connection.h"
#include "rudp/rudp_stream.h"
#include "rudp/rudp_socket.h"
#include "core/core_message_processor.h"

BASE_NAMESPACE_BEGIN_DECL

RUDPListenerHandler::RUDPListenerHandler()
{

}

RUDPListenerHandler::~RUDPListenerHandler()
{
}

int32_t RUDPListenerHandler::rudp_accept_event(int32_t rudp_id)
{
    GAIN_RUDP_CONN(conn);

    RUDPStream& sock = conn->get_rudp_stream();
    sock.set_handler(rudp_id);

    int buffer_size = RUDP_SEND_BUFFER;
    sock.set_option(RUDP_SEND_BUFF_SIZE, buffer_size);
    //sock.set_option(RUDP_TIMEOUT_COUNT, 2); //6秒超时
    //关联HANDLER
    RUDP()->bind_event_handle(sock.get_handler(), conn);

    //获取远端地址
    Inet_Addr remote_addr;
    sock.get_peer_addr(remote_addr);
    conn->set_remote_addr(remote_addr);

    conn->set_state(RudpConnection::CONN_CONNECTED);

    MSG_PROCESSOR()->on_connect(0, conn);
    return 0;
}

int32_t RUDPListenerHandler::rudp_input_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenerHandler::rudp_output_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenerHandler::rudp_close_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenerHandler::rudp_exception_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListener::open(const Inet_Addr& local_addr) {
    _rudp_handler.attach_adapter(&_adapter);
    if (_rudp_handler.open(local_addr) == 0)
    {
        RUDP()->attach_listener(RUDP_LISTEN_HANDLER());
        CORE_INFO("open rudp listener, listener addr = " << local_addr);
        return 0;
    }
    else
    {
        CORE_ERROR("open rudp listener failed, listener addr = " << local_addr << ",error = " << error_no());
        return -1;
    }
    return 0;
}

int32_t RUDPListener::close() {
    _rudp_handler.close();
    return 0;
}

int32_t create_rudp_listener() {
    CREATE_RUDP_LISTEN_HANDLER();
    RUDP()->attach_listener(RUDP_LISTEN_HANDLER());
    return 0;
}

void destroy_rudp_listener() {
    RUDP()->attach_listener(0);
    DESTROY_RUDP_LISTEN_HANDLER();
}

BASE_NAMESPACE_END_DECL
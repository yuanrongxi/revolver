#include "rudp/rudp_listen_handler.h"
#include "rudp/rudp_connection.h"
#include "rudp/rudp_stream.h"
#include "rudp/rudp_socket.h"
#include "core/core_message_processor.h"

RUDPListenHandler::RUDPListenHandler()
{

}

RUDPListenHandler::~RUDPListenHandler()
{
}

int32_t RUDPListenHandler::rudp_accept_event(int32_t rudp_id)
{
    GAIN_RUDP_CONN(conn);

    RUDPStream& sock = conn->get_rudp_stream();
    sock.set_handler(rudp_id);

    int buffer_size = RUDP_SEND_BUFFER;
    sock.set_option(RUDP_SEND_BUFF_SIZE, buffer_size);
    sock.set_option(RUDP_TIMEOUT_COUNT, 2); //6秒超时
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

int32_t RUDPListenHandler::rudp_input_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenHandler::rudp_output_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenHandler::rudp_close_event(int32_t rudp_id)
{
    return 0;
}

int32_t RUDPListenHandler::rudp_exception_event(int32_t rudp_id)
{
    return 0;
}




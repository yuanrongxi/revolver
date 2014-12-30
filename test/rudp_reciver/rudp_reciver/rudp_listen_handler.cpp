#include "rudp_listen_handler.h"
#include "rudp_connection.h"
#include "rudp/rudp_socket.h"
#include "rudp/rudp_stream.h"

RUDPListenHandler::RUDPListenHandler()
{

}

RUDPListenHandler::~RUDPListenHandler()
{
}

int32_t RUDPListenHandler::rudp_accept_event(int32_t rudp_id)
{
	RUDPConnection *conn = new RUDPConnection();

	RUDPStream& sock = conn->get_rudp_stream();
	sock.set_handler(rudp_id);

	int buffer_size = RUDP_SEND_BUFFER;
	sock.set_option(RUDP_SEND_BUFF_SIZE, buffer_size);
	//¹ØÁªHANDLER
	RUDP()->bind_event_handle(sock.get_handler(), conn);

	conn->set_state(RUDP_CONN_CONNECTED);
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


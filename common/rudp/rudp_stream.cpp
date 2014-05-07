#include "rudp/rudp_stream.h"
#include "rudp/rudp_socket.h"
#include "rudp/rudp_interface.h"

BASE_NAMESPACE_BEGIN_DECL

RUDPStream::RUDPStream() : handler_(INVALID_RUDP_HANDLE)
{

}

RUDPStream::~RUDPStream()
{

}

int32_t RUDPStream::get_handler() const
{
	return handler_;
}

void RUDPStream::set_handler(int32_t handle)
{
	handler_ = handle;
}

int32_t RUDPStream::open(const Inet_Addr& local_addr)
{
	if(handler_ != INVALID_RUDP_HANDLE)
		return -1;

	handler_ = rudp_socket();
	if(handler_ != INVALID_RUDP_HANDLE)
	{
		//绑定端口
		if(rdup_bind(handler_, local_addr) != 0)
		{
			this->close();
			return -1;
		}

		//设置发送缓冲区
		int32_t send_buffer_size = RUDP_SEND_BUFFER;
		this->set_option(RUDP_SEND_BUFF_SIZE, send_buffer_size);

		return 0;
	}

	return -1;
}

int32_t RUDPStream::close()
{
	if(isopen())
	{
		rudp_close(handler_);
		handler_ = INVALID_RUDP_HANDLE;
	}

	return 0;
}

int32_t	RUDPStream::force_close()
{
	if(isopen())
	{
		rudp_force_close(handler_);
		handler_ = INVALID_RUDP_HANDLE;
	}

	return 0;
}

int32_t RUDPStream::connect(const Inet_Addr& remote_addr)
{
	if(!isopen())
	{
		return -1;
	}

	return rudp_connect(handler_, remote_addr);
}

bool RUDPStream::isopen() const 
{
	return (handler_ == INVALID_RUDP_HANDLE) ? false : true;
}

int32_t RUDPStream::set_option(int32_t op_type, int32_t op_value) const
{
	if(!isopen())
		return -1;

	return rudp_setoption(handler_, op_type, op_value);
}

int32_t RUDPStream::get_local_addr(Inet_Addr& local_addr) const 
{
	if(!isopen())
		return -1;

	return RUDP()->get_local_addr(handler_, local_addr);
}

int32_t RUDPStream::get_peer_addr(Inet_Addr& remote_addr) const
{
	if(!isopen())
		return -1;

	return RUDP()->get_peer_addr(handler_, remote_addr);
}

int32_t RUDPStream::recv(uint8_t* buf, uint32_t buf_size)
{
	if(!isopen())
		return -1;

	return rudp_recv(handler_, buf, buf_size);
}

int32_t RUDPStream::send(const uint8_t* buf, uint32_t buf_size)
{
	if(!isopen())
		return -1;

	return rudp_send(handler_, buf, buf_size);
}

BASE_NAMESPACE_END_DECL



#include "base_sock_acceptor.h"

BASE_NAMESPACE_BEGIN_DECL
CSockAcceptor::CSockAcceptor()
{

}

CSockAcceptor::~CSockAcceptor()
{
	sock_stream_.close();
}

BASE_HANDLER CSockAcceptor::get_socket_handler() const
{
	return sock_stream_.get_handler();
}

int32_t CSockAcceptor::open(const BASEOBJECT::Inet_Addr &addr, bool nonblocking, int32_t listen_max)
{
	if(sock_stream_.open(addr, nonblocking) == -1)
		return -1;

	int32_t ret = ::listen(sock_stream_.get_handler(), listen_max);
	if(ret != 0)
	{
		sock_stream_.close();
		return -1;
	}

	return 0;
}

int32_t	CSockAcceptor::close()
{
	return sock_stream_.close();
}

int32_t CSockAcceptor::accept(CSockStream& new_stream, Inet_Addr &remote_addr,  bool nonblocking)
{
	sockaddr_in addr;
#ifdef WIN32
	int32_t addr_len = sizeof(addr);
#else
	uint32_t addr_len = sizeof(addr);
#endif

	BASE_HANDLER new_socket = ::accept(get_socket_handler(),(sockaddr *)&addr, &addr_len);
	if(new_socket == INVALID_HANDLER)
	{
		return -1;
	}

	if(nonblocking)
	{
		set_socket_nonblocking(new_socket);
	}

	new_stream.set_handler(new_socket);
	remote_addr = addr;

	return 0;
}

int32_t CSockAcceptor::get_local_addr(Inet_Addr& local_addr) const
{
	return sock_stream_.get_local_addr(local_addr);
}

BASE_NAMESPACE_END_DECL

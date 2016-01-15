#include "base_socket.h"

BASE_NAMESPACE_BEGIN_DECL
CBaseSocket::CBaseSocket() : handler_(INVALID_HANDLER)
{
}

CBaseSocket::~CBaseSocket()
{
}

BASE_HANDLER CBaseSocket::get_handler() const
{
	return handler_;
}

void CBaseSocket::set_handler(BASE_HANDLER handle)
{
	handler_ = handle;
}

int32_t CBaseSocket::close()
{
	if(handler_ != INVALID_HANDLER)
	{
		::closesocket(handler_);

		handler_ = INVALID_HANDLER;
	}

	return 0;
}

bool CBaseSocket::isopen() const
{
	return (handler_ != INVALID_HANDLER);
}

int32_t CBaseSocket::set_option(int level, int option, void *optval, int optlen) const
{
	return setsockopt(get_handler(), level, option, (char *)optval, optlen);
}

int32_t CBaseSocket::get_option(int level, int option, void *optval, int *optlen) const
{
#ifdef WIN32
	return getsockopt(get_handler(), level, option, (char *)optval, optlen);
#else
	return getsockopt(get_handler(), level, option, (char *)optval, (socklen_t *)optlen);
#endif
}

int32_t CBaseSocket::get_local_addr(Inet_Addr &local_addr) const
{
#ifdef WIN32
	int32_t len = sizeof(sockaddr_in);
#else
	socklen_t len = sizeof(sockaddr_in);
#endif

	  sockaddr *addr = reinterpret_cast<sockaddr *> (local_addr.get_addr());
	if(::getsockname(get_handler(), addr, &len) == -1)
		return -1;

	return 0;
}

int32_t CBaseSocket::recv(void *buf, uint32_t buf_size)
{
	if(!isopen())
		return -1;

	return ::recv(get_handler(), (char *)buf, buf_size, 0);
}

int32_t CBaseSocket::recv(void* buf, uint32_t buf_size, Inet_Addr& remote_addr)
{
	if(!isopen())
		return -1;

#ifdef WIN32
	int32_t from_size = sizeof(struct sockaddr_in);
#else
	socklen_t from_size = sizeof(struct sockaddr_in);
#endif

	return ::recvfrom(get_handler(), (char *)buf, buf_size, 0, (struct sockaddr *)remote_addr.get_addr(), &from_size);
}

int32_t CBaseSocket::send(const void *buf, uint32_t buf_size)
{
	if(!isopen())
		return -1;

	return ::send(get_handler(), (const char *)buf, buf_size, 0);
}

int32_t CBaseSocket::send(const void* buf, uint32_t buf_size, const Inet_Addr& remote_addr)
{
	if(!isopen())
		return -1;

	return ::sendto(get_handler(), (const char *)buf, buf_size, 0, (struct sockaddr *)remote_addr.get_addr(), sizeof(sockaddr_in));
}

BASE_NAMESPACE_END_DECL


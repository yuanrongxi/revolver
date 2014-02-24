#include "base_sock_dgram.h"

BASE_NAMESPACE_BEGIN_DECL

CSockDgram::CSockDgram()
{

}

CSockDgram::~CSockDgram()
{

}

int32_t CSockDgram::open(const Inet_Addr& local_addr, bool nonblocking, bool resue, bool client)
{
	handler_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(INVALID_HANDLER == handler_)
	{
		return -1;
	}

#ifdef WIN32 //解决WINSOCK2 的UDP端口ICMP的问题
	int32_t byte_retruned = 0;
	bool new_be = false;  

	int32_t status = WSAIoctl(handler_, SIO_UDP_CONNRESET,  
		&new_be, sizeof(new_be), NULL, 0, (LPDWORD)&byte_retruned, NULL, NULL);  
#endif

	//设置异步模式
	if(nonblocking)
		set_socket_nonblocking(get_handler());

	if(resue)
	{
		//设置端口复用
		int32_t val = 1;
		set_option(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32_t));
	}

	local_addr_ = local_addr;
	int32_t ret = bind(local_addr_);
	if(ret == -1)
	{
		return ret;
	}

	//64K buffer,设置收发缓冲区
	int32_t buf_size = 64 * 1024;
	set_option(SOL_SOCKET, SO_RCVBUF, (void *)&buf_size, sizeof(int32_t));
	set_option(SOL_SOCKET, SO_SNDBUF, (void *)&buf_size, sizeof(int32_t));

	return ret;
}

int32_t CSockDgram::bind(Inet_Addr &local_addr)
{
	int32_t ret = ::bind(handler_, (struct sockaddr *)local_addr.get_addr(), sizeof(sockaddr_in));
	if(ret != 0)
	{
		CBaseSocket::close();
		return -1;
	}

	return 0;
}

BASE_NAMESPACE_END_DECL

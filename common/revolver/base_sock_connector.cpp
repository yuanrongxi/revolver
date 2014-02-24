#include "base_sock_connector.h"

BASE_NAMESPACE_BEGIN_DECL
CSockConnector::CSockConnector()
{

}

CSockConnector::~CSockConnector()
{

}

int32_t CSockConnector::connect(CSockStream &sock_stream, const Inet_Addr &remote_addr)
{
	if(remote_addr.is_null())
	{
		return -1;
	}

	remote_addr_ = remote_addr;
	
	int32_t ret = ::connect(sock_stream.get_handler(), (struct sockaddr *)remote_addr_.get_addr(), sizeof(sockaddr_in));
	if(ret != 0 && error_no() != XEINPROGRESS)
	{
		return -1;
	}

	return 0;
}
BASE_NAMESPACE_END_DECL

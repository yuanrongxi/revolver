#include "reciver_adapter.h"
#include "udp_handler.h"

ReciverAdapter::ReciverAdapter()
{

}

ReciverAdapter::~ReciverAdapter()
{

}

void ReciverAdapter::send(BinStream& strm, const Inet_Addr& remote_addr)
{
	if(RECV_UDP()->is_open())
		RECV_UDP()->send(strm, remote_addr);
}




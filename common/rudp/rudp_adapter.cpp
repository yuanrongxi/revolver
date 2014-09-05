#include "rudp/rudp_adapter.h"
#include "rudp/rudp_interface.h"

BASE_NAMESPACE_BEGIN_DECL

void IRUDPAdapter::on_data(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP()->process(this, strm, remote_addr);
}

BASE_NAMESPACE_END_DECL
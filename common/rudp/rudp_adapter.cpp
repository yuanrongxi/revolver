#include "rudp/rudp_adapter.h"
#include "rudp/rudp_interface.h"
#include "rudp/rudp_listener.h"
#include "rudp/rudp_handler.h"

BASE_NAMESPACE_BEGIN_DECL

void IRUDPAdapter::on_data(BinStream& strm, const Inet_Addr& remote_addr)
{
    RUDP()->process(this, strm, remote_addr);
}

void RudpAdapter::send(BinStream& strm, const Inet_Addr& remote_addr) {
    if (_handler && _handler->is_open())
    {
        _handler->send(strm, remote_addr);
    }
}

BASE_NAMESPACE_END_DECL
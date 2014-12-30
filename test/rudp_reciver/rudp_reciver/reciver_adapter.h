#ifndef __RECIVER_ADAPTER_H_
#define __RECIVER_ADAPTER_H_

#include "rudp/rudp_adapter.h"

using namespace BASE;

class ReciverAdapter : public IRUDPAdapter
{
public:
	ReciverAdapter();
	virtual ~ReciverAdapter();

	virtual void		send(BinStream& strm, const Inet_Addr& remote_addr);
};

#endif

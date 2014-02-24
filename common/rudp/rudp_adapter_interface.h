#ifndef __RUDP_ADAPTER_INTERFACE_H_
#define __RUDP_ADAPTER_INTERFACE_H_

#include "base_typedef.h"
#include "base_namespace.h"
#include "base_bin_stream.h"
#include "base_inet_addr.h"

//RUDPÊÊÅäÆ÷½Ó¿Ú
BASE_NAMESPACE_BEGIN_DECL

class IRUDPAdapter
{
public:
	IRUDPAdapter(){index_ = 0;};
	virtual ~IRUDPAdapter() {};

	uint8_t				get_index() const {return index_;};
	void				set_index(uint8_t index) {index_ = index;};

	virtual	void		on_data(BinStream& strm, const Inet_Addr& remote_addr) = 0;
	virtual void		send(BinStream& strm, const Inet_Addr& remote_addr) = 0;

protected:
	uint8_t				index_;
};

BASE_NAMESPACE_END_DECL

#endif

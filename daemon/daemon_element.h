#ifndef __DAEMON_ELEMENT_H
#define __DAEMON_ELEMENT_H

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_inet_addr.h"

#include <map>

using namespace BASE;

class CDaemonElement
{
public:
	CDaemonElement();
	~CDaemonElement();

	uint32_t get_server_id() const;
	uint8_t  get_server_type() const;
	uint16_t get_net_type() const;

	const Inet_Addr& get_tel_addr() const;
	const Inet_Addr& get_cnc_addr() const;

	uint32_t get_max_user() const {return max_user_;};
	void	 set_max_user(uint32_t max_user) {max_user_ = max_user;};

	void	 set_server_info(uint8_t stype, uint32_t sid, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr);
	void	 clear_info();

	void	 start_notify(uint32_t server_id, uint8_t server_type, uint16_t net_type, const Inet_Addr &tel_addr, const Inet_Addr &cnc_addr);
	void	 stop_notify(uint32_t server_id, uint8_t server_type);
	
	bool	 is_null() const { return null_;};

public:
	uint32_t	server_id_;
	uint8_t		server_type_;
	uint16_t	net_type_;
	uint32_t	max_user_;

	Inet_Addr	tel_addr_;
	Inet_Addr	cnc_addr_;

	bool		null_;
};

typedef map<uint32_t, CDaemonElement>	ServerElementMap;

#endif

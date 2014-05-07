/*************************************************************************************
*filename:	base_inet_addr.h
*
*to do:		定义IP地址封装类，实现IPV4地址的一些相关操作
*Create on: 2012-04
*Author:	zerok
*check list:
			2012-05-25 增加域名解析获取地址功能
*************************************************************************************/
#ifndef __BASE_INET_ADDR_H
#define __BASE_INET_ADDR_H

#include "revolver/base_namespace.h"
#include "revolver/base_os.h"
#include "revolver/base_bin_stream.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

class Inet_Addr
{
public:
	Inet_Addr();
	Inet_Addr(const Inet_Addr& addr);
	Inet_Addr(const string& hostname, uint16_t port);
	Inet_Addr(uint32_t ip, uint16_t port);
	Inet_Addr(const sockaddr_in& addr);

	virtual ~Inet_Addr();

	bool			is_null() const;
	Inet_Addr&		operator=(const Inet_Addr& addr);
	Inet_Addr&		operator=(const sockaddr_in& addr);
	Inet_Addr&		operator=(const string& addr_str);

	void			set_ip(uint32_t ip);
	void			set_ip(const string& hostname);
	void			set_port(uint16_t port);

	uint32_t		get_ip() const;
	uint16_t		get_port() const;
	const sockaddr_in& ipaddr() const;
	sockaddr_in*	get_addr();
	const sockaddr_in*	get_addr() const;
	string			to_string() const;
	string			ip_to_string() const;

	bool			operator==(const Inet_Addr& addr) const;
	bool			operator!=(const Inet_Addr& addr) const;
	bool			operator>(const Inet_Addr& addr) const;
	bool			operator<(const Inet_Addr& addr) const;

	friend std::ostream& operator<<(std::ostream& os, const Inet_Addr& addr);
	friend BinStream& operator<<(BinStream& os, const Inet_Addr& addr);
	friend BinStream& operator>>(BinStream& os, Inet_Addr& addr);
private:
	sockaddr_in		addr_;
};
BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/


#include "base_inet_addr.h"
#include <stdio.h>


BASE_NAMESPACE_BEGIN_DECL

Inet_Addr::Inet_Addr()
{
	memset(&addr_, 0, sizeof(struct sockaddr_in));
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_.sin_port = htons(0);
}

Inet_Addr::Inet_Addr(const sockaddr_in& addr)
{
	memcpy(&addr_, &addr, sizeof(struct sockaddr_in));
}

Inet_Addr::Inet_Addr(uint32_t ip, uint16_t port)
{
	memset(&addr_, 0, sizeof(struct sockaddr_in));

	set_ip(ip);
	set_port(port);

	addr_.sin_family = AF_INET;
}

Inet_Addr::Inet_Addr(const BASEOBJECT::Inet_Addr &addr)
{
	addr_ = addr.addr_;
}

Inet_Addr::Inet_Addr(const string& hostname, uint16_t port)
{
	memset(&addr_, 0, sizeof(struct sockaddr_in));

	set_ip(hostname);
	set_port(port);

	addr_.sin_family = AF_INET;
}

Inet_Addr::~Inet_Addr()
{
}

bool Inet_Addr::is_null() const
{
	return (ntohs(addr_.sin_port) == 0);
}

Inet_Addr& Inet_Addr::operator = (const Inet_Addr& addr)
{
	this->addr_ = addr.addr_;
	return *this;
}

Inet_Addr& Inet_Addr::operator =(const sockaddr_in &addr)
{
	this->addr_ = addr;
	return *this;
}

Inet_Addr& Inet_Addr::operator =(const std::string &addr_str)
{
	std::string::size_type pos = addr_str.find(':');
	if(std::string::npos == pos)
	{
		return *this;
	}
	
	set_port(static_cast<uint16_t>(strtoul(addr_str.substr(pos + 1).c_str(), NULL, 10)));
	set_ip(addr_str.substr(0, pos));

	return *this;
}

void Inet_Addr::set_ip(uint32_t ip)
{
	addr_.sin_addr.s_addr = htonl(ip);
}

void Inet_Addr::set_ip(const std::string &hostname)
{
	 addr_.sin_addr.s_addr = inet_addr(hostname.c_str());
	if (addr_.sin_addr.s_addr == -1) //地址为0.0.0.0，无效地址 
	{
		if (hostent * pHost = gethostbyname(hostname.c_str())) 
		{
			addr_.sin_addr.s_addr = (*reinterpret_cast<uint32_t *>(pHost->h_addr_list[0]));
		} 
	}
}

void Inet_Addr::set_port(uint16_t port)
{
	addr_.sin_port = htons(port);
}

uint32_t Inet_Addr::get_ip() const
{
	return ntohl(addr_.sin_addr.s_addr);
}

uint16_t Inet_Addr::get_port() const
{
	return ntohs(addr_.sin_port);
}

const sockaddr_in& Inet_Addr::ipaddr() const
{
	return addr_;
}

sockaddr_in* Inet_Addr::get_addr()
{
	return &addr_;
}

const sockaddr_in* Inet_Addr::get_addr() const
{
	return &addr_;
}

std::string Inet_Addr::to_string() const
{
	char ip_str[32] = {0};
#ifdef WIN32
	sprintf(ip_str, "%d.%d.%d.%d", addr_.sin_addr.S_un.S_un_b.s_b1,
		addr_.sin_addr.S_un.S_un_b.s_b2,
		addr_.sin_addr.S_un.S_un_b.s_b3,
		addr_.sin_addr.S_un.S_un_b.s_b4);
#else
	::inet_ntop(addr_.sin_family, &(addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

	sprintf(ip_str, "%s:%d", ip_str, get_port());

	return ip_str;
}

std::string Inet_Addr::ip_to_string() const
{
	char ip_str[32] = {0};
#ifdef WIN32
	sprintf(ip_str, "%d.%d.%d.%d", addr_.sin_addr.S_un.S_un_b.s_b1,
		addr_.sin_addr.S_un.S_un_b.s_b2,
		addr_.sin_addr.S_un.S_un_b.s_b3,
		addr_.sin_addr.S_un.S_un_b.s_b4);
#else
	::inet_ntop(addr_.sin_family, &(addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

	return ip_str;
}

bool Inet_Addr::operator ==(const Inet_Addr& addr) const
{
	return (ntohl(addr_.sin_addr.s_addr) == ntohl(addr.addr_.sin_addr.s_addr)
		&& ntohs(addr_.sin_port) == ntohs(addr.addr_.sin_port));
}

bool Inet_Addr::operator !=(const Inet_Addr &addr) const
{
	return !((*this) == addr);
}

bool Inet_Addr::operator <(const Inet_Addr &addr) const
{
	if(ntohl(addr_.sin_addr.s_addr) < ntohl(addr.addr_.sin_addr.s_addr))
		return true;
	else if(ntohl(addr_.sin_addr.s_addr) > ntohl(addr.addr_.sin_addr.s_addr))
		return false;
	else
	{
		return (ntohs(addr_.sin_port) < ntohs(addr.addr_.sin_port) ? true : false);
	}
}

bool Inet_Addr::operator >(const Inet_Addr& addr) const
{
	if(ntohl(addr_.sin_addr.s_addr) > ntohl(addr.addr_.sin_addr.s_addr))
		return true;
	else if(ntohl(addr_.sin_addr.s_addr) < ntohl(addr.addr_.sin_addr.s_addr))
		return false;
	else
	{
		return (ntohs(addr_.sin_port) > ntohs(addr.addr_.sin_port) ? true : false);
	}
}

std::ostream& operator<<(std::ostream& os, const Inet_Addr& addr)
{
	os << addr.to_string();
	return os;
}

BinStream& operator<<(BinStream& os, const Inet_Addr& addr)
{
	os << addr.get_ip();
	os << addr.get_port();

	return os;
}

BinStream& operator>>(BinStream& os, Inet_Addr& addr)
{
	uint32_t ip = 0;
	uint16_t port = 0;
	
	os >> ip >> port;
	addr.set_ip(ip);
	addr.set_port(port);

	return os;
}

BASE_NAMESPACE_END_DECL

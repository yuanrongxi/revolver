#ifndef __CORE_DAEMON_MSG_H
#define __CORE_DAEMON_MSG_H

#include "revolver/base_packet.h"
#include "core/core_server_type.h"
#include "core/core_message_map_decl.h"
#include "revolver/base_inet_addr.h"

BASE_NAMESPACE_BEGIN_DECL

#define DAEMON_REGISTER_REQ 0x01000001
class Daemon_Register_Req : public CBasePacket
{
public:
	Daemon_Register_Req() : tel_addr_("127.0.0.1", 8080), cnc_addr_("127.0.0.1", 8080)
	{
		stype_ = eDaemon_Server;
		sid_ = 0;
	};

	~Daemon_Register_Req()
	{

	}

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << stype_ << net_type_ << sid_ << tel_addr_ << cnc_addr_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> stype_ >> net_type_ >> sid_ >> tel_addr_ >> cnc_addr_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Daemon Register Req, {"\
			<<"sid_ = " << sid_ \
			<<", stype = " << (uint16_t)stype_ \
			<<", net type = " << (uint16_t)net_type_ \
			<<", cnc_addr = " <<  cnc_addr_\
			<<", tel_addr = " << tel_addr_\
			<<"}" << std::endl;
	};

public:
	uint8_t		stype_;
	uint16_t	net_type_;
	uint32_t	sid_;
	Inet_Addr	tel_addr_;
	Inet_Addr	cnc_addr_;
};

#define DAEMON_REGISTER_RES 0x01000002
class Daemon_Register_Res : public CBasePacket
{
public:
	Daemon_Register_Res()
	{
		sid_ = 0;
		net_type_ = 0x0000;
		stype_ = eDaemon_Server;
	};

	~Daemon_Register_Res()
	{

	}

	virtual void release_self()
	{
		delete this;
	};
protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << stype_ << net_type_ << sid_ << tel_addr_ << cnc_addr_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> stype_ >> net_type_ >> sid_ >> tel_addr_ >> cnc_addr_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Daemon Register Res, {"\
			<< "sid = " << sid_ \
			<<", stype = " << (uint16_t)stype_ \
			<<", net type = " << net_type_ \
			<<", tel_addr = " << tel_addr_ \
			<<", cnc_addr = " << cnc_addr_ \
			<<"}" << std::endl;
	}

public:
	uint32_t	sid_;
	uint8_t		stype_;
	uint16_t	net_type_;		
	Inet_Addr	tel_addr_;
	Inet_Addr	cnc_addr_;
};

#define DAEMON_ADD_SERVER 0x01000003
class Daemon_Add_Server : public CBasePacket
{
public:
	Daemon_Add_Server()
	{
		sid_ = 0;
		stype_ = eDaemon_Server;
	};

	~Daemon_Add_Server()
	{

	}

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << stype_ << net_type_ << sid_ << tel_addr_ << cnc_addr_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> stype_ >> net_type_ >> sid_ >> tel_addr_ >> cnc_addr_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Daemon Add Server, {"\
			<< "sid = " << sid_ \
			<<", stype = " << (uint16_t)stype_ \
			<<", net type = " << net_type_\
			<<", tel_addr_ = " << tel_addr_ \
			<<", cnc_addr_ = " << cnc_addr_\
			<<"}" << std::endl;
	}

public:
	uint32_t	sid_;
	uint8_t		stype_;
	uint16_t	net_type_;
	Inet_Addr	tel_addr_;
	Inet_Addr	cnc_addr_;
};

#define DAEMON_DEL_SERVER 0x01000004
class Daemon_Del_Server : public CBasePacket
{
public:
	Daemon_Del_Server()
	{
		sid_ = 0;
		stype_ = eDaemon_Server;
	};

	~Daemon_Del_Server()
	{

	}

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << stype_ << sid_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> stype_ >> sid_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Daemon Del Server, {"\
			<< "sid = " << sid_ \
			<<", stype = " << (uint16_t)stype_ \
			<<"}" << std::endl;
	}

public:
	uint32_t	sid_;
	uint8_t		stype_;

};

#define DAEMON_STUN_PING	0x01000005
class Daemon_Stun_Ping : public CBasePacket
{
public:
	Daemon_Stun_Ping()
	{
		optype_ = 0;
	};

	~Daemon_Stun_Ping()
	{

	};

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << optype_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> optype_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Stun Ping, {"\
			<<", otype = " << (uint16_t)optype_ \
			<<"}" << std::endl;
	}

public:
	uint8_t		optype_;	//要求的返回类型，暂时定为公网源地址
};

#define DAEMON_STUN_PONG	0x0100006
class Daemon_Stun_Pong : public CBasePacket
{
public:
	Daemon_Stun_Pong()
	{

	};

	~Daemon_Stun_Pong()
	{

	};

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << wan_addr_.get_ip() << wan_addr_.get_port();
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		uint32_t ip;
		uint16_t port;
		strm >> ip >> port;
		wan_addr_.set_ip(ip);
		wan_addr_.set_port(port);
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Stun Pong, {"\
			<<", wan_addr_ = " << wan_addr_ \
			<<"}" << std::endl;

	};

public:
	Inet_Addr	wan_addr_;
};

#define DAEMON_NODE_STATE	0x0100007
class Daemon_Node_State : public CBasePacket
{
public:
	Daemon_Node_State()
	{

	};

	~Daemon_Node_State()
	{

	};

	virtual void release_self()
	{
		delete this;
	};

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << node_state_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> node_state_;
	};

	virtual void	Print(std::ostream& os) const
	{
		os << "Stun Pong, {"\
			<<", node state = " << node_state_ \
			<<"}" << std::endl;

	};

public:
	string	node_state_;
};


//消息体映射关系
MESSAGEMAP_DECL_BEGIN(DAEMON)
MESSAGEMAP_REGISTER(DAEMON_REGISTER_REQ, new Daemon_Register_Req())
MESSAGEMAP_REGISTER(DAEMON_REGISTER_RES, new Daemon_Register_Res())
MESSAGEMAP_REGISTER(DAEMON_ADD_SERVER, new Daemon_Add_Server())
MESSAGEMAP_REGISTER(DAEMON_DEL_SERVER, new Daemon_Del_Server())
MESSAGEMAP_REGISTER(DAEMON_STUN_PING, new Daemon_Stun_Ping())
MESSAGEMAP_REGISTER(DAEMON_STUN_PONG, new Daemon_Stun_Pong())
MESSAGEMAP_REGISTER(DAEMON_NODE_STATE, new Daemon_Node_State())
MESSAGEMAP_DECL_END()

BASE_NAMESPACE_END_DECL

#endif

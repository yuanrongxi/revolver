#include "core_packet.h"
#include "daemon_log.h"
#include "core_daemon_msg.h"
#include "daemon_element.h"
#include "core_local_info.h"
#include "core_connection_manager.h"

CDaemonElement::CDaemonElement()
{
	server_id_ = 0;
	server_type_ = 0;
	net_type_ = 0x0000;
	max_user_ = 1000;

	null_ = true;
}

CDaemonElement::~CDaemonElement()
{

}

void CDaemonElement::set_server_info(uint8_t stype, uint32_t sid, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr)
{
	server_id_ = sid;
	server_type_ = stype;
	net_type_ = net_type;
	tel_addr_ = tel_addr;
	cnc_addr_ = cnc_addr;

	null_ = false;
}

void CDaemonElement::clear_info()
{
	null_ = true;
}

uint8_t CDaemonElement::get_server_type() const
{
	return server_type_;
}

uint32_t CDaemonElement::get_server_id() const
{
	return server_id_;
}

uint16_t CDaemonElement::get_net_type() const
{
	return net_type_;
}

const Inet_Addr& CDaemonElement::get_tel_addr() const
{
	return tel_addr_;
}

const Inet_Addr& CDaemonElement::get_cnc_addr() const
{
	return cnc_addr_;
}

void CDaemonElement::start_notify(uint32_t server_id, uint8_t server_type, uint16_t net_type, const Inet_Addr &tel_addr, const Inet_Addr &cnc_addr)
{
	//将通告发送给自己
	INIT_CORE_REQUEST(self_packet, DAEMON_ADD_SERVER);

	Daemon_Add_Server self_add_server;
	self_add_server.sid_ = server_id;
	self_add_server.stype_ = server_type;
	self_add_server.net_type_ = net_type;
	self_add_server.tel_addr_ = tel_addr;
	self_add_server.cnc_addr_ = cnc_addr;

	self_packet.set_body(self_add_server);
	SendDispathByID(self_packet, server_id_);

	DAEMON_INFO("send DAEMON_ADD_SERVER, to server id = " << server_id_ << ", server type = " << GetServerName(server_type_)\
		<< "{packet.sid = " <<server_id << ", packet.stype = " <<  GetServerName(server_type) << ", net type = " << net_type << ", packet.tel_addr = " << tel_addr  \
		<< ", packet.cnc_addr = " << cnc_addr <<"}");

	//将通告发送给伙伴

	INIT_CORE_REQUEST(peer_packet, DAEMON_ADD_SERVER);

	Daemon_Add_Server peer_add_server;
	peer_add_server.sid_ = server_id_;
	peer_add_server.stype_ = server_type_;
	peer_add_server.net_type_ = net_type_;
	peer_add_server.tel_addr_ = tel_addr_;
	peer_add_server.cnc_addr_ = cnc_addr_;

	peer_packet.set_body(peer_add_server);
	SendDispathByID(peer_packet, server_id);

	DAEMON_INFO("send DAEMON_ADD_SERVER, to server id = " << server_id << ", server type = " << GetServerName(server_type) \
		<< "{packet.sid = " <<server_id_ << ", packet.stype = " <<  GetServerName(server_type_) << ", net type = " << net_type_ \
		<< ", tel_addr = " << tel_addr_ << ", packet.cnc_addr = " << cnc_addr_ <<"}");
}

void CDaemonElement::stop_notify(uint32_t server_id, uint8_t server_type)
{
	//将通告发送给自己
	INIT_CORE_REQUEST(self_packet, DAEMON_DEL_SERVER);

	Daemon_Del_Server self_del_server;
	self_del_server.sid_ = server_id;
	self_del_server.stype_ = server_type;

	DAEMON_INFO("send DAEMON_DEL_SERVER, to server id = " << server_id_ << ", server type = " << GetServerName(server_type_) \
		<< "{packet.sid = " <<server_id << ", packet.stype = " <<  GetServerName(server_type) << "}");

	self_packet.set_body(self_del_server);
	SendDispathByID(self_packet, server_id_);
}




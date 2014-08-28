#include "core/core_local_info.h"
#include "core/core_event_message.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_server_type.h"
#include "##var_prefix#_server.h"
#include "##var_prefix#_log.h"
#include "##var_prefix#_frame.h"

#define ##marco_prefix#_HEARTBEAT_TIMER 30000

BEGIN_MSGCALL_MAP(##class_prefix#Server)
END_MSGCALL_MAP()

##class_prefix#Server::##class_prefix#Server() : _heartbeat_timer_id(0) {
}


##class_prefix#Server::~##class_prefix#Server() {
}

void ##class_prefix#Server::init() {
}

void ##class_prefix#Server::destroy() {
}

int32_t ##class_prefix#Server::on_connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection) {
  SRV_INFO("on tcp connect event, sid = " << sid << ", stype = "
    << GetServerName(connection->get_server_type()));
  return 0;
}

int32_t ##class_prefix#Server::on_disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection) {
  SRV_INFO("on disconnect event, sid = " << sid <<
    ", stype = " << GetServerName(connection->get_server_type()));
  return 0;
}

void ##class_prefix#Server::on_register(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr) {
  SRV_INFO("on_register sid = " << sid << ", stype = " << GetServerName(stype)
           << ", tel addr = " << tel_addr << ", cnc addr = " << cnc_addr);

  //启动第一次收到登记结果，如果timer id != 0表示有可能是本服务与daemond之间TCP断开后进行了重新连接！！！
  if(_heartbeat_timer_id == 0) {
    //保存地址信息
    if(CHECK_CNC_NETTYPE(net_type))
      SERVER_PORT = cnc_addr.get_port();
    else
      SERVER_PORT = tel_addr.get_port();

    SERVER_NET_TYPE = net_type;

    SERVER_ID = sid;
    TEL_IPADDR = tel_addr;
    CNC_IPADDR = cnc_addr;

    //绑定服务端口，包括tcp udp监听端口
    ##marco_prefix#_FRAME()->bind_port(SERVER_PORT);

    //设置心跳事件
    set_timer();
  }
}

void ##class_prefix#Server::on_add_server(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr) {
  SRV_INFO("on add server, sid = " << sid << ", stype = " << GetServerName(stype));
}

void ##class_prefix#Server::on_del_server(uint32_t sid, uint8_t stype) {
  SRV_WARNING("on_del_server sid = " << sid << ", stype = " << GetServerName(stype));
}

void ##class_prefix#Server::on_server_dead(uint32_t sid, uint8_t stype) {
  SRV_WARNING("on_server_dead, sid = " << sid << ", stype = " << GetServerName(stype));
}

int32_t ##class_prefix#Server::handle_timeout(const void* act, uint32_t timer_id) {
  if(_heartbeat_timer_id == timer_id) {
    heartbeat();
  }
  return 0;
}

void ##class_prefix#Server::set_timer() {
  if(_heartbeat_timer_id == 0)
    _heartbeat_timer_id = REACTOR_INSTANCE()->set_timer(this, NULL, ##marco_prefix#_HEARTBEAT_TIMER);
}

void ##class_prefix#Server::cancel_timer() {
  if(_heartbeat_timer_id > 0) {
    const void* act = NULL;
    REACTOR_INSTANCE()->cancel_timer(_heartbeat_timer_id, &act);

    _heartbeat_timer_id = 0;
  }
}

void ##class_prefix#Server::release_timer_act(const void* act) {
}

void ##class_prefix#Server::heartbeat() {
}
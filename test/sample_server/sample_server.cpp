#include "core_local_info.h"
#include "sample_server.h"
#include "core_event_message.h"
#include "sample_frame.h"
#include "base_reactor_instance.h"
#include "core_server_type.h"
#include "sample_log_macro.h"
#include "sample_msg.h"

using namespace SAMPLE_SERVER;
//30秒心跳一次
#define HEARTBEAT_TIMER 30000

//定义消息处理函数
BEGIN_MSGCALL_MAP(SampleServer)
	ON_MSG_CON_INT(TCP_CONNECT_EVENT, &SampleServer::on_connect_event)
	ON_MSG_CON_INT(TCP_CLOSE_EVENT, &SampleServer::on_disconnect_event)
	ON_MSG_CON_INT(SAMPLE_MSGID, &SampleServer::on_sample_msg)
END_MSGCALL_MAP()

SampleServer::SampleServer() : timer_id_(0)
{

}

SampleServer::~SampleServer()
{
}

void SampleServer::init()
{

}

void SampleServer::destroy()
{
	cancel_timer();
	//清除掉所有关联到本实例的定时器
	clear_timer_events();
}

void SampleServer::on_register(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr)
{
	SAM_INFO("on_register sid = " << sid << ", stype = " << GetServerName(stype) << ", tel addr = " << tel_addr << ", cnc addr = " << cnc_addr);

	//启动第一次收到登记结果，如果timer id != 0表示有可能是本服务与daemond之间TCP断开后进行了重新连接！！！
	if(timer_id_ == 0)
	{
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
		SAMPLE_FRAME()->bind_port(SERVER_PORT);

		//设置心跳事件
		set_timer();
	}
}

void SampleServer::on_add_server(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr)
{
	SAM_INFO("on add server, sid = " << sid << ", stype = " << GetServerName(stype));
	//本服务感知的服务单元启动，可以在此处理服务启动后的工作

}

void SampleServer::on_del_server(uint32_t sid, uint8_t stype)
{
	SAM_WARNING("on_del_server sid = " << sid << ", stype = " << GetServerName(stype));

	//本服务感知的服务与daemond断开
}

void SampleServer::on_server_dead(uint32_t sid, uint8_t stype)
{
	SAM_WARNING("on_server_dead, sid = " << sid << ", stype = " << GetServerName(stype));

	//本服务感知的服务停止运行

	//TODO:在此处理感知服务停止运行！！！！！
}

int32_t SampleServer::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id_ == timer_id)
	{
		heartbeat();
	}

	return 0;
}

void SampleServer::heartbeat()
{
	//TODO:处理心跳事件
}

void SampleServer::release_timer_act(const void* act)
{
	//TODO:在此释放set_timer函数传入的act参数
}

void SampleServer::set_timer()
{
	if(timer_id_ == 0)
		timer_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, HEARTBEAT_TIMER);
}

void SampleServer::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);

		timer_id_ = 0;
	}
}

int32_t SampleServer::on_connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	SAM_INFO("on tcp connect event, sid = " << sid << ", stype = " << GetServerName(connection->get_server_type()));

	//TODO:处理TCP已经连接的事件
	return 0;
}

int32_t SampleServer::on_disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	SAM_INFO("on disconnect event, sid = " << sid << ", stype = " << GetServerName(connection->get_server_type()));

	//TODO:处理TCP连接点开的事件
	return 0;
}

int32_t SampleServer::on_sample_msg(CBasePacket* packet, uint32_t sid, CConnection* connection)
{
	SAM_INFO("on sample msg!!");

	CSamplePacket* body = (CSamplePacket *)packet;

	cout << "msg body = " << *body << endl;

	return 0;
}





#ifndef __SAMPLE_SERVER_H
#define __SAMPLE_SERVER_H

#include "core_msg_.h"
#include "core_cmd_target.h"
#include "core_daemon_event.h"
#include "base_event_handler.h"
#include "core_tcp_disconnect_notify.h"

using namespace BASE;

//消息处理器，可以设置多个
class SampleServer : public ICmdTarget,			//消息处理接口
					 public IDaemonEvent,		//daemon client事件接口
					 public ICoreServerNotify,	//服务断线接口
					 public CEventHandler		//定时器handler
{
public:
	SampleServer();
	~SampleServer();

public:
	void				init();
	void				destroy();

public:
	//TCP连接和断开信号处理
	int32_t				on_connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection);
	int32_t				on_disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection);

	//处理SAMPLE_MSGID
	int32_t				on_sample_msg(CBasePacket* packet, uint32_t sid, CConnection* connection);

	//DAEMON CLIENT事件
	//登记daemond事件，获取到sid, net type, 地址信息等通告
	void				on_register(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr);
	//本服务感知的服务单元启动
	void				on_add_server(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr);
	//本服务感知的服务与daemond断开，不可作为服务单元断网的依据
	void				on_del_server(uint32_t sid, uint8_t stype);

	//服务器停止服务或者断线通告，作为服务单元停止服务的依据
	void				on_server_dead(uint32_t sid, uint8_t stype);

	//超时事件
	int32_t				handle_timeout(const void *act, uint32_t timer_id);

protected:
	void				set_timer();
	void				cancel_timer();

	//定时器参数释放接口
	void				release_timer_act(const void* act);

	void				heartbeat();

protected:
	//定时器的ID
	uint32_t			timer_id_;

protected:
	//消息映射回调容器定义
	DECLARE_MSGCALL_MAP()
};
#endif

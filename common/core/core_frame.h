/*************************************************************************************
*filename:	core_frame.h
*
*to do:		定义CORE的框架，主要是本地节点信息管理、UDP/TCP监听管理、Daemon Client启动
			管理
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_FRAME_H
#define __CORE_FRAME_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "core/core_daemon_client.h"
#include "core/core_dc_client.h"

BASE_NAMESPACE_BEGIN_DECL

class CoreUDPHandler;
class CCoreTCPListener;
class ICoreServerNotify;

class ICoreFrame
{
public:
	ICoreFrame();
	~ICoreFrame();

	void				init();
	void				destroy();

	void				start(bool wan = false);
	void				stop();

	void				release_handler();

	//CORE库的运行函数
	void				frame_run();
	
	//DAEMON CLIENT返回分配好的地址，进行网络绑定，如果是DAEMON
	void				bind_port(uint16_t port);

	//组件设置
	void				create_udp();
	void				create_tcp_listener();
	void				create_daemon_client(IDaemonEvent* daemon_event, IDaemonConfig* config = NULL);
	void				create_dc_client();
	void				attach_server_notify(ICoreServerNotify* notify);

	CoreUDPHandler*		get_udp_handler(){return udp_handler_;};
	
	uint32_t			post_write_php(uint32_t dc_sid, CoreDCParam* param, const string& php, const string& php_param, bool ack = true);
	uint32_t			post_read_php(uint32_t dc_sid, CoreDCParam* param, const string& php, const string& php_param, bool ack = true);

	CoreDCParam*		cancel_dc_request(uint32_t exc_id);
	//报告节点状态到daemond服务上
	int32_t				post_node_state(const string& node_info);

	//提供给上层的事件
	virtual void		on_init() = 0;
	virtual void		on_destroy() = 0;
	
	virtual void		on_start() = 0;
	virtual void		on_stop() = 0;
	
	//检查子进程是否被退出
	virtual void		wait_pid(){};

protected:
	CDaemonClient*		daemon_client_;
	CCoreTCPListener*	listener_;
	CoreUDPHandler*		udp_handler_;
	CCoreDCClient*		dc_client_;
	bool				single_thread_; //	全单线程设计
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/



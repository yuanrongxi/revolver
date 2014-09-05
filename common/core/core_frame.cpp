#include "core/core_frame.h"
#include "core/core_udp_handler.h"
#include "core/core_connection.h"
#include "core/core_tcp_listener.h"
#include "core/core_daemon_client.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_event_message.h"
#include "core/core_message_processor.h"
#include "core/core_message_map_decl.h"
#include "core/core_local_info.h"
#include "core/core_reactor_thread.h"
#include "core/core_connection_manager.h"
#include "revolver/base_log_thread.h"
#include "revolver/base_log.h"
#include "core/core_bandwidth_throttler.h"
#include "core/core_main.h"
#include "revolver/lzo_object.h"

BASE_NAMESPACE_BEGIN_DECL

ICoreFrame::ICoreFrame()
{
	daemon_client_ = NULL;
	listener_= NULL;
	udp_handler_ = NULL;
	dc_client_ = NULL;

	single_thread_ = false;
}

ICoreFrame::~ICoreFrame()
{

}

void ICoreFrame::release_handler()
{
	if(daemon_client_)
	{
		delete daemon_client_;
		daemon_client_ = NULL;
	}

	if(listener_)
	{
		delete listener_;
		listener_  = NULL;
	}

	if(udp_handler_)
	{
		delete udp_handler_;
		udp_handler_ = NULL;
	}

	if(dc_client_)
	{
		dc_client_->clear();

		delete dc_client_;
		dc_client_ = NULL;
	}
}

void ICoreFrame::init()
{
	init_socket();
	//屏蔽信号
	ignore_pipe();

	//创建LZO压缩对象
	LZO_CREATE();
	//启动LOG系统
	LOG_CREATE();
	LOG_THREAD_CREATE();

	CREATE_LOCAL_SERVER();

	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(20000);

	CREATE_REACTOR_THREAD();

	//建立消息映射关系
	CREATE_MSG_PROCESSOR();

	//创建连接管理器
	CREATE_CONN_MANAGER();

	//建立TCP通告消息体映射
	LOAD_MESSAGEMAP_DECL(TCP_EVENT);
	
	CREATE_THROTTLER();
	//调用上层初始化接口
	on_init();
	//启动SELECT线程
	if(!single_thread_)
	{
		REACTOR_THREAD()->start();
		LOG_THREAD_INSTANCE()->start();
	}
}

void ICoreFrame::destroy()
{
	REACTOR_THREAD()->terminate();

	CONN_MANAGER()->close_all(); 
	
	usleep(50000);

	//停止所有服务
	stop();

	//停止线程
	if(!single_thread_)
	{
		LOG_THREAD_INSTANCE()->terminate();
	}

	//删除REACTOR
	REACTOR_INSTANCE()->close_reactor();

	//销毁消息关联
	DESTROY_MESSAGE_MAP_DECL();
	//释放插件句柄
	release_handler();
	on_destroy();

	DESTROY_THROTTLER();

	//删除连接管理
	DESTROY_CONN_MANAGER();
	//删除线程对象
	DESTROY_REACTOR_THREAD();
	//删除MSG_处理器
	DESTROY_MSG_PROCESSOR();
	//删除全局节点信息
	DESTROY_LOCAL_SERVER();
	//关闭LOG系统
	LOG_THREAD_DESTROY();
	LOG_DESTROY();
	
	REACTOR_DESTROY();
	//释放压缩对象
	LZO_DESTROY();

	destroy_socket();
}

void ICoreFrame::start(bool wan)
{
	if(daemon_client_)
	{
		daemon_client_->start(wan);
	}

	on_start();
}

void ICoreFrame::stop()
{
	on_stop();

	if(daemon_client_)
	{
		daemon_client_->stop();
	}

	if(listener_)
	{
		listener_->close();
	}

	if(udp_handler_)
	{
		udp_handler_->close();
	}
}

void ICoreFrame::bind_port(uint16_t port)
{
	Inet_Addr local_addr(INADDR_ANY, port);

	if(listener_ && !listener_->is_open())
	{
		listener_->open(local_addr);
	}

	if(udp_handler_ && !udp_handler_->is_open())
	{
		udp_handler_->open(local_addr);
	}
}

void ICoreFrame::create_daemon_client(IDaemonEvent * daemon_event, IDaemonConfig* config)
{
	if(daemon_client_ == NULL)
	{
		daemon_client_ = new CDaemonClient();
		daemon_client_->attach_event(daemon_event, config);
	}
}

void ICoreFrame::create_udp()
{
	if(udp_handler_ == NULL)
	{
		udp_handler_ = new CoreUDPHandler();
		udp_handler_->attach(MSG_PROCESSOR());

		CONN_MANAGER()->attach_udp(udp_handler_);
	}
}

void ICoreFrame::create_tcp_listener()
{
	if(listener_ == NULL)
		listener_ = new CCoreTCPListener();
}

void ICoreFrame::create_dc_client()
{
	if(dc_client_ == NULL)
	{
		dc_client_ = new CCoreDCClient();
	}
}

void ICoreFrame::attach_server_notify(ICoreServerNotify* notify)
{
	AttachServerNotify(notify);
}

void ICoreFrame::frame_run()
{
	if(single_thread_) //单线程模式
	{
		while(true)
		{
			REACTOR_INSTANCE()->event_loop();	
			wait_pid();
		}

		REACTOR_INSTANCE()->stop_event_loop();
	}
	else
	{
		core_main();
	}
}

uint32_t ICoreFrame::post_read_php(uint32_t dc_sid, CoreDCParam* param, const string& php, const string& php_param, bool ack /* = true */)
{
	uint32_t ret = 0;
	if(dc_client_ != NULL)
	{
		ret = dc_client_->post_dc_request(dc_sid, param, php, php_param, true, ack);
	}

	return ret;
}

uint32_t ICoreFrame::post_write_php(uint32_t dc_sid, CoreDCParam* param, const string& php, const string& php_param, bool ack /* = true */)
{
	uint32_t ret = 0;
	if(dc_client_ != NULL)
	{
		ret = dc_client_->post_dc_request(dc_sid, param, php, php_param, false, ack);
	}

	return ret;
}

CoreDCParam* ICoreFrame::cancel_dc_request(uint32_t exc_id)
{
	CoreDCParam* ret = NULL;
	if(dc_client_ != NULL)
	{
		ret = dc_client_->cancel_dc_request(exc_id);
	}

	return ret;
}

int32_t ICoreFrame::post_node_state(const string& node_info)
{
	int32_t ret = -1;
	if(daemon_client_ != NULL)
	{
		ret = daemon_client_->send_node_state(node_info);
	}

	return ret;
}


BASE_NAMESPACE_END_DECL


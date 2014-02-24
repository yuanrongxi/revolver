#include "core_server_type.h"
#include "sample_frame.h"
#include "core_local_info.h"
#include "core_message_processor.h"
#include "core_message_map_decl.h"
#include "sample_msg.h"

using namespace SAMPLE_SERVER;

SampleFrame::SampleFrame()
{

}

SampleFrame::~SampleFrame()
{

}

void SampleFrame::on_init()
{
	//设置节点类型
	SERVER_TYPE = eSample_Server;

	//加载daemon client插件
	create_daemon_client(&sample_server_, &config_);

	//加载TCP监听服务
	create_tcp_listener();

	//增加服务器连接状态通告接口
	attach_server_notify(&sample_server_);

	//设置关联服务, add_focus参数的是本服务需要感知的服务，如果设置了此项，对方服务启动和关闭，将会通过特定事件通知过来，具体可以看SampleServer的实现
	//daemon_client_->add_focus(eData_Center);

	//设置消息处理器
	INIT_MSG_PROCESSOR1(&sample_server_);

	//设置要处理的消息群体
	LOAD_MESSAGEMAP_DECL(SAMPLE_MSG);
	
	//初始化服务处理器
	sample_server_.init();

	//启动daemon client
	daemon_client_->init();
}

void SampleFrame::on_destroy()
{
	//撤销服务处理器
	sample_server_.destroy();
}

void SampleFrame::on_start()
{

}

void SampleFrame::on_stop()
{

}

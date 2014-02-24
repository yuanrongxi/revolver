/*************************************************************************************
*filename:	sample_frame.h
*
*to do:		定义一个简易的服务，使用core层的IFrame接口，可以作为例子参考
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __SAMPLE_FRAME_H
#define __SAMPLE_FRAME_H

#include "core_frame.h"
#include "base_singleton.h"
#include "daemon_config.h"
#include "sample_server.h"

//使用revolver的命名空间
using namespace BASE;

class SampleFrame : public ICoreFrame
{
public:
	SampleFrame();
	~SampleFrame();

	//服务初始化事件，由core层的Frame模块触发
	virtual void on_init();
	//服务撤销事件，由core层的Frame模块触发
	virtual void on_destroy();
	//服务的启动事件
	virtual void on_start();
	//服务的停止事件
	virtual void on_stop();

protected:
	//服务主体,里面实现消息隐射、服务检测等
	SampleServer	sample_server_;

	//服务的配置接口，主要记录第一次确定的sid, server type等信息，用JSON格式保存到文件中
	CDaemonConfig	config_;
};

#define CREATE_SAMPLE_FRAME		CSingleton<SampleFrame>::instance
#define SAMPLE_FRAME			CSingleton<SampleFrame>::instance
#define DESTROY_SAMPLE_FRAME	CSingleton<SampleFrame>::destroy

#endif
/************************************************************************************/


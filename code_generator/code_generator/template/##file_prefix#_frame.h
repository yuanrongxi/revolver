#ifndef __##marco_prefix#_FRAME_H__
#define __##marco_prefix#_FRAME_H__

#include "core/core_frame.h"
#include "revolver/base_singleton.h"
#include "daemon_config.h"
#include "##var_prefix#_server.h"

using namespace BASE;

class ##class_prefix#Frame : public ICoreFrame {
public:
  ##class_prefix#Frame();
  ~##class_prefix#Frame();

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
  ##class_prefix#Server 	_server;

  //服务的配置接口，主要记录第一次确定的sid, server type等信息，用JSON格式保存到文件中
  CDaemonConfig  _daemon_config;
};

#define CREATE_##marco_prefix#_FRAME		CSingleton<##class_prefix#Frame>::instance
#define ##marco_prefix#_FRAME			CSingleton<##class_prefix#Frame>::instance
#define DESTROY_##marco_prefix#_FRAME	CSingleton<##class_prefix#Frame>::destroy

#endif // __##marco_prefix#_FRAME_H__

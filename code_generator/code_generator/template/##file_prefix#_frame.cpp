#include "core/core_server_type.h"
#include "core/core_frame.h"
#include "core/core_local_info.h"
#include "core/core_message_processor.h"
#include "core/core_message_map_decl.h"
#include "##var_prefix#_frame.h"
#include "##var_prefix#_log.h"

##class_prefix#Frame::##class_prefix#Frame() {
}


##class_prefix#Frame::~##class_prefix#Frame() {
}

void ##class_prefix#Frame::on_init() {
  //设置节点类型
  SERVER_TYPE = eMediaProxy;

  //加载daemon client插件
  create_daemon_client(&_server, &_daemon_config);

  //加载TCP监听服务
  create_tcp_listener();

  //增加服务器连接状态通告接口
  attach_server_notify(&_server);

  //设置关联服务, add_focus参数的是本服务需要感知的服务，如果设置了此项，对方服务启动和关闭，将会通过特定事件通知过来，具体可以看SampleServer的实现
  //daemon_client_->add_focus(eData_Center);

  //config msg processor.
  INIT_MSG_PROCESSOR1(&_server);

  //set processing msg
  //LOAD_MESSAGEMAP_DECL(SAMPLE_MSG);

  //initialize server
  _server.init();

  //start daemon client
  daemon_client_->init();
}

void ##class_prefix#Frame::on_destroy() {
  //撤销服务处理器
  _server.destroy();
}

void ##class_prefix#Frame::on_start() {

}

void ##class_prefix#Frame::on_stop() {

}
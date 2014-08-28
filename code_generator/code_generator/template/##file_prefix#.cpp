#include "##file_prefix#_frame.h"

int main(int argc, char* argv[]) {
  //创建一个服务框架
  CREATE_##marco_prefix#_FRAME();

  //初始化框架
  ##marco_prefix#_FRAME()->init();

  //启动服务
  ##marco_prefix#_FRAME()->start();

  //主线程等待，在WINDOWS下按e键退出，在LINUX下kill -41 进程ID 退出
  ##marco_prefix#_FRAME()->frame_run();

  //撤销服务
 ##marco_prefix#_FRAME()->destroy();

  //销毁服务框架对象
  DESTROY_##marco_prefix#_FRAME();

  return 0;
}
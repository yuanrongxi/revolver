#include "sample_frame.h"

int main(int argc, const char* argv[])
{
	//创建一个服务框架
	CREATE_SAMPLE_FRAME();
	
	//初始化框架
	SAMPLE_FRAME()->init();

	//启动服务
	SAMPLE_FRAME()->start();

	//主线程等待，在WINDOWS下按e键退出，在LINUX下kill -41 进程ID 退出
	SAMPLE_FRAME()->frame_run();

	//撤销服务
	SAMPLE_FRAME()->destroy();

	//销毁服务框架对象
	DESTROY_SAMPLE_FRAME();
}

#ifndef __DAEMON_FRAME_H
#define __DAEMON_FRAME_H

#include "core_frame.h"
#include "base_singleton.h"
#include "daemon_server.h"
#include "daemon_simple_physical.h"

using namespace BASE_NAMEPSACE_DECL;

class CDaemonFrame : public ICoreFrame
{
public:
	CDaemonFrame();
	~CDaemonFrame();

	//提供给上层的事件
	virtual void on_init();
	virtual void on_destroy();

	virtual void on_start();
	virtual void on_stop();

private:
	CDaemonServer daemon_;
	DaemonServerDB db_;
};

#define CREATE_DAEMON_FRAME		CSingleton<CDaemonFrame>::instance
#define DAEMON_FRAME			CSingleton<CDaemonFrame>::instance
#define DESTROY_DAEMON_FRAME	CSingleton<CDaemonFrame>::destroy

#endif


#include "daemon_frame.h"
#include "core_daemon_msg.h"
#include "core_local_info.h"
#include "core_server_type.h"
#include "core_message_processor.h"

#define DAEMON_SERVER_CNC_IP	"daemon.5211game.com"
#define DAEMON_SERVER_TEL_IP	"daemon.5211game.com"
#define DAEMON_SERVER_PORT		7600

CDaemonFrame::CDaemonFrame() : db_(&daemon_)
{

}

CDaemonFrame::~CDaemonFrame()
{

}

void CDaemonFrame::on_init()
{
	//设置DAEMON节点的全局信息
	SERVER_ID = 1;
	SERVER_TYPE = eDaemon_Server;
	//默认设置为双线
	SERVER_NET_TYPE = 0x00ff;

	SERVER_PORT = DAEMON_SERVER_PORT;
	TEL_IPADDR.set_ip(DAEMON_SERVER_CNC_IP); //可以直接用域名
	TEL_IPADDR.set_port(SERVER_PORT);
	CNC_IPADDR.set_ip(DAEMON_SERVER_TEL_IP);
	CNC_IPADDR.set_port(SERVER_PORT);

	//增加一个TCP服务插件
	create_tcp_listener();
	create_udp();
	create_dc_client();

	//增加DAEMON消息处理映射
	INIT_MSG_PROCESSOR1(&daemon_);

	LOAD_MESSAGEMAP_DECL(DAEMON);
}

void CDaemonFrame::on_destroy()
{

}

void CDaemonFrame::on_start()
{
	//直接绑定一个服务端口
	bind_port(SERVER_PORT);

	daemon_.init(&db_);
}

void CDaemonFrame::on_stop()
{
	daemon_.destroy();
}




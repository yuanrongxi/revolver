#include "revolver/base_reactor_instance.h"
#include "reciver_thread.h"
#include "udp_handler.h"
#include "rudp_listen_handler.h"
#include "rudp/rudp_interface.h"
#include "rudp_connection.h"
#include "stat_packet.h"

#include <iostream>


using namespace std;

void init()
{
	init_socket();

	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(20000);

	init_rudp_socket();

	CREATE_RECV_THREAD();
	CREATE_RECV_UDP();

	Inet_Addr local_addr(INADDR_ANY, 2030);
	RECV_UDP()->open(local_addr);

	//创建一个监听者
	CREATE_RUDP_LISTEN();
	RUDP()->attach_listener(RUDP_LISTEN());

	RECV_THREAD()->start();

}

void destroy()
{
	RECV_THREAD()->terminate();

	RUDP()->attach_listener(NULL);
	RUDP()->destroy();

	RECV_UDP()->close();

	REACTOR_INSTANCE()->close_reactor();

	DESTROY_RECV_UDP();
	DESTROY_RUDP_LISTEN();
	destroy_rudp_socket();

	DESTROY_RECV_THREAD();
	REACTOR_DESTROY();
	destroy_socket();
}

int main(int agc, char* argv[])
{
	cout << "start rudp sender... " << endl;

	init();

	StatPacketObj	stat;

	RUDPConnection *conn = NULL;
	while(true)
	{
		char c = getchar();

		if(c == 'e')
		{
			if(conn != NULL && conn->get_state() != RUDP_CONN_IDLE)
				conn->close();

			break;
		}
		else if(c == 'c')
		{
			Inet_Addr local_addr(INADDR_ANY, 2030);	//本地绑定的RUDP监听端口
			Inet_Addr remote_addr("10.1.6.240", 2020);	//远端帮顶的rudp监听端口,rudp_recv的地址!!!!!!

			//for(int32_t i = 0; i < 1; i ++)
			{
				conn = new RUDPConnection();
				conn->set_stat(&stat);

				conn->connect(local_addr, remote_addr);
			}
		}
	}

	destroy();

	return 0;
}

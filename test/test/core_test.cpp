#include "core_test.h"
#include "core_msg_.h"
#include "core_message_map_decl.h"
#include "core_cmd_target.h"
#include "core_packet.h"
#include "core_message_processor.h"
#include "core_reactor_thread.h"
#include "core_udp_handler.h"
#include "base_reactor_instance.h"
#include "core_event_message.h"
#include "core_tcp_listener.h"
#include "core_connection_manager.h"
#include "base_log.h"
#include "base_log_thread.h"

using namespace BASE_NAMEPSACE_DECL;

MESSAGEMAP_DECL_BEGIN(core_packet)
MESSAGEMAP_REGISTER(10, new CCorePacket())
MESSAGEMAP_DECL_END()

#define TEST_MSG 10
#define TEST_MSG2 20
#define TEST_MSG3 30
#define TEST_MSG4 40
#define TEST_MSG5 50
#define TEST_SERVER_TYPE 244

void test_message_map()
{
	CREATE_MESSAGE_MAP_DECL();
	//装载消息体映射
	LOAD_MESSAGEMAP_DECL(core_packet);

	CCorePacket* packet = (CCorePacket *)MESSAGE_MAP_DECL()->gain_message(10);
	if(packet != NULL)
	{
		packet->server_type_ = TEST_SERVER_TYPE;
		cout << packet->server_type_ << endl;
	}

	DESTROY_MESSAGE_MAP_DECL();
}

class CValuePacket : public CBasePacket
{
public:
	CValuePacket(){};
	virtual ~CValuePacket(){};

	virtual void release_self()
	{
		delete this;
	}

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << value_;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> value_;
	};

public:
	uint32_t value_;
};

//消息体映射关系
MESSAGEMAP_DECL_BEGIN(ValuePacket)
MESSAGEMAP_REGISTER(TEST_MSG, new CValuePacket())
MESSAGEMAP_REGISTER(TEST_MSG2, new CValuePacket())
MESSAGEMAP_REGISTER(TEST_MSG3, new CValuePacket())
MESSAGEMAP_REGISTER(TEST_MSG4, new CValuePacket())
MESSAGEMAP_REGISTER(TEST_MSG5, new CValuePacket())
MESSAGEMAP_DECL_END()

//消息映射触发
class CTestMessageProcessor : public ICmdTarget
{
public:
	CTestMessageProcessor(){};
	~CTestMessageProcessor(){};

	void process_1(CBasePacket* packet)
	{
		CValuePacket* p = (CValuePacket *)packet;
		cout << "process_1, value = " << p->value_ << endl;
	};
	int32_t process_2(CBasePacket* packet)
	{
		CValuePacket* p = (CValuePacket *)packet;
		cout << "process_2, value = " << p->value_ << endl;

		return 0;
	};
	int32_t process_3(CBasePacket* packet, uint32_t sid)
	{
		CValuePacket* p = (CValuePacket *)packet;
		cout << "process_3, value = " << p->value_ << ", sid = " << sid << endl;
		
		return 0;
	};
	int32_t process_4(CBasePacket* packet, uint32_t sid, CConnection* connection)
	{
		CValuePacket* p = (CValuePacket *)packet;
		cout << "process_4, value = " << p->value_ << ", sid = " << sid << endl;

		return 0;
	};

	int32_t process_5(CBasePacket* packet, uint32_t sid, const Inet_Addr& remote_addr)
	{
		CValuePacket* p = (CValuePacket *)packet;
		cout << "process_5, value =" << (uint32_t)p->value_ << endl;

		return 0;
	}

	int32_t process_connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
	{
		cout << "process_connect_event " << endl;
		return 0;
	}

	int32_t process_disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection)
	{
		cout << "process_disconnect_event " << endl;
		return 0;
	}

	DECLARE_MSGCALL_MAP()
};

//消息映射安装
BEGIN_MSGCALL_MAP(CTestMessageProcessor)
	ON_MSG_VOID(TEST_MSG, &CTestMessageProcessor::process_1)
	ON_MSG_INT(TEST_MSG2, &CTestMessageProcessor::process_2)
	ON_MSG_ID_INT(TEST_MSG3, &CTestMessageProcessor::process_3)
	ON_MSG_CON_INT(TEST_MSG4, &CTestMessageProcessor::process_4)
	ON_MSG_ADDR_INT(TEST_MSG5, &CTestMessageProcessor::process_5)
	ON_MSG_CON_INT(TCP_CONNECT_EVENT, &CTestMessageProcessor::process_connect_event)
	ON_MSG_CON_INT(TCP_CLOSE_EVENT, &CTestMessageProcessor::process_disconnect_event)
END_MSGCALL_MAP()

void test_message_call()
{
	CValuePacket packet;
	packet.value_ = 100;

	CTestMessageProcessor test_pr;
	test_pr.on_event(TEST_MSG, 0, &packet, NULL);
	packet.value_ = TEST_SERVER_TYPE + 1;
	test_pr.on_event(TEST_MSG2, 0, &packet, NULL);
	packet.value_ = TEST_SERVER_TYPE + 2;
	test_pr.on_event(TEST_MSG3, 0, &packet, NULL);
	packet.value_ = TEST_SERVER_TYPE + 3;
	test_pr.on_event(TEST_MSG4, 0, &packet, NULL);
}


void test_message_call2()
{
	CTestMessageProcessor processor;
	INIT_MSG_PROCESSOR1(&processor);
	LOAD_MESSAGEMAP_DECL(ValuePacket);

	CCorePacket packet;
	CValuePacket test_pack;

	test_pack.value_ = 100;

	BinStream data_strm;
	data_strm << test_pack;

	packet.server_type_ = TEST_SERVER_TYPE;
	packet.msg_id_ = TEST_MSG;
	packet.server_id_ = 10;
	MSG_PROCESSOR()->on_message(packet, data_strm, NULL);

	data_strm.rewind(false);
	packet.server_type_ = TEST_SERVER_TYPE + 1;
	packet.msg_id_ = TEST_MSG2;
	packet.server_id_ = 22;
	MSG_PROCESSOR()->on_message(packet, data_strm, NULL);

	data_strm.rewind(false);
	packet.server_type_ = TEST_SERVER_TYPE + 2;
	packet.msg_id_ = TEST_MSG3;
	packet.server_id_ = 23;
	MSG_PROCESSOR()->on_message(packet, data_strm, NULL);

	data_strm.rewind(false);
	packet.server_type_ = TEST_SERVER_TYPE + 3;
	packet.msg_id_ = TEST_MSG4;
	packet.server_id_ = 24;
	MSG_PROCESSOR()->on_message(packet, data_strm, NULL);
}

void test_core_udp()
{
	//建立日志
	LOG_CREATE();
	LOG_THREAD_CREATE();
	LOG_THREAD_INSTANCE()->start();

	//建立一个REACTOR_THERAD
	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(20000);
	CREATE_REACTOR_THREAD();

	//建立消息映射关系
	CREATE_MSG_PROCESSOR();

	CTestMessageProcessor processor;
	INIT_MSG_PROCESSOR1(&processor);

	//建立消息体管理关系
	LOAD_MESSAGEMAP_DECL(ValuePacket);

	REACTOR_THREAD()->start();

	//创建一个UDP对象
	CoreUDPHandler udp_handler;
	Inet_Addr local_addr("127.0.0.1", 4563);
	udp_handler.open(local_addr);

	//关键消息接收
	udp_handler.attach(MSG_PROCESSOR());

	CCorePacket packet;
	CValuePacket test_pack;

	test_pack.value_ = 100;

	BinStream data_strm, bin_strm;
	data_strm << test_pack;

	packet.set_body(test_pack);

	packet.server_type_ = TEST_SERVER_TYPE;
	packet.msg_id_ = TEST_MSG5;
	packet.server_id_ = 10;

	while(true)
	{
		usleep(2000000);
		cout << "send udp ok" << endl;
		bin_strm.rewind(true);
		packet.server_id_ ++;
		bin_strm << packet;
		udp_handler.send(bin_strm, local_addr);
		
	}

	REACTOR_INSTANCE()->close_reactor();
}

void test_core_tcp_server()
{
	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(20000);
	
	CREATE_REACTOR_THREAD();

	//建立消息映射关系
	CREATE_MSG_PROCESSOR();

	CTestMessageProcessor processor;
	INIT_MSG_PROCESSOR1(&processor);

	//建立消息体管理关系
	LOAD_MESSAGEMAP_DECL(ValuePacket);
	LOAD_MESSAGEMAP_DECL(TCP_EVENT);

	REACTOR_THREAD()->start();

	//创建一个UDP对象
	CCoreTCPListener tcp_listener;
	Inet_Addr local_addr(INADDR_ANY, 4563);
	tcp_listener.open(local_addr);

	CConnection* conn = CONNECTION_POOL.pop_obj();
	Inet_Addr remote_addr("127.0.0.1", 4563);
	conn->connect(remote_addr);
	int i = 0;
	while(true)
	{
		usleep(661000);
		i ++;
		//if(i == 5)
		//	conn->close();
	}

	REACTOR_INSTANCE()->close_reactor();
}



#include "test_rudp.h"
#include "revolver/base_reactor_instance.h"
#include "revolver/base_singleton.h"

#include "rudp/rudp_connection.h"
#include "rudp/rudp_handler.h"
#include "rudp/rudp_listener.h"

#include "revolver/base_thread.h"
#include "core/core_cmd_target.h"
#include "core/core_msg_.h"
#include "core/core_message_processor.h"
#include "core/core_event_message.h"

#include "../defs/client_msg.h"
//#include "cli_gateway.h"

using namespace BASE;
using namespace BASE_NAMEPSACE_DECL;

class RudpEvThread : public CThread {
public:
    RudpEvThread() {}
    virtual ~RudpEvThread() {}

    void execute() {
        while (!get_terminated()) {
            REACTOR_INSTANCE()->event_loop();
        }
        REACTOR_INSTANCE()->stop_event_loop();
        clear_thread();
    }
};

#define RUDP_EV_THREAD  CSingleton<RudpEvThread>::instance

#define CREATE_RUDP_HANDLER CSingleton<RudpHandler>::instance
#define RUDP_HANDLER CSingleton<RudpHandler>::instance
#define DESTROY_RUDP_HANDLER CSingleton<RudpHandler>::destroy


#define CREATE_RUDP_LISTEN	CSingleton<RUDPListenerHandler>::instance
#define RUDP_LISTEN			CSingleton<RUDPListenerHandler>::instance
#define DESTROY_RUDP_LISTEN	CSingleton<RUDPListenerHandler>::destroy

class TestRudpAdapter : public IRUDPAdapter
{
public:
    TestRudpAdapter() {}
    virtual ~TestRudpAdapter() {}

    virtual void send(BinStream& strm, const Inet_Addr& remote_addr) {
        if (RUDP_HANDLER()->is_open())
        {
            RUDP_HANDLER()->send(strm, remote_addr);
        }
    }
};

#define RUDP_ADAPTER  CSingleton<TestRudpAdapter>::instance

class TestRudpProcessor : public ICmdTarget {
public:
    int32_t  on_connect_event(CBasePacket* packet, uint32_t sid, CConnection* connection) {
        std::cout << "rudp connected" << std::endl;
        return 0;
    }
    int32_t  on_disconnect_event(CBasePacket* packet, uint32_t sid, CConnection* connection) {
        std::cout << "rudp disconnected" << std::endl;
        return 0;
    }
protected:
    DECLARE_MSGCALL_MAP()
};

#define CREATE_CLIENT_PROCESSOR  CSingleton<TestRudpProcessor>::instance
#define CLIENT_PROCESSOR  CSingleton<TestRudpProcessor>::instance
#define DESTROY_CLIENT_PROCESSOR  CSingleton<TestRudpProcessor>::destroy

BEGIN_MSGCALL_MAP(TestRudpProcessor)
    ON_MSG_CON_INT(TCP_CONNECT_EVENT, &TestRudpProcessor::on_connect_event)
    ON_MSG_CON_INT(TCP_CLOSE_EVENT, &TestRudpProcessor::on_disconnect_event)
    //ON_MSG_CON_INT(START_LIVE_SHOW_REQ, &TestRudpProcessor::on_start_live)
END_MSGCALL_MAP()


#define CREATE_LISTENER CSingleton<RUDPListener>::instance
#define RUDP_LISTENER		CSingleton<RUDPListener>::instance
#define DESTROY_LISTENER	CSingleton<RUDPListener>::destroy

#define CREATE_RUDP_CONN CSingleton<RudpConnection>::instance
#define RUDP_CONN		CSingleton<RudpConnection>::instance
#define DESTROY_RUDP_CONN	CSingleton<RudpConnection>::destroy

void test_rudp_cli() {
    REACTOR_CREATE();
    REACTOR_INSTANCE()->open_reactor(256);

    init_rudp_socket();
    RUDP_EV_THREAD();

    RUDP_EV_THREAD()->start();

    CREATE_RUDP_CONN();

    create_rudp_client();

    Inet_Addr addr;
    addr.set_ip("127.0.0.1");
    addr.set_port(4040);
    RUDP_CONN()->connect(addr);

    usleep(2000000);

    CLIENT_GATEWAY::StartLiveShowRequest req;
    req.user_account_id = "111111";
    req.user_type = 0;
    req.room_id = 1000;

    RUDP_CONN()->send(req);

    char ch = getchar();

    RUDP_CONN()->close();
    usleep(2000000);

    destroy_rudp_client();

    RUDP_EV_THREAD()->terminate();

    destroy_rudp_socket();

    REACTOR_INSTANCE()->close_reactor();

    DESTROY_RUDP_HANDLER();
    REACTOR_DESTROY();
}

void test_rudp_srv() {
    REACTOR_CREATE();
    REACTOR_INSTANCE()->open_reactor(256);

    init_rudp_socket();
    RUDP_EV_THREAD();

    //CREATE_RUDP_HANDLER();
    create_rudp_listener();

    Inet_Addr local_addr(INADDR_ANY, 4040);
    RUDP_LISTENER()->open(local_addr);
    /*RUDP_HANDLER()->attach_adapter(RUDP_ADAPTER());
    RUDP_HANDLER()->open(local_addr);

    CREATE_RUDP_LISTEN();
    RUDP()->attach_listener(RUDP_LISTEN());*/

    INIT_MSG_PROCESSOR1(CLIENT_PROCESSOR());

    RUDP_EV_THREAD()->start();

    char ch = getchar();

    RUDP_EV_THREAD()->terminate();

    RUDP_LISTENER()->close();
    
    //RUDP_HANDLER()->close();
    destroy_rudp_listener();

    RUDP()->destroy();

    REACTOR_INSTANCE()->close_reactor();

    DESTROY_RUDP_HANDLER();
    REACTOR_DESTROY();
}

void test_rudp_init() {
    REACTOR_CREATE();
    REACTOR_INSTANCE()->open_reactor(256);

    init_rudp_socket();
    RUDP_EV_THREAD();

    CREATE_RUDP_HANDLER();

    Inet_Addr local_addr(INADDR_ANY, 0);
    RUDP_HANDLER()->attach_adapter(RUDP_ADAPTER());
    RUDP_HANDLER()->open(local_addr);

    RUDP()->attach_listener(0);  // client mode.


    RUDP_EV_THREAD()->start();

    char ch = getchar();

    RUDP_EV_THREAD()->terminate();

    RUDP()->attach_listener(0);
    RUDP()->destroy();

    RUDP_HANDLER()->close();

    REACTOR_INSTANCE()->close_reactor();

    DESTROY_RUDP_HANDLER();
    REACTOR_DESTROY();
}

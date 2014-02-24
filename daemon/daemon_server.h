#ifndef __DAEMON_SERVER_H
#define __DAEMON_SERVER_H

#include "core_msg_.h"
#include "core_cmd_target.h"
#include "daemon_element.h"
#include "base_event_handler.h"
#include "daemon_json.h"
#include "core_dc_client.h"
#include "daemon_physical.h"

#include <map>
#include <set>
#include <vector>

using namespace std;
using namespace BASE_NAMEPSACE_DECL;

class CDaemonServer : public ICmdTarget,
					  public CEventHandler
{
public:
	CDaemonServer();
	virtual ~CDaemonServer();

public:
	void			init(IPhysicalServer* db);
	void			destroy();

	int32_t			on_daemon_ping(CBasePacket* packet, uint32_t sid, const Inet_Addr& remote_addr);
	int32_t			on_register_req(CBasePacket* packet, uint32_t sid, CConnection* connection);
	int32_t			on_node_state(CBasePacket* packet, uint32_t sid, CConnection* connection);
	int32_t			on_disconnect(CBasePacket* packet, uint32_t sid, CConnection* connection);

public:
	//定时器函数
	virtual int32_t handle_timeout(const void *act, uint32_t timer_id);

	//返回物理机的信息
	void			on_physical_server_event(const ServerInfoJson& server_json, CConnection* connection);

protected:
	uint32_t		set_timer();
	void			cancel_timer();
	void			heartbeat();

	uint16_t		get_port(uint8_t server_type);

	void			process_server_info(const ServerInfoJson& json_obj, CConnection* connection);

protected:
	void			send_register_res(uint8_t type, uint32_t sid, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr, CConnection* connection);

	DECLARE_MSGCALL_MAP()

protected:
	uint32_t		time_id_;
	uint32_t		dc_id_;

	ServerElementMap element_map_;
	IPhysicalServer*  phy_db_;
};

#endif

#include "daemon_server.h"
#include "core_connection.h"
#include "daemon_simple_physical.h"


DaemonServerDB::DaemonServerDB(CDaemonServer* server) : server_(server)
{
	init();
}

DaemonServerDB::~DaemonServerDB()
{
	destroy();
}

void DaemonServerDB::init()
{
	//设置写模拟数据,这些信息可以通过数据库配置的

	//一个电信的物理机
	PhysicalServerInfo phy_tel_server;
	phy_tel_server.net_type = 0x0000;
	phy_tel_server.tel_addr = "10.1.6.36";
	phy_tel_server.cnc_addr = "0.0.0.0";
	physical_server_map_[phy_tel_server.tel_addr] = phy_tel_server;

	//一个网通的物理机
	PhysicalServerInfo phy_cnc_server;
	phy_cnc_server.net_type = 0x0001;
	phy_cnc_server.tel_addr = "0.0.0.0";
	phy_cnc_server.cnc_addr = "10.1.6.61";
	physical_server_map_[phy_cnc_server.cnc_addr] = phy_cnc_server;

	//添加一个双线机
	PhysicalServerInfo phy_wan_server;
	phy_wan_server.net_type = 0x00ff;
	phy_wan_server.tel_addr = "10.1.6.84";
	phy_wan_server.cnc_addr = "10.1.6.125";
	physical_server_map_[phy_wan_server.tel_addr] = phy_wan_server;
	physical_server_map_[phy_wan_server.cnc_addr] = phy_wan_server;
}

void DaemonServerDB::destroy()
{

}

void DaemonServerDB::get_physical_server(uint32_t sid, uint8_t stype, const string& server_ip, CConnection* connection)
{
	if(server_ != NULL && connection != NULL)
	{
		ServerInfoJson server_json;

		if(sid > 0) //已经存在的服务
		{
			DaemonServerInfoMap::iterator it = server_info_map_.find(sid);
			if(it != server_info_map_.end() && (it->second.cnc_addr == server_ip || it->second.tel_addr == server_ip)) //判断IP是否相等
			{
				server_json.result = 0;
				server_json.ip_ct = it->second.tel_addr;
				server_json.ip_un = it->second.cnc_addr;
				server_json.sid = sid;
				server_json.type = stype;
				server_json.net_type = it->second.net_type;

				server_->on_physical_server_event(server_json, connection);
			}
			else //重新构建一个新的单元
			{
				server_json = create_server(sid, stype, server_ip);
				server_->on_physical_server_event(server_json, connection);
			}
		}
		else //第一次启动的服务SID = 0
		{
			server_json = create_server(sid, stype, server_ip);
			server_->on_physical_server_event(server_json, connection);
		}
	}
}

ServerInfoJson DaemonServerDB::create_server(uint32_t sid, uint8_t stype, const string& server_ip)
{
	ServerInfoJson ret;
	ret.result = 0x03;

	PhysicalServerInfoMap::iterator it = physical_server_map_.find(server_ip);
	if(it != physical_server_map_.end())
	{
		ret.result = 0;
		ret.ip_ct = it->second.tel_addr;
		ret.ip_un = it->second.cnc_addr;
		ret.net_type = it->second.net_type;
		ret.type = stype;
		ret.sid = max_sid();

		DaemonServerInfo info;
		info.sid = ret.sid;
		info.net_type = ret.net_type;
		info.tel_addr = ret.ip_ct;
		info.cnc_addr = ret.ip_un;
		info.stype = stype;

		server_info_map_[info.sid] = info;
	}

	return ret;
}

uint32_t DaemonServerDB::max_sid()
{
	if(physical_server_map_.size() <= 0)
		return 2; //1被daemond占用了

	uint32_t sid = 1;
	for(DaemonServerInfoMap::iterator it = server_info_map_.begin(); it != server_info_map_.end(); ++ it)
	{
		if(it->first > sid)
			sid = it->first;
	}

	return sid + 1;
}

void DaemonServerDB::on_connection_disconnected(CConnection* connection)
{
	//todo:如果是数据库异步读取的话，就必须要把connection获取信息的步骤停止
}





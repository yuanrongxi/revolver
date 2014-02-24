/*************************************************************************************
*filename:	daemon_simple_physical.h
*
*to do:		定义一个简单的物理服务器管理的类，如果是正式的系统，可以继承IPhysicalServer
			来实现一个和后台数据库访问和存储，这样可以很方便管理服务单元和物理机
*Create on: 2013-08
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __DAEMON_SIMPLE_PHYSICAL_H
#define __DAEMON_SIMPLE_PHYSICAL_H

#include "daemon_physical.h"
#include "daemon_json.h"

#include <map>
using namespace std;
using namespace BASE;

//物理机器信息
typedef struct tagPhysicalServerInfo
{
	//电信地址,如果是网通，地址为0.0.0.0
	string		tel_addr;
	//网通地址,如果是电信，地址为0.0.0.0
	string		cnc_addr;
	//网络类型，高位1字节表示地区，例如0x00为北京，0x01为上海，低位一字节表示运营商，例如：0x00表示电信，0x01表示网通，.......0xff表示双线
	uint16_t	net_type;
}PhysicalServerInfo;

//服务单元信息
typedef struct tagDaemonServerInfo
{
	//服务ID，不能重复
	uint32_t	sid;
	//服务类型，参见core_server_type.h
	uint8_t		stype;
	//网络类型
	uint16_t	net_type;

	//电信地址,如果是网通，地址为0.0.0.0
	string		tel_addr;
	//网通地址,如果是电信，地址为0.0.0.0
	string		cnc_addr;
}DaemonServerInfo;

typedef map<string, PhysicalServerInfo>		PhysicalServerInfoMap;
typedef map<uint32_t, DaemonServerInfo>		DaemonServerInfoMap;

class CDaemonServer;
//模拟一个简单的DB管理服务单元信息的用例
class DaemonServerDB : public IPhysicalServer
{
public:
	DaemonServerDB(CDaemonServer* server);
	~DaemonServerDB();

	void					get_physical_server(uint32_t sid, uint8_t stype, const string& server_ip, CConnection* connection);
	void					on_connection_disconnected(CConnection* connection);

protected:
	void					init();
	void					destroy();

	//如果是数据库存储的话，可以用数据库脚本实现以下2个函数
	ServerInfoJson			create_server(uint32_t sid, uint8_t stype, const string& server_ip);
	uint32_t				max_sid();
protected:
	CDaemonServer*			server_;
	//TODO:如果是数据库存储，请将这些physical_server_map_的信息配置在数据库中
	PhysicalServerInfoMap	physical_server_map_;
	//TODO:如果是数据库存储，请将server_info_map_产生的信息保存到数据库
	DaemonServerInfoMap		server_info_map_;
};
#endif
/************************************************************************************/


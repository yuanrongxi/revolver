#include "core/core_server_type.h"

#define MAX_SERVER_SIZE	eMax_Server_Type

const char*	 ServerName[] = {
	"eClient",
	"eDaemon_Server",
	"eSample_Server",
	//TODO:填写对应的SERVER类型名称
	"errorServerName"
};

const char* GetServerName(uint8_t stype)
{
	return ServerName[stype > MAX_SERVER_SIZE ? MAX_SERVER_SIZE : stype];
}



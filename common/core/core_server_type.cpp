#include "core/core_server_type.h"

#define MAX_SERVER_SIZE	eMax_Server_Type
static uint8_t _max_server_size = 0;
const char*	 _ServerName[256] = { 0 };


//const char*	 ServerName[256] = {
//    "eClient",
//    "eDaemon_Server",
//    "eSample_Server",
//    //TODO:填写对应的SERVER类型名称
//    "Gateway_Server",
//    "errorServerName"
//};
//
const char* GetServerName(uint8_t stype)
{
    return server_name_list[stype > _max_server_size ? _max_server_size : stype];
}

void SetMaxServerCount(uint8_t size) {
    _max_server_size = size;
}

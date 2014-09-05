/**********************************************************************************/
#include "daemon_config.h"
#include "json/json_stream.h"
#include "core/core_local_info.h"
#include "core/core_log_macro.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define FILE_NAME	"server.json"
#define MAX_PATH	1024

using namespace std;
using namespace JSON_NS;

BASE_NAMESPACE_BEGIN_DECL

//JSON¸ñÊ½
class ServerJson : public JsonInterface
{
public:
	ServerJson() {};

protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("sid", jobj, sid);
		strm.encode("stype", jobj, stype);

		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("sid", jobj, sid);
		strm.decode("stype", jobj, stype);

		return strm;
	}

public:
	uint32_t	sid;
	uint32_t	stype;
};

////////////////////////////////////////////////////////////////////////////////////
CDaemonConfig::CDaemonConfig()
{

}

CDaemonConfig::~CDaemonConfig()
{

}

string CDaemonConfig::get_path()
{
	string str_exepath;
	string file_name(FILE_NAME);

	char buffer[MAX_PATH]= {0};

#ifdef WIN32
	char *path = _getcwd(buffer, MAX_PATH);
	str_exepath = buffer;
	str_exepath += "\\";
#else
	char* path = getcwd(buffer, MAX_PATH);
	str_exepath = buffer;
	str_exepath += "/";
#endif

	str_exepath += file_name;

	return str_exepath;
}

void CDaemonConfig::read()
{
	ifstream json_file;

	string str_exepath = get_path();

	json_file.open(str_exepath.c_str(), ios_base::in);

	if(!json_file.is_open())
	{
		SERVER_ID = 0;
		return ;
	}

	string json_str = "";
	string line_str;

	while(!json_file.eof())
	{
		line_str.clear();

		json_file >> line_str;
		json_str += line_str;
	}

	json_file.close();

	ServerJson json_obj;
	string err;

	if (json_obj.UnSerialize(json_str, err) != JSON_SUCCESS)
	{
		CORE_ERROR("json parse failed! json str = " << json_str);
		SERVER_ID = 0;
		return ;
	}

	SERVER_ID = json_obj.sid;
}

void CDaemonConfig::write()
{
	ServerJson json_obj;
	json_obj.sid = SERVER_ID;
	json_obj.stype = SERVER_TYPE;

	string json_str;
	json_obj.Serialize(json_str);

	string str_exepath = get_path();

	ofstream json_file;
	json_file.open(str_exepath.c_str(), std::ios_base::trunc|std::ios_base::out);

	if(!json_file.is_open())
	{
		return ;
	}

	json_file << json_str;

	json_file.flush();
	json_file.close();
}


BASE_NAMESPACE_END_DECL

/**********************************************************************************/
#ifndef __DAEMON_JSON_H_
#define __DAEMON_JSON_H_

#include "json_stream.h"

#include <vector>

using namespace std;
using namespace JSON_NS;

class QueryServerJson : public JsonInterface
{
public:
	QueryServerJson() {};

protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("sid", jobj, sid);
		strm.encode("type", jobj, stype);
		strm.encode("ip", jobj, ip);

		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("sid", jobj, sid);
		strm.decode("type", jobj, stype);
		strm.decode("ip", jobj, ip);

		return strm;
	}

public:
	uint32_t		sid;
	uint32_t		stype;
	string			ip;
};

class ServerInfoJson : public JsonInterface
{
public:
	ServerInfoJson(){}

protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("result", jobj, result);
		strm.encode("sid", jobj, sid);
		strm.encode("type", jobj, type);
		strm.encode("nettype", jobj, net_type);
		strm.encode("ip_ct", jobj, ip_ct);
		strm.encode("ip_un", jobj, ip_un);

		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("result", jobj, result);
		strm.decode("sid", jobj, sid);
		strm.decode("type", jobj, type);
		strm.decode("nettype", jobj, net_type);
		strm.decode("ip_ct", jobj, ip_ct);
		strm.decode("ip_un", jobj, ip_un);

		return strm;
	}

public:
	uint32_t	result;		//=0表示成功，其他表示失败
	uint32_t	sid;
	uint32_t	type;
	uint32_t	net_type;
	string		ip_ct;
	string		ip_un;
};

class NodeStateJson : public JsonInterface
{
public:
	NodeStateJson(){}

protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("sid", jobj, sid);
		strm.encode("info", jobj, info);

		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("sid", jobj, sid);
		strm.decode("type", jobj, info);

		return strm;
	}

public:
	uint32_t	sid;
	string		info;
};

#endif


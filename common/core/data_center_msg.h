#ifndef DATA_CENTER_MSG_H__
#define DATA_CENTER_MSG_H__

#include "revolver/base_packet.h"
#include "core/core_server_type.h"
#include "core/core_message_map_decl.h"
#include "revolver/base_inet_addr.h"
#include <string>
using namespace std;
using namespace BASEOBJECT;

namespace SERVERMESSAGE {

#define PHP_EXC_REQUEST	0xff040001
#define PHP_EXC_RESPONSE	0x04ff0001

typedef string		BIN_DATA;



class PHPExcRequst : public CBasePacket
{
public:
	PHPExcRequst()
	{
	}
	~PHPExcRequst()
	{
	}

public:
	uint32_t		exc_id;
	uint8_t		exc_type;
	uint8_t		exc_ret;
	uint16_t		time_delay;
	string		php_file;
	string		php_param;

public:
	void	Pack(BinStream& strm) const
	{
		strm << exc_id;
		strm << exc_type;
		strm << exc_ret;
		strm << time_delay;
		strm << php_file;
		strm << php_param;
	}
	void	UnPack(BinStream& strm)
	{
		strm >> exc_id;
		strm >> exc_type;
		strm >> exc_ret;
		strm >> time_delay;
		strm >> php_file;
		strm >> php_param;
	}

	void  Print(std::ostream& os) const
	{
		os << "PHPExcRequst, {";
		os << "exc_id = " << exc_id << ",";
		os << "exc_type = " << (uint16_t)exc_type << ",";
		os << "exc_ret = " << (uint16_t)exc_ret << ",";
		os << "time_delay = " << time_delay << ",";
		os << "php_file = " << php_file << ",";
		os << "php_param = " << php_param << ",";
		os << "}";
	}
};
class PHPExcResponse : public CBasePacket
{
public:
	PHPExcResponse()
	{
	}
	~PHPExcResponse()
	{
	}

public:
	uint32_t		exc_id;
	uint32_t		src_size;
	uint8_t			zip;
	string		result;

public:
	void	Pack(BinStream& strm) const
	{
		strm << exc_id;
		strm << src_size;
		strm << zip;
		strm << result;
	}
	void	UnPack(BinStream& strm)
	{
		strm >> exc_id;
		strm >> src_size;
		strm >> zip;
		strm >> result;

	}

	void  Print(std::ostream& os) const
	{
		os << "PHPExcResponse, {";
		os << "exc_id = " << exc_id << ",";
		os << "src_size = " << src_size << ",";
		os << "zip = " << zip << ",";
		os << "result = " << result << ",";
		os << "}";
	}
};


MESSAGEMAP_DECL_BEGIN(DATA_CENTER_MSG)
MESSAGEMAP_REGISTER(PHP_EXC_REQUEST, new PHPExcRequst())
MESSAGEMAP_REGISTER(PHP_EXC_RESPONSE, new PHPExcResponse())
MESSAGEMAP_DECL_END()

}
#endif

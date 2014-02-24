#ifndef SAMPLE_MSG_H__
#define SAMPLE_MSG_H__

#include "base_packet.h"
#include "core_server_type.h"
#include "core_message_map_decl.h"
#include "base_inet_addr.h"
#include <string>
using namespace std;
using namespace BASEOBJECT;

namespace SAMPLE_SERVER {

#define SAMPLE_MSGID	0xff000001

typedef vector<int32_t>		INT32_ARRAY;
typedef vector<int8_t>		INT8_ARRAY;

class CDesc
{
public:
	CDesc()
	{
		n_type = 0;
		str_desc = "";
	}
	~CDesc()
	{
	}

public:
	int8_t		n_type;
	string		str_desc;
	INT32_ARRAY		arr_fd;
	INT8_ARRAY		arr_status;


	friend BinStream& operator<<(BinStream& strm, const CDesc& packet)
	{
		BinStream bstrm;
		string data_str;
		bstrm << packet.n_type;
		bstrm << packet.str_desc;

		int32_t count_0 = packet.arr_fd.size();
		bstrm<< count_0;
		for (int32_t i = 0; i < count_0; i++)
		{
			bstrm << packet.arr_fd[i];
		}

		int32_t count_1 = packet.arr_status.size();
		bstrm<< count_1;
		for (int32_t i = 0; i < count_1; i++)
		{
			bstrm << packet.arr_status[i];
		}
		bstrm.bin_to_string(data_str);
		strm << data_str;
		return strm;
	}

	friend BinStream& operator>>(BinStream& strm, CDesc& packet)
	{
		BinStream bstrm;
		string data_str;
		strm >> data_str;
		bstrm = data_str;
		bstrm >> packet.n_type;
		bstrm >> packet.str_desc;

		int32_t count_2 = 0;
		bstrm>> count_2;
		packet.arr_fd.resize(count_2);
		for (int32_t i = 0; i < count_2; i++)
		{
			bstrm >> packet.arr_fd[i];
		}

		int32_t count_3 = 0;
		bstrm>> count_3;
		packet.arr_status.resize(count_3);
		for (int32_t i = 0; i < count_3; i++)
		{
			bstrm >> packet.arr_status[i];
		}
		return strm;
	}

	void  Print(std::ostream& os) const
	{
		os << "CDesc, {";
		os << "n_type = " << (int16_t)n_type << ",";
		os << "str_desc = " << str_desc << ",";
		os << "arr_fd = ";
		for (uint32_t i = 0; i < arr_fd.size(); i++)
		{
			os << arr_fd[i] << ",";
		}
		os << "arr_status = ";
		for (uint32_t i = 0; i < arr_status.size(); i++)
		{
			os << (int16_t)arr_status[i] << ",";
		}
		os << "}";
	}

	friend ostream& operator<<(ostream& os, const CDesc& packet)
	{
		packet.Print(os);
		return os;
	}
};


class CSamplePacket : public CBasePacket
{
public:
	CSamplePacket()
	{
		msg_type = 0;
	}
	~CSamplePacket()
	{
	}

public:
	uint8_t		msg_type;
	CDesc		msg_desc;

public:
	void	Pack(BinStream& strm) const
	{
		strm << msg_type;
		strm << msg_desc;
	}
	void	UnPack(BinStream& strm)
	{
		strm >> msg_type;
		strm >> msg_desc;
	}

	void  Print(std::ostream& os) const
	{
		os << "CSamplePacket, {";
		os << "msg_type = " << (uint16_t)msg_type << ",";
		os << "msg_desc = " << msg_desc << ",";
		os << "}";
	}
};


MESSAGEMAP_DECL_BEGIN(SAMPLE_MSG)
MESSAGEMAP_REGISTER(SAMPLE_MSGID, new CSamplePacket())
MESSAGEMAP_DECL_END()

}
#endif

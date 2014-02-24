#ifndef SAMPLE_MSG_H__
#define SAMPLE_MSG_H__

#include "base_packet.h"
#include "core_server_type.h"
#include "core_message_map_decl.h"
#include "base_inet_addr.h"

namespace BASE_NAMEPSACE_DECL {

#define SAMPLE_MSGID	0xff000001

typedef vector<int32_t>		INT32_ARRAY;
typedef vector<int8_t>		array<int8>;


class CDesc
{
public:
	CDesc()
	{
		int8_t	n_type = 0;
		string	str_desc = "";
	}
	~CDesc()
	{
	}
public:
	INT32_ARRAY	arr_fd;
	INT8_ARRAY	arr_status;
	int8_t	n_type;
	string	str_desc;
};

class CSamplePacket : public CBasePacket
{
public:
	CSamplePacket()
	{
		uint8_t	msg_type = 0;
	}
	~CSamplePacket()
	{
	}
public:
	CDesc	msg_desc;
	uint8_t	msg_type;
	void	Pack(BinStream& strm) const
	{
		strm << msg_desc<< msg_type;
	}
	void	UnPack(BinStream& strm) const
	{
		strm >> msg_desc>> msg_type;
	}
};
  }
#endif

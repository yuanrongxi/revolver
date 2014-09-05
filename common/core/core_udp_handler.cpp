#include "core/core_packet.h"
#include "core/core_udp_handler.h"
#include "core/core_log_macro.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_bandwidth_throttler.h"

#define MAX_UDP_PACKET 1492

#define UDP_CORE_PACKET	0
#define UDP_CORE_MEDIA	1

BASE_NAMESPACE_BEGIN_DECL

CoreUDPHandler::CoreUDPHandler()
{
	bin_strm_.resize(MAX_UDP_PACKET);

	CORE_DEBUG("Create core UDP");
}

CoreUDPHandler::~CoreUDPHandler()
{
	recivers_.clear();
	if(is_open())
	{
		this->close();
	}

	CORE_DEBUG("Destory UDP");
}

bool CoreUDPHandler::is_open() const
{
	return (get_handle() != INVALID_HANDLER);
}

int32_t CoreUDPHandler::open(const Inet_Addr& local_addr)
{
	CORE_INFO("open core udp socket, bind local addr = " << local_addr);

	int32_t ret = sock_dgram_.open(local_addr, true);
	if(ret == 0)
	{
		//设置缓冲区大小
		int32_t buf_size = 1024 * 1024; //1M
		sock_dgram_.set_option(SOL_SOCKET, SO_RCVBUF, (void *)&buf_size, sizeof(int32_t));
		sock_dgram_.set_option(SOL_SOCKET, SO_SNDBUF, (void *)&buf_size, sizeof(int32_t));

		CORE_DEBUG("CoreUDPHandler, register MASK_READ");
		return REACTOR_INSTANCE()->register_handler(this, MASK_READ);
	}
	else
	{
		CORE_FATAL("open UDP failed");
		return -1;
	}
}

int32_t CoreUDPHandler::close()
{
	CORE_INFO("close core udp");

	REACTOR_INSTANCE()->delete_handler(this);
	sock_dgram_.close();

	return 0;
}

int32_t CoreUDPHandler::send(BinStream& bin_strm, const Inet_Addr& remote_addr)
{
	int32_t rc = sock_dgram_.send(bin_strm.get_rptr(), bin_strm.data_size(), remote_addr);
	if(rc < 0)
	{
		if(XEAGAIN == error_no() || XEINPROGRESS == error_no()) //插入一个写事件，防止SOCKET异常
		{
			CORE_WARNING("core udp send EAGAIN");
			REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);

			bin_strm.rewind(true);
			return 0;
		}
		else
		{
			bin_strm.rewind(true);
			CORE_FATAL("core udp send failed, remote addr = " << remote_addr);
			return -1;
		}

		bin_strm.rewind(true);
	}

	THROTTLER()->add_udp_packet(bin_strm.data_size(), true);

	bin_strm.rewind(true);
	return rc;
}

int32_t CoreUDPHandler::send(const CCorePacket& packet, const Inet_Addr& remote_addr)
{
	GAIN_BINSTREAM(bin_strm);

	uint8_t packet_type = UDP_CORE_PACKET;
	*bin_strm << packet_type << packet;

	int32_t ret = send(*bin_strm, remote_addr);
	RETURN_BINSTREAM(bin_strm);

	return ret;
}

int32_t CoreUDPHandler::send(const string& data, const Inet_Addr& remote_addr)
{
	THROTTLER()->add_udp_packet(data.size(), true);

	GAIN_BINSTREAM(bin_strm);
	*bin_strm = data;

	int rc = send(*bin_strm, remote_addr);
	RETURN_BINSTREAM(bin_strm);

	return rc;
}

BASE_HANDLER CoreUDPHandler::get_handle() const
{
	return sock_dgram_.get_handler();
}

void CoreUDPHandler::attach(IReciver* reciver)
{
	if(reciver != NULL)
	{
		for(size_t i = 0; i < recivers_.size(); ++i)
		{
			if(recivers_[i] == reciver)
			{
				return ;
			}
		}

		CORE_INFO("core udp handler, attach reciver = 0x" << bin2asc((uint8_t *)reciver, sizeof(reciver)));
		recivers_.push_back(reciver);
	}
}

int32_t CoreUDPHandler::handle_input(BASE_HANDLER handle)
{
	Inet_Addr remote_addr;
	while(true)
	{
		bin_strm_.rewind(true);
		int32_t rc = sock_dgram_.recv(bin_strm_.get_wptr(), MAX_UDP_PACKET, remote_addr);
		if(rc > 0)
		{
			bin_strm_.set_used_size(rc);
			//轮询消息处理
			THROTTLER()->add_udp_packet(rc, false);
			uint8_t packet_type = 0;
			bin_strm_ >> packet_type;

			for(size_t i = 0; i < recivers_.size(); ++i)
			{
				if(process(recivers_[i], packet_type, bin_strm_, remote_addr) == 0)
				{
					break;
				}
			}
		}
		else
		{
			if(XEAGAIN == error_no())
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}
	return 0;
}

int32_t CoreUDPHandler::process(IReciver* reciver, uint8_t packet_type, BinStream& bin_strm, const Inet_Addr& remote_addr)
{
	int32_t ret = -1;
	switch(packet_type)
	{
	case UDP_CORE_PACKET:
		ret = reciver->reciver(bin_strm, remote_addr);
		break;

	case UDP_CORE_MEDIA:
		ret = reciver->reciver_media(bin_strm, remote_addr);
		break;

	default:
		;
	}

	return ret;
}

int32_t CoreUDPHandler::handle_output(BASE_HANDLER handle)
{
	REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);
	return 0;
}

int32_t CoreUDPHandler::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	return 0;
}

int32_t CoreUDPHandler::handle_exception(BASE_HANDLER handle)
{
	return 0;
}

BASE_NAMESPACE_END_DECL


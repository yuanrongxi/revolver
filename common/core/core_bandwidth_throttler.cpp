#include "core_bandwidth_throttler.h"

BASE_NAMESPACE_BEGIN_DECL

CoreThrottler::CoreThrottler()
{
	total_up_count_ = 0;
	total_down_count_ = 0;

	reset();
}

CoreThrottler::~CoreThrottler()
{

}

void CoreThrottler::reset()
{
	up_bandwidth_ = 0;
	down_bandwidth_ = 0;

	up_packet_count_ = 0;
	down_packet_count_ = 0;

	up_udp_count_ = 0;
	down_udp_count_ = 0;

	up_tcp_count_ = 0;
	down_tcp_count_ = 0;
}

void CoreThrottler::add_udp_packet(uint32_t packet_size, bool up /* = false */)
{
	if(up)
	{
		up_bandwidth_ += packet_size;
		up_packet_count_ ++;
		up_udp_count_ ++;

		total_up_count_ ++;
	}
	else
	{
		down_bandwidth_ += packet_size;
		down_packet_count_ ++;
		down_udp_count_ ++;

		total_down_count_ ++;
	}
}

void CoreThrottler::add_tcp_packet(uint32_t packet_size, bool up /* = false */)
{
	if(up)
	{
		up_bandwidth_ += packet_size;
		up_packet_count_ ++;
		up_tcp_count_ ++;

		total_up_count_ ++;
	}
	else
	{
		down_bandwidth_ += packet_size;
		down_packet_count_ ++;
		down_tcp_count_ ++;

		total_down_count_ ++;
	}
}

BASE_NAMESPACE_END_DECL


#ifndef __TEST_TCP_DELAY_H_
#define __TEST_TCP_DELAY_H_

#include "base_reactor_instance.h"
#include "base_event_handler.h"
#include "base_sock_connector.h"
#include "stat_packet.h"
#include "base_packet.h"
#include "base_block_buffer.h"
#include <iostream>
#include <string>

using namespace std;
using namespace BASE_NAMEPSACE_DECL;
class CDelayPacket : public CBasePacket
{
public:
	void Pack(BinStream& strm) const
	{
		strm << ts_ << data_;
	};

	void UnPack(BinStream& strm)
	{
		strm >> ts_ >> data_;
	};

	virtual void release_self()
	{
		
	};

	void Print(std::ostream& os) const
	{
	};

public:
	uint64_t	ts_;
	string		data_;
};


enum TestState
{
	eTestIdle,
	eTestConnecting,
	eTestConnected,
};

class CTestConnection: public CEventHandler
{
public:
	CTestConnection();
	virtual ~CTestConnection();

	void		set_stat_packet(StatPacketObj* stat) {stat_ = stat;}
	void		 close();

	CSockStream& get_sock_stream() {return sock_stream_;};
	BASE_HANDLER get_handle() const {return sock_stream_.get_handler();};
	void		 set_handle(BASE_HANDLER handle) {sock_stream_.set_handler(handle);};

	int32_t		connect(const Inet_Addr& remote_addr);

public:
	int32_t		 handle_input(BASE_HANDLER handle);
	int32_t		handle_output(BASE_HANDLER handle);
	int32_t		handle_timeout(const void *act, uint32_t timer_id);

	int32_t		handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	int32_t		handle_exception(BASE_HANDLER handle);

	//每次发送一个报文
	void		send();

protected:
	uint32_t	set_timer(uint32_t delay);
	void		cancel_timer();

	CReciverBuffer_T<BinStream, CSockStream, 1024 * 256> recv_buffer_;
	CSenderBuffer_T<BinStream, CSockStream, 1024 * 256> send_buffer_;

	uint32_t	timer_id_;

	StatPacketObj* stat_;

	CSockStream sock_stream_;

	uint8_t		state_;
	BinStream		istrm_;

	int32_t		timer_count_;
};
#endif


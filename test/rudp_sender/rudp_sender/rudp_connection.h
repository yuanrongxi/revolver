#ifndef __RUDP_CONNECTION_H_
#define __RUDP_CONNECTION_H_

#include "revolver/base_event_handler.h"
#include "rudp/rudp_stream.h"
#include "rudp/rudp_interface.h"
#include "revolver/base_block_buffer.h"
#include "revolver/base_bin_stream.h"
#include "revolver/base_packet.h"
#include "stat_packet.h"

using namespace BASE;

typedef CReciverBuffer_T<BinStream, RUDPStream, 1024 * 16>		RUDPRBuffer; 
typedef CSenderBuffer_T<BinStream, RUDPStream, 1024 * 64>		RUDPSBuffer;

#ifdef __16K_BUFFER
typedef RBuffer_16K		RUDPRBuffer;
typedef SBuffer_16K		RUDPSBuffer;
#endif


enum RUDPConnState
{
	RUDP_CONN_IDLE,			//平静状态,有可能是未连接
	RUDP_CONN_CONNECTING,	//连接状态
	RUDP_CONN_CONNECTED,	//连接状态
};

class RUDPTestPacket : public CBasePacket
{
public:
	RUDPTestPacket()
	{
		user_id		= 1000;
		ts			= 0;
		nick		= "zerok775";
		ctx			= "1234567890987654321";
	};

	~RUDPTestPacket()
	{

	}

	virtual void release_self()
	{
		delete this;
	}

protected:
	//编码解码函数
	virtual void	Pack(BinStream& strm) const
	{
		strm << user_id << ts << nick << ctx;
	};

	//解码函数
	virtual void	UnPack(BinStream& strm)
	{
		strm >> user_id >> ts >> nick >> ctx;
	};

	virtual void	Print(std::ostream& os) const
	{
	};

public:
	uint32_t	user_id;
	uint64_t	ts;
	string		nick;
	string		ctx;
};

class RUDPConnection : public CEventHandler,
					   public RUDPEventHandler
{
public:
	RUDPConnection();
	virtual ~RUDPConnection();

	void			set_stat(StatPacketObj* obj) {stat_ = obj;};
public:
	RUDPStream&		get_rudp_stream() {return rudp_sock_;};

	int32_t			handle_timeout(const void *act, uint32_t timer_id);

	int32_t			rudp_accept_event(int32_t rudp_id);
	int32_t			rudp_input_event(int32_t rudp_id);
	int32_t			rudp_output_event(int32_t rudp_id);
	int32_t			rudp_close_event(int32_t rudp_id);
	int32_t			rudp_exception_event(int32_t rudp_id);

public:
	int32_t			connect(const Inet_Addr& src_addr, const Inet_Addr& dst_addr);
	void			close();
	int32_t			send(RUDPTestPacket& packet);

	void			reset();
	void			set_timer(uint32_t delay);
	void			cancel_timer();
	void			heartbeat();

	void			set_state(uint8_t state) {state_ = state;};
	uint8_t			get_state() const {return state_;};

protected:
	void			process(RUDPTestPacket* packet);
	void			send_packet();

protected:
	RUDPStream		rudp_sock_;

	uint32_t		timer_id_;

	RUDPRBuffer		rbuffer_;			//接收BUFFER
	RUDPSBuffer		sbuffer_;			//发送BUFFER

	uint8_t			state_;

	BinStream		istrm_;

	uint32_t		packet_count_;
	uint64_t		ts_;

	uint32_t		count_;

	StatPacketObj*	stat_;
	uint64_t		tick_count_;
};

#endif

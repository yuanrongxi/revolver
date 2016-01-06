#ifndef __REVOLVER_RUDP_CONNECTION_H__
#define __REVOLVER_RUDP_CONNECTION_H__

#include "revolver/object_pool.h"
#include "revolver/base_thread_mutex.h"
#include "revolver/base_event_handler.h"
#include "core/core_connection.h"

#include "rudp/rudp_stream.h"
#include "rudp/rudp_interface.h"
#include "revolver/base_packet.h"
#include "revolver/base_block_buffer.h"
#include "revolver/base_bin_stream.h"


using namespace BASE;

typedef CReciverBuffer_T<BinStream, RUDPStream, 1024 * 16>  RUDPRBuffer;
typedef CSenderBuffer_T<BinStream, RUDPStream, 1024 * 16>   RUDPSBuffer;


class RudpConnection : public CEventHandler, public CConnection,
    public RUDPEventHandler
{
public:
    RudpConnection();
    virtual ~RudpConnection();

public:
    int32_t			rudp_accept_event(int32_t rudp_id);
    int32_t			rudp_input_event(int32_t rudp_id);
    int32_t			rudp_output_event(int32_t rudp_id);
    int32_t			rudp_close_event(int32_t rudp_id);
    int32_t			rudp_exception_event(int32_t rudp_id);

public:
    int32_t			connect(const Inet_Addr& dst_addr);
    void			close();
    void			force_close();
    int32_t			send(CBasePacket& packet, bool no_delay = false);
    int32_t			send(BinStream& strm);

public:
    void			reset();

    void			set_timer(uint32_t delay);
    void			cancel_timer();
    int32_t			handle_timeout(const void *act, uint32_t timer_id);

public:
    void			set_state(uint8_t state) { state_ = state; };
    uint8_t			get_state() const { return state_; };
    RUDPStream&		get_rudp_stream() { return rudp_sock_; };
    const Inet_Addr& get_remote_addr() { rudp_sock_.get_peer_addr(remote_addr_); return remote_addr_; };
    void			set_remote_addr(const Inet_Addr& remote_addr) { remote_addr_ = remote_addr; };

    const Inet_Addr& get_local_addr() const;
    void            set_local_addr(const Inet_Addr& addr) { local_addr_ = addr; }
    void			get_send_state(uint32_t& bw, uint32_t& cache_size);
protected:
    void			process(CBasePacket &packet);
    void			send_packet();

protected:
    RUDPStream		rudp_sock_;

    uint32_t		timer_id_;

    Inet_Addr		remote_addr_;
    Inet_Addr		local_addr_;

    RUDPRBuffer		rbuffer_;			//接收BUFFER
    RUDPSBuffer		sbuffer_;			//发送BUFFER

    uint8_t			state_;

    BinStream		istrm_;
};

extern ObjectMutexPool<RudpConnection, BaseThreadMutex, 512> RUDPCONNPOOL;


#define GAIN_RUDP_CONN(conn) \
    RudpConnection* conn = RUDPCONNPOOL.pop_obj(); \
    conn->reset()

#define RETURN_RUDP_CONN(conn) \
if (conn != NULL) \
{\
    conn->reset(); \
    RUDPCONNPOOL.push_obj(conn); \
}


#endif // __REVOLVER_RUDP_CONNECTION_H__
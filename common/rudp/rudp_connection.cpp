#include "revolver/base_reactor_instance.h"
#include "rudp/rudp_connection.h"
#include "core/core_packet.h"
#include "rudp/rudp_socket.h"
#include "rudp/rudp_log_macro.h"
#include "core/core_message_processor.h"

BASE_NAMESPACE_BEGIN_DECL

#define RUDP_CONNECT_DELAY 25000

//ObjectMutexPool<RudpConnection, BaseThreadMutex, 512> RUDPCONNPOOL;

RudpConnection::RudpConnection()
{
    timer_id_ = 0;
    state_ = CONN_IDLE;
}

RudpConnection::~RudpConnection()
{
    reset();
}

void RudpConnection::reset()
{
    cancel_timer();

    state_ = CONN_IDLE;

    rbuffer_.reset();
    sbuffer_.reset();

    istrm_.rewind(true);

    rudp_sock_.close();
}

void RudpConnection::set_timer(uint32_t delay)
{
    if (timer_id_ == 0)
        timer_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, delay);
}

void RudpConnection::cancel_timer()
{
    if (timer_id_ != 0)
    {
        const void* act = NULL;
        REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);
        timer_id_ = 0;
    }
}

int32_t RudpConnection::handle_timeout(const void *act, uint32_t timer_id)
{
    if (timer_id == timer_id_)
    {
        timer_id_ = 0;

        switch (state_)
        {
        case CONN_CONNECTING:
            this->close();
            break;
        }
    }

    return 0;
}

int32_t RudpConnection::connect(const Inet_Addr& dst_addr)
{
    local_addr_ = RUDP_CLI()->get_local_addr();

    if (rudp_sock_.open(local_addr_) != 0)
    {
        return -1;
    }

    state_ = CONN_CONNECTING;

    if (rudp_sock_.connect(dst_addr) != 0)
    {
        this->close();
        return -1;
    }

    //local_addr_ = src_addr;

    rudp_setoption(rudp_sock_.get_handler(), RUDP_TIMEOUT_COUNT, 2); //6秒超时

    //绑定一个事件器
    RUDP()->bind_event_handle(rudp_sock_.get_handler(), this);
    set_timer(RUDP_CONNECT_DELAY);

    return 0;
}

void RudpConnection::close()
{
    RUDP()->bind_event_handle(rudp_sock_.get_handler(), NULL);
    rudp_close_event(rudp_sock_.get_handler());
}

void RudpConnection::force_close()
{
    RUDP()->bind_event_handle(rudp_sock_.get_handler(), NULL);
    rudp_sock_.force_close();
    rudp_close_event(rudp_sock_.get_handler());
}

int32_t RudpConnection::send(CBasePacket& packet, bool no_delay /* = false */)
{
    if (get_state() != CONN_CONNECTED)
        return -1;

    int32_t ret = -1;
    GAIN_BINSTREAM(bin_strm);
    *bin_strm << packet;

    if (sbuffer_.remaining_length() < bin_strm->data_size() + sizeof(uint32_t))
    {
        if (sbuffer_.length() < MAX_BUFFER_SIZE)//扩大TCP发送缓冲区,防止缓冲区太小造成发送包异常
        {
            sbuffer_.realloc_buffer(bin_strm->data_size());
            CORE_WARNING("sbuffer realloc buffer, size = " << sbuffer_.length());
        }
        else //发送报文丢弃
        {
            CORE_ERROR("sbuffer is full, sbuffer.size = " << sbuffer_.length());
            RETURN_BINSTREAM(bin_strm);
            return -1;
        }
    }

    if (sbuffer_.put(*bin_strm))
    {
        if (no_delay)
        {
            sbuffer_.send(rudp_sock_);
        }


        if (sbuffer_.data_length() > 0)
        {
            RUDP()->register_event(rudp_sock_.get_handler(), MASK_WRITE);
        }
        ret = 0;
    }

    RETURN_BINSTREAM(bin_strm);

    return ret;
}

int32_t RudpConnection::send(BinStream& strm)
{
    uint32_t ret = -1;
    if (sbuffer_.remaining_length() < strm.data_size() + sizeof(uint32_t))
    {
        if (sbuffer_.length() < MAX_BUFFER_SIZE)//扩大TCP发送缓冲区,防止缓冲区太小造成发送包异常
        {
            sbuffer_.realloc_buffer(strm.data_size());
        }
        else //发送报文丢弃
        {
            return -1;
        }
    }

    if (sbuffer_.put(strm))
    {
        RUDP()->register_event(rudp_sock_.get_handler(), MASK_WRITE);
        ret = 0;
    }

    return ret;
}

int32_t RudpConnection::rudp_accept_event(int32_t rudp_id)
{
    return 0;
}

int32_t RudpConnection::rudp_input_event(int32_t rudp_id)
{
    if (state_ == CONN_CONNECTING)
    {
        return 0;
    }

    while (true)
    {
        int32_t rc = 0;
        if (rbuffer_.remaining_length() == 0) //扩大TCP接收缓冲区,防止缓冲区太小造成收包异常
        {
            if (rbuffer_.length() < MAX_BUFFER_SIZE)
                rbuffer_.realloc_buffer(rbuffer_.length() - 1);
            else
            {
                return -1;
            }
        }

        rc = rbuffer_.recv(rudp_sock_);
        if (rc > 0)
        {
            bool split_flag = true;

            while (split_flag) //消息组装
            {
                int32_t split_ret = rbuffer_.split(istrm_);
                if (split_ret == 0)
                {

                    try{ istrm_ >> recv_packet_; }
                    catch (...)
                    {
                        RUDP_WARNING("parse received packet encounter error, close the connection(" << rudp_id << ")");
                        this->close();
                        return -2;
                    }

                    process(recv_packet_, istrm_);
                }
                else if (split_ret < 0) //非法报文
                {
                    RUDP_WARNING("illeagle received packet, close the connection(" << rudp_id << ")");
                    this->close();
                    return -2;
                }
                else //报文长度不够判断
                {
                    split_flag = false;
                }
            }
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

int32_t RudpConnection::rudp_output_event(int32_t rudp_id)
{
    if (state_ == CONN_CONNECTING)
    {
        cancel_timer();
        state_ = CONN_CONNECTED;

        rudp_sock_.get_peer_addr(remote_addr_);
        rudp_sock_.get_local_addr(local_addr_);
    }

    if (sbuffer_.data_length() > 0)
    {
        sbuffer_.send(rudp_sock_);
    }

    return 0;
}

int32_t RudpConnection::rudp_close_event(int32_t rudp_id)
{
    if (state_ != CONN_IDLE)
    {
        //通知SESSION
        MSG_PROCESSOR()->on_disconnect(server_id_, this);
        RETURN_RUDP_CONN(this);
    }

    return 0;
}

int32_t RudpConnection::rudp_exception_event(int32_t rudp_id)
{
    if (state_ != CONN_IDLE)
    {
        //通知SESSION
        RETURN_RUDP_CONN(this);
    }

    return 0;
}

void RudpConnection::process(CCorePacket& packet, BinStream& istrm)
{
    //GROUP_MANAGER()->on_message(packet, this);
    //send(packet);
    //RUDP_DEBUG("receive rudp payload packet, data size: " << packet.ctx.length());
    //消息处理
    MSG_PROCESSOR()->on_message(packet, istrm, this);

}

void RudpConnection::get_send_state(uint32_t& send_bw, uint32_t& cache_size)
{
    send_bw = 0;
    cache_size = 0;

    if (state_ == CONN_CONNECTED)
    {
        send_bw = rudp_send_bandwidth(rudp_sock_.get_handler());
        cache_size = rudp_send_cache_size(rudp_sock_.get_handler());
    }
}

const Inet_Addr& RudpConnection::get_local_addr() const
{
    return local_addr_;
}


int32_t RudpClientCtx::open(const Inet_Addr& local_addr) {
    _rudp_handler.attach_adapter(&_adapter);
    if (_rudp_handler.open(local_addr) == 0)
    {
        CORE_INFO("open rudp client, client addr = " << local_addr);
        return 0;
    }
    else
    {
        CORE_ERROR("open rudp client failed, client addr = " << local_addr << ",error = " << error_no());
        return -1;
    }
    return 0;
}

int32_t RudpClientCtx::close() {
    _rudp_handler.close();
    return 0;
}


int32_t create_rudp_client(uint16_t port) {
    CREATE_RUDP_CLI();
    Inet_Addr addr(INADDR_ANY, port);
    return RUDP_CLI()->open(addr);
}

void destroy_rudp_client() {
    RUDP_CLI()->close();
    DESTROY_RUDP_CLI();
}

BASE_NAMESPACE_END_DECL



/*************************************************************************************
*filename:	core_connection.h
*
*to do:		CORE连接对象基础，实现TCP连接数据收发处理流程
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_CONNECTION_H
#define __CORE_CONNECTION_H

#include "revolver/base_bin_stream.h"
#include "revolver/base_event_handler.h"
#include "revolver/base_sock_stream.h"
#include "core/core_reciver_interface.h"
#include "revolver/base_block_buffer.h"
#include "revolver/base_packet.h"
#include "core/core_log_macro.h"
#include "core/core_connection_base.h"

#include <list>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL


class CCorePacket;
class CoreExpPacket;

class CCoreConnection : public CConnection
{
public:
    CCoreConnection();
    virtual ~CCoreConnection();

public:
    void			reset();
    CSockStream&	get_sock_stream();

    BASE_HANDLER	get_handle() const;
    void			set_handle(BASE_HANDLER handle);

    //事件接口
    int32_t			handle_input(BASE_HANDLER handle);
    int32_t			handle_output(BASE_HANDLER handle);
    int32_t			handle_close(BASE_HANDLER handle, ReactorMask close_mask);
    int32_t			handle_exception(BASE_HANDLER handle);
    int32_t			handle_timeout(const void *act, uint32_t timer_id);

    //发起一条TCP连接
    int32_t			connect(const Inet_Addr& remote_addr);
    int32_t			connect(const Inet_Addr& src_addr, const Inet_Addr& dst_addr);
    //关闭连接
    void			close();
    void			extern_close();

    //发送数据
    int32_t			send(CBasePacket/*CCorePacket*/& packet, bool no_delay = false);
    int32_t			send(const string& bin_stream);
    int32_t			send(BinStream& bin_stream, bool no_delay);

protected:
    int32_t			process(CCorePacket &packet, BinStream& istrm);
    void			process_ping(const CCorePacket &packet);
    int32_t			process_handshake(const CCorePacket &packet, BinStream& istrm);

    void			send_ping();
    void			send_handshake();

    void			check_connecting_state();
    void			buffer_reduce();

    uint32_t		set_timer(uint32_t timer_type, uint32_t delay = 60000); 
    void			cancel_timer();
    void			release_timer_act(const void* act);

    int32_t			heartbeat();
    
    void			generate_digest(uint32_t server_id, uint8_t server_type, string& digest_data);
protected:
    uint32_t		timer_id_;

    CSockStream		sock_stream_;

    SBuffer			sbuffer_;			//发送BUFFER
    RBuffer			rbuffer_;			//接收BUFFER

    BinStream		istrm_;

    uint32_t		timer_count_;		//定时器计数

    string			conn_name_;
    bool			send_flag_;			//发送报文标志
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/


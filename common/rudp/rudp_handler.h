#ifndef __REVOLVER_RUDP_HANDLER_H__
#define __REVOLVER_RUDP_HANDLER_H__

#include "revolver/base_event_handler.h"
#include "revolver/base_sock_dgram.h"
#include "revolver/base_bin_stream.h"
#include "rudp/rudp_adapter.h"
#include "core/core_bandwidth_throttler.h"

BASE_NAMESPACE_BEGIN_DECL

class RudpHandler : public CEventHandler {
public:
    RudpHandler();
    virtual ~RudpHandler();

    bool is_open() const;

    int32_t open(const Inet_Addr& local_addr);
    int32_t close();

    void attach_adapter(IRUDPAdapter *adaptor) { adapter_ = adaptor; }

    int32_t send(BinStream& bin_strm, const Inet_Addr& remote_addr);
    const Inet_Addr& get_local_addr() const { return local_addr_; };

    BASE_HANDLER get_handle() const;

    virtual int32_t	handle_input(BASE_HANDLER handle);
    virtual int32_t handle_output(BASE_HANDLER handle);
    virtual int32_t handle_close(BASE_HANDLER handle, ReactorMask close_mask);
    virtual int32_t handle_exception(BASE_HANDLER handle);

    void get_net_stat(uint32_t &up_band, uint32_t &up_cnt, uint32_t &down_band, uint32_t &down_cnt);

protected:
    BinStream bin_strm_;
    CSockDgram sock_dgram_;
    IRUDPAdapter *adapter_;
    Inet_Addr local_addr_;	//本地绑定的网卡地址
    CoreThrottler throllter_;
};

BASE_NAMESPACE_END_DECL

#endif // __REVOLVER_RUDP_HANDLER_H__
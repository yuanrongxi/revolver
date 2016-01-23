#ifndef __CORE_CONNECTION_BASE_H__
#define __CORE_CONNECTION_BASE_H__

#include "revolver/base_namespace.h"
#include "revolver/base_event_handler.h"
#include "revolver/base_inet_addr.h"
#include "core/core_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL

//16K队列
typedef CReciverBuffer_T<BinStream, CSockStream, 1024 * 4>		RBuffer_16K;
typedef CSenderBuffer_T<BinStream, CSockStream, 1024 * 4>		SBuffer_16K;
//256K队列
typedef CReciverBuffer_T<BinStream, CSockStream, 1024 * 256>	RBuffer_256K;
typedef CSenderBuffer_T<BinStream, CSockStream, 1024 * 256>		SBuffer_256K;
//1M队列
typedef CReciverBuffer_T<BinStream, CSockStream, 1024 * 1024>	RBuffer_1M;
typedef CSenderBuffer_T<BinStream, CSockStream, 1024 * 1024>	SBuffer_1M;
//10M队列
typedef CReciverBuffer_T<BinStream, CSockStream, 1024 * 1280 * 8>	RBuffer_10M;
typedef CSenderBuffer_T<BinStream, CSockStream, 1024 * 1280 * 8>	SBuffer_10M;

#define __16K_BUFFER

#ifdef __10M_BUFFER
typedef RBuffer_10M		RBuffer;
typedef SBuffer_10M		SBuffer;
#endif

#ifdef __1M_BUFFER
typedef RBuffer_1M		RBuffer;
typedef SBuffer_1M		SBuffer;
#endif

#ifdef __256K_BUFFER
typedef RBuffer_256K	RBuffer;
typedef SBuffer_256K	SBuffer;
#endif

#ifdef __16K_BUFFER
typedef RBuffer_16K		RBuffer;
typedef SBuffer_16K		SBuffer;
#endif

class CBasePacket;

class CConnection : public CEventHandler {
public:
    enum
    {
        CONN_IDLE,			//平静状态,有可能是未连接
        CONN_CONNECTING,	//连接状态
        CONN_CONNECTED,		//连接状态
    };

    CConnection() : state_(CONN_IDLE) {
        server_id_ = 0;
        server_type_ = 0;
        index_ = 0;
    }
    virtual ~CConnection() {}

    uint32_t get_server_id() const { return server_id_; };
    void set_server_id(uint32_t server_id){ server_id_ = server_id; };

    //获取端的信息
    uint8_t get_server_type() const{ return server_type_; };
    void set_server_type(uint8_t server_type){ server_type_ = server_type; };

    const Inet_Addr& get_remote_addr() const { return remote_addr_; };
    void set_remote_addr(const Inet_Addr& remote_addr) { remote_addr_ = remote_addr; };

    uint32_t get_index() const{ return index_; };
    void set_index(uint32_t index){ index_ = index; };

    void set_conn_ptr(void* ptr) { conn_ptr_ = ptr; }
    void* get_conn_ptr() { return conn_ptr_; }

    uint16_t get_state() const { return state_; };
    void set_state(uint16_t state) {
        state_ = state;
        CORE_DEBUG("CConnection, state = CONN_CONNECTING");
    }

    bool is_open() const
    {
        return (get_handle() != INVALID_HANDLER);
    }

public:
    virtual int32_t send(CBasePacket& packet, bool no_delay = false) = 0;
    virtual int32_t send(const string& bin_stream) = 0;
    //发起一条TCP连接
    virtual int32_t connect(const Inet_Addr& remote_addr) = 0;
    //关闭连接
    virtual void close() = 0;

protected:
    uint32_t server_id_;		//端ID
    uint8_t server_type_;		//0表示客户端

    Inet_Addr remote_addr_;

    uint32_t index_;  //连接映射索引
    void* conn_ptr_;  //连接的特征信息

    uint16_t state_;
};

BASE_NAMESPACE_END_DECL

#endif // __CORE_CONNECTION_BASE_H__

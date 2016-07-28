#include <limits.h>
#include "gtest/gtest.h"

#include "rudp/rudp_recv_buffer.h"

using namespace BASE;

static LossIDArray _nack_ids;
uint64_t _nack_first_seq_no = 0;

class RudpEventListener : public IRUDPNetChannel {
  public:
    //BUFFER报文发送接口
    void send_ack(uint64_t ack_seq_id) {

    }
    void send_nack(uint64_t base_seq_id, const LossIDArray& ids) {
        _nack_first_seq_no = base_seq_id;
        _nack_ids = ids;
    }
    void send_data(uint64_t ack_seq_id, uint64_t cur_seq_id, const uint8_t* data, uint16_t data_size, uint64_t now_ts) {

    }

    //BUFFER状态接口,BUFFER可以进行数据发送通告
    void on_write() {

    }
    //buffer可以进行数据读取
    void on_read() {

    }
    //错误通告
    void on_exception() {

    }

    void reset() {

    }
};

static RudpEventListener _rudp_ev_listener;

class RudpRecvBufferTest : public testing::Test, public RUDPRecvBuffer {
  protected:
    virtual void SetUp() {
        _rudp_ev_listener.reset();
        set_net_channel(&_rudp_ev_listener);
    }

    virtual void TearDown() {
        set_net_channel(0);
        _rudp_ev_listener.reset();
    }

};

const char* data = "123456";

TEST_F(RudpRecvBufferTest, NackSeqGenerate) {
    on_data(1, (uint8_t*)data, 6);
    on_data(3, (uint8_t*)data, 6);
    on_data(5, (uint8_t*)data, 6);
    on_data(6, (uint8_t*)data, 6);

    check_loss();

    EXPECT_EQ(_nack_first_seq_no, 1);
    EXPECT_EQ(_nack_ids.size(), 2);
    
    EXPECT_EQ(_nack_ids[0], 1);
    EXPECT_EQ(_nack_ids[1], 3);
}
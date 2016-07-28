#include <limits.h>
#include "gtest/gtest.h"

#include "revolver/base_os.h"
#include "revolver/base_timer_value.h"

#include "rudp/rudp_send_buffer.h"
#include "rudp/rudp_ccc.h"

using namespace BASE;
static vector<uint64_t> _loss_seq_no;
class RudpSendEventListener : public IRUDPNetChannel {
public:
    //BUFFER报文发送接口
    void send_ack(uint64_t ack_seq_id) {

    }
    void send_nack(uint64_t base_seq_id, const LossIDArray& ids) {
    }
    void send_data(uint64_t ack_seq_id, uint64_t cur_seq_id, const uint8_t* data, uint16_t data_size, uint64_t now_ts) {
        std::cout << "seq: " << cur_seq_id << ", data size: " << data_size << std::endl;
        _loss_seq_no.push_back(cur_seq_id);
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

static RudpSendEventListener  _send_ev_listner;

class RudpSendBufferTest : public testing::Test, public RUDPSendBuffer {
protected:
    virtual void SetUp() {
        set_ccc(&_ccc);
        set_net_channel(&_send_ev_listner);

    }

    virtual void TearDown() {
        set_new_handler(0);
        set_ccc(0);
    }

protected:
    RUDPCCCObject _ccc;
};

static const char* send_data = "123456";

TEST_F(RudpSendBufferTest, SendInPassiveMode) {
    set_passive(1);
    uint64_t base_seq = buffer_seq_;

    // send 6 packets.
    usleep(1000 * 1000);
    send((uint8_t*)send_data, 6);
    //cout << send_data_.size() << endl;
    //usleep(100 * 1000);
    send((uint8_t*)send_data, 6);
    //cout << send_data_.size() << endl;
    //usleep(100 * 1000);
    send((uint8_t*)send_data, 6);
    send((uint8_t*)send_data, 6);
    send((uint8_t*)send_data, 6);
    send((uint8_t*)send_data, 6);

    ASSERT_EQ(buffer_seq_, base_seq + 6);
    usleep(100 * 1000);
    attempt_send(CBaseTimeValue::get_time_value().msec());
    EXPECT_EQ(send_window_.size(), 6);

    LossIDArray ids;
    ids.push_back(2);
    ids.push_back(4);
    on_nack(base_seq, ids);

    ASSERT_EQ(_loss_seq_no.size(), 8);
    EXPECT_EQ(_loss_seq_no[2], base_seq + 2);
    EXPECT_EQ(_loss_seq_no[4], base_seq + 4);
    EXPECT_EQ(_loss_seq_no[6], base_seq + 2);
    EXPECT_EQ(_loss_seq_no[7], base_seq + 4);

    _loss_seq_no.clear();
}

TEST_F(RudpSendBufferTest, SendTimeout) {
    reset();

}
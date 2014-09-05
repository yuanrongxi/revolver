#include "revolver/base_reactor_instance.h"
#include "rudp/rudp_interface.h"
#include "rudp/rudp_socket.h"
#include "revolver/base_timer_value.h"
#include "rudp/rudp_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL

//每6秒发送一次KEEPLIVE
#define DEFAULT_KEEPLIVE			3000
//默认2分钟心跳未收到就断开
#define DEFAULT_TIMEOUT_COUNT		40
#define CONNECT_DELAY				1000

#define SYN_MAX_COUNT				20
#define FIN_MAX_COUNT				8

#define TIMER_MIN_DELAY				50

#define MIN_SEND_BUFFER_SIZE		4096
	

RUDPSocket::RUDPSocket()
{
	timer_id_ = 0;
	rudp_id_ = -1;
	local_index_ = INVALID_ADAPTER_INDEX;
	local_title_ = 0; //用于区分UDP还是RUDP

	remote_rudp_id_ = INVALID_RUDP_HANDLE;

	heart_ts_ = CBaseTimeValue::get_time_value().msec();;
	keeplive_intnal_ = DEFAULT_KEEPLIVE;
	keeplive_count_ = 0;
	timeout_count_ = DEFAULT_TIMEOUT_COUNT;

	event_handler_ = NULL;
	state_ = RUDP_IDLE;

	error_code_ = RUDP_SUCCESS;
	send_count_ = 0;

	send_buffer_.set_net_channel(this);
	send_buffer_.set_ccc(&ccc_);

	recv_buffer_.set_net_channel(this);

	check_sum_ = 0;
}

RUDPSocket::~RUDPSocket()
{
	reset();

	clear_timer_events();
}

void RUDPSocket::reset()
{
	if(state_ != RUDP_IDLE)
	{
		RUDP_DEBUG("state = RUDP_IDLE, rudp id = " << rudp_id_);
		state_ = RUDP_IDLE; 
	}

	rudp_id_ = -1;
	local_index_ = INVALID_ADAPTER_INDEX;
	local_title_ = 0;

	remote_rudp_id_ = INVALID_RUDP_HANDLE;

	heart_ts_ = CBaseTimeValue::get_time_value().msec();
	keeplive_intnal_ = DEFAULT_KEEPLIVE;
	keeplive_count_ = 0;
	timeout_count_ = DEFAULT_TIMEOUT_COUNT;

	event_handler_ = NULL;

	error_code_ = RUDP_SUCCESS;

	send_count_ = 0;

	cancel_timer();

	ccc_.reset();

	send_buffer_.reset();
	recv_buffer_.reset();

	check_sum_ = 0;
}

void RUDPSocket::set_state(uint16_t state)
{
	if(state_ >= state)
		return ;

	state_ = state;
	send_count_ = 0;

	cancel_timer();

	//释放SOCKET对象
	if(state == RUDP_CLOSE)
	{
		RUDP()->free_sockets(rudp_id_);
	}
}

void RUDPSocket::set_timer(uint32_t delay)
{
	if(timer_id_ == 0)
		timer_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, delay);
}

void RUDPSocket::cancel_timer()
{
	if(timer_id_ != 0)
	{
		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);
		timer_id_ = 0;
	}
}

int32_t RUDPSocket::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id_ != timer_id)
		return 0;

	timer_id_= 0;

	//解决状态定时的问题，尤其是报文发送
	switch(state_)
	{
	case RUDP_CONNECTING:
		if(send_count_ < SYN_MAX_COUNT)
		{
			send_syn();
			set_timer(CONNECT_DELAY);
			RUDP_DEBUG("resend syn, rudp socket id = " << rudp_id_);
			send_count_ ++;
		}
		else //连接超时
		{
			RUDP_INFO("connecting timeout! state = RUDP_FIN2_STATE, rudp id = " << rudp_id_);
			set_state(RUDP_FIN2_STATE);

			if(event_handler_ != NULL)
				event_handler_->rudp_exception_event(rudp_id_);
			else
			{
				RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
				set_state(RUDP_CLOSE);
			}
		}
		break;

	case RUDP_FIN_STATE:
		if(send_count_ < FIN_MAX_COUNT)
		{
			RUDP_DEBUG("resend fin, rudp socket id = " << rudp_id_);
			send_fin();
			set_timer(ccc_.get_rtt() + TIMER_MIN_DELAY);
			send_count_ ++;
		}
		else
		{
			RUDP_INFO("fin timeout, state = RUDP_CLOSE, rudp id = " << rudp_id_);
			set_state(RUDP_CLOSE);
		}
		break;
	}

	return 0;
}

int32_t RUDPSocket::open(int32_t rudp_id)
{
	reset();

	rudp_id_ = rudp_id;

	return 0;
}
//正常关闭RUDP SOCKET
void RUDPSocket::close()
{
	switch(state_)
	{
	case RUDP_CONNECTING:
	case RUDP_CONNECTED:
		if(event_handler_ != NULL)
		{
			event_handler_->rudp_close_event(rudp_id_);
		}

		//发送SYN
		RUDP_INFO("close rudp socket, state = RUDP_FIN_STATE, rudp id = " << rudp_id_);
		set_state(RUDP_FIN_STATE);
		
		RUDP_DEBUG("send fin, rudp socket id = " << rudp_id_);
		send_fin();
		set_timer(ccc_.get_rtt() + TIMER_MIN_DELAY);

		send_count_ ++;
		break;

	case RUDP_FIN2_STATE:
		set_state(RUDP_CLOSE);
		break;
	}
}
//强制关闭RUDP SOCKET
void RUDPSocket::force_close()
{
	switch(state_)
	{
	case RUDP_CONNECTING:
	case RUDP_CONNECTED:
		RUDP_DEBUG("send fin, rudp socket id = " << rudp_id_);
		for(uint8_t i = 0; i < 6; i ++) //直接发送3个fin
			send_fin();

	case RUDP_FIN2_STATE:
		RUDP_INFO("state = RUDP_CLOSE");
		set_state(RUDP_CLOSE);
		break;
	}
}

int32_t RUDPSocket::bind(uint8_t index, uint8_t title)
{
	if(rudp_id_ == -1 || state_ != RUDP_IDLE)
	{
		RUDP_FATAL("rudp socket bind failed!");
		error_code_ = RUDP_BIND_FAIL;
		return -1;
	}

	local_index_ = index;
	local_title_ = title;

	return 0;
}

int32_t RUDPSocket::connect(const Inet_Addr& remote_addr)
{
	if(rudp_id_ == INVALID_RUDP_HANDLE || state_ != RUDP_IDLE 
		|| local_index_ == 255 || remote_rudp_id_ != INVALID_RUDP_HANDLE)
	{
		RUDP_FATAL("rudp connect failed! rudp socket id = " << rudp_id_);
		error_code_ = RUDP_CONNECT_FAIL;
		return -1;
	}

	remote_addr_ = remote_addr;
	check_sum_ = rand() % 65536; //产生一个会话的CHECK SUM

	RUDP_INFO("state = RUDP_CONNECTING, rudp id = " << rudp_id_ << ", remote addr = " << remote_addr_);
	set_state(RUDP_CONNECTING);

	RUDP_DEBUG("send syn, rudp socket id = " << rudp_id_);
	send_syn();
	ccc_.init(send_buffer_.get_buffer_seq() - 1);
	set_timer(CONNECT_DELAY);
	send_count_ ++;

	return 0;
}

int32_t RUDPSocket::setoption(int32_t op_type, int32_t op_value)
{
	if(rudp_id_ == -1 || state_ > RUDP_FIN_STATE)
	{
		RUDP_FATAL("setoption failed!");

		return -1;
	}

	int32_t ret = 0;
	switch(op_type)
	{
	case RUDP_KEEPLIVE:
		RUDP_INFO("keep live intnal = " << op_value << "ms, rudp socket id = " << rudp_id_);
		keeplive_intnal_ = op_value > DEFAULT_KEEPLIVE ? op_value : DEFAULT_KEEPLIVE;
		break;

	case RUDP_NAGLE:
		if(op_value == 0)
		{
			send_buffer_.set_nagle(false);
			RUDP_INFO("cancel nagle, rudp socket id = " << rudp_id_);
		}
		else
		{
			send_buffer_.set_nagle(true);
			RUDP_INFO("set nagle, rudp socket id = " << rudp_id_);
		}
		break;

	case RUDP_RECV_BUFF_SIZE:
		break;

	case RUDP_SEND_BUFF_SIZE:
		send_buffer_.set_buffer_size(op_value > MIN_SEND_BUFFER_SIZE ? op_value : MIN_SEND_BUFFER_SIZE);
		RUDP_INFO("set send buffer, buffer size = " << op_value);
		break;

	case RUDP_TIMEOUT_COUNT:
		timeout_count_ = op_value > 2 ? op_value : 2;
		RUDP_INFO("set rudp timeout delay = " << timeout_count_ * 6 << "s");
		break;

	default:
		ret = -1;
	}

	return ret;
}

int32_t RUDPSocket::send(const uint8_t* data, int32_t data_size)
{
	if(state_ != RUDP_CONNECTED || data_size <= 0)
	{
		RUDP_FATAL("send failed! state_ != RUDP_CONNECTED or data size = 0, rudp id = " << rudp_id_);
		error_code_ = RUDP_SEND_ERROR;
		return -1;
	}

	int32_t ret = send_buffer_.send(data, data_size);
	if(ret <= 0)
	{
		RUDP_INFO("send buffer is full, rudp socket id = " << rudp_id_);
		error_code_ = RUDP_SEND_EAGIN;
	}

	return ret;
}

int32_t RUDPSocket::recv(uint8_t* data, int32_t data_size)
{
	if(state_ != RUDP_CONNECTED || data_size <= 0)
	{
		RUDP_FATAL("recv failed! state_ != RUDP_CONNECTED or data size = 0, rudp id = " << rudp_id_);
		error_code_ = RUDP_SEND_ERROR;
		return -1;
	}

	return recv_buffer_.read(data, data_size);

}

void RUDPSocket::send_ack(uint64_t ack_seq_id)
{
	if(state_ != RUDP_CONNECTED || remote_rudp_id_ < 0)
	{
		RUDP_WARNING("send ack failed! rudp socket id = " << rudp_id_);
		return ;
	}

	RUDPHeadPacket head;
	head.msg_id_ = RUDP_DATA_ACK;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;
	
	RUDPDataAck ack;
	ack.ack_seq_id_ = ack_seq_id;

	strm_.rewind(true);
	strm_ << local_title_ << head << ack;

	//RUDP_DEBUG("send ack, rudp_id = " << rudp_id_ << ", seq = " << ack_seq_id);

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_nack(uint64_t base_seq_id, const LossIDArray& ids)
{
	if(state_ != RUDP_CONNECTED || remote_rudp_id_ < 0)
	{
		RUDP_WARNING("send nack failed! rudp socket id = " << rudp_id_);
		return ;
	}

	RUDPHeadPacket head;
	head.msg_id_ = RUDP_DATA_NACK;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RUDPDataNack nack;
	nack.base_seq_ = base_seq_id;
	nack.loss_ids_ = ids;

	strm_.rewind(true);
	strm_ << local_title_ << head << nack;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_data(uint64_t ack_seq_id, uint64_t cur_seq_id, const uint8_t* data, uint16_t data_size, uint64_t now_ts)
{
	if(state_ != RUDP_CONNECTED || remote_rudp_id_ < 0 || data_size <= 0)
	{
		RUDP_WARNING("send data failed! rudp socket id = " << rudp_id_);
		return ;
	}

	RUDPHeadPacket head;
	head.msg_id_ = RUDP_DATA;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RUDPData body;
	body.ack_seq_id_ = recv_buffer_.get_ack_id();
	body.cur_seq_id_ = cur_seq_id;
	//body.data_.assign((const char*)data, data_size); //todo:减少一次拷贝

	//设置一个最后发送ACK的时刻
	recv_buffer_.set_send_last_ack_ts(now_ts);

	strm_.rewind(true);
	strm_ << local_title_ << head << body;
	//加入数据
	strm_.push_data(data, data_size);

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_syn()
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_SYN;
	head.remote_rudp_id_ = INVALID_RUDP_HANDLE;
	head.check_sum_ = check_sum_;

	RUDPSynPacket syn;
	syn.version_ = RUDP_VERSION;
	syn.local_rudp_id_ = rudp_id_;
	syn.local_ts_ = CBaseTimeValue::get_time_value().msec();
	syn.max_segment_size_ = MAX_SEGMENT_SIZE;
	syn.start_seq_ = send_buffer_.get_buffer_seq();

	strm_.rewind(true);
	strm_ << local_title_ << head << syn;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_syn2(uint8_t result, uint64_t remote_ts)
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_SYN2;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RUDPSyn2Packet syn2;
	syn2.version_ = RUDP_VERSION;
	syn2.local_rudp_id_ = rudp_id_;
	syn2.local_ts_ = CBaseTimeValue::get_time_value().msec();
	syn2.remote_ts_ = remote_ts;
	syn2.syn2_result_ = result;
	syn2.max_segment_size_ = MAX_SEGMENT_SIZE;
	syn2.start_seq_ = send_buffer_.get_buffer_seq();

	strm_.rewind(true);
	strm_ << local_title_ << head << syn2;

	for(uint8_t i = 0; i < 3; ++i)
		RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_syn_ack(uint8_t result , uint64_t remote_ts)
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_SYN_ACK;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RUDPSyn2AckPacket syn_ack;
	syn_ack.result_ = result;
	syn_ack.remote_ts_ = remote_ts;

	strm_.rewind(true);
	strm_ << local_title_ << head << syn_ack;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_fin()
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_FIN;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	strm_.rewind(true);
	strm_ << local_title_ << head;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_fin2()
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_FIN2;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	strm_.rewind(true);
	strm_ << local_title_ << head;

	for(uint8_t i = 0; i < 3; i++)
		RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::send_keeplive(uint64_t now_ts)
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_KEEPALIVE;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RDUPKeepLive body;
	body.timestamp_ = now_ts;

	strm_.rewind(true);
	strm_ << local_title_ << head << body;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);

	heart_ts_ = now_ts;
}

void RUDPSocket::send_keeplive_ack(uint64_t ts)
{
	RUDPHeadPacket head;
	head.msg_id_ = RUDP_KEEPALIVE_ACK;
	head.remote_rudp_id_ = remote_rudp_id_;
	head.check_sum_ = check_sum_;

	RDUPKeepLive body;
	body.timestamp_ = ts;

	strm_.rewind(true);
	strm_ << local_title_ << head << body;

	RUDP()->send_udp(local_index_, strm_, remote_addr_);
}

void RUDPSocket::on_write()
{
	if(state_ != RUDP_CONNECTED || event_handler_ == NULL)
	{
		return ;
	}

	event_handler_->rudp_output_event(rudp_id_);
}

void RUDPSocket::on_read()
{
	if(state_ != RUDP_CONNECTED || event_handler_ == NULL)
	{
		return ;
	}

	event_handler_->rudp_input_event(rudp_id_);
}

void RUDPSocket::on_exception()
{
	if(state_ != RUDP_CONNECTED || event_handler_ == NULL)
	{
		return ;
	}

	event_handler_->rudp_close_event(rudp_id_);
}

void RUDPSocket::process(uint8_t msg_id, uint16_t check_sum, BinStream& strm, const Inet_Addr& remote_addr)
{
	if(check_sum != check_sum_)
		return ;

	keeplive_count_ = 0;

	//地址学习
	if(remote_addr_ != remote_addr)
	{
		RUDP()->delete_peer_index(remote_rudp_id_, remote_addr_);
		remote_addr_ = remote_addr;
	}

	switch(msg_id)
	{
	case RUDP_DATA:
		process_data(strm, remote_addr);
		break;

	case RUDP_DATA_ACK:
		process_data_ack(strm, remote_addr);
		break;

	case RUDP_DATA_NACK:
		process_data_nack(strm, remote_addr);
		break;

	case RUDP_SYN2:
		process_syn2(strm, remote_addr);
		break;

	case RUDP_SYN_ACK:
		process_syn_ack(strm, remote_addr);
		break;

	case RUDP_FIN:
		process_fin(strm, remote_addr);
		break;

	case RUDP_FIN2:
		process_fin2(strm, remote_addr);
		break;

	case RUDP_KEEPALIVE:
		process_keeplive(strm, remote_addr);
		break;

	case RUDP_KEEPALIVE_ACK:
		process_keeplive_ack(strm, remote_addr);
		break;
	}
}

void RUDPSocket::process_data(BinStream& strm, const Inet_Addr& remote_addr)
{
	if(state_ != RUDP_CONNECTED)
	{
		RUDP_WARNING("process data, state != RUDP_CONNECTED");
		return ;
	}

	RUDPData data;
	strm >> data;

	ccc_.on_ack(data.ack_seq_id_);
	send_buffer_.on_ack(data.ack_seq_id_);

	uint32_t data_size = 0;
	strm >> data_size;

	recv_buffer_.on_data(data.cur_seq_id_, (const uint8_t *)strm.get_rptr(), data_size);
}

void RUDPSocket::process_data_ack(BinStream& strm, const Inet_Addr& remote_addr)
{
	if(state_ != RUDP_CONNECTED)
	{
		RUDP_WARNING("process_data_ack, state_ != RUDP_CONNECTED");
		return ;
	}

	//RUDP_DEBUG("on recv data ack, remote_addr = " << remote_addr);
	RUDPDataAck ack;
	strm >> ack;

	ccc_.on_ack(ack.ack_seq_id_);
	send_buffer_.on_ack(ack.ack_seq_id_);
}

void RUDPSocket::process_data_nack(BinStream& strm, const Inet_Addr& remote_addr)
{
	if(state_ != RUDP_CONNECTED)
	{
		RUDP_WARNING("process_data_nack, state_ != RUDP_CONNECTED");
		return ;
	}

	RUDPDataNack nack;
	strm >> nack;

	send_buffer_.on_nack(nack.base_seq_, nack.loss_ids_);
	ccc_.on_loss(nack.base_seq_, nack.loss_ids_);

	ccc_.on_ack(nack.base_seq_);
	send_buffer_.on_ack(nack.base_seq_);
}

void RUDPSocket::process_syn(RUDPSynPacket& syn, const Inet_Addr& remote_addr)
{
	RUDP_INFO("recv syn from " << remote_addr << ", rudp id = " << rudp_id_);

	if(state_ == RUDP_IDLE && rudp_id_ != INVALID_RUDP_HANDLE)
	{
		uint64_t now_timer = CBaseTimeValue::get_time_value().msec();
		//初始化接收SEQ
		recv_buffer_.set_first_seq(syn.start_seq_);
		recv_buffer_.set_send_last_ack_ts(now_timer);

		ccc_.init(send_buffer_.get_buffer_seq() - 1);
		//设置连接信息
		remote_addr_ = remote_addr;
		remote_rudp_id_ = syn.local_rudp_id_;

		RUDP_INFO("sart seq = " << syn.start_seq_ << ", remote_rudp_id = " << remote_rudp_id_);

		RUDP_DEBUG("send syn2, rudp socket id = " << rudp_id_);
		send_syn2(0, syn.local_ts_);

		RUDP_INFO("state = RUDP_CONNECTED, rudp id = " << rudp_id_);
		set_state(RUDP_CONNECTED);

		//发送一个KEEPLIVE
		RUDP_DEBUG("send keeplive, rudp socket id =" << rudp_id_);
		send_keeplive(now_timer);
	}
	else if(state_ == RUDP_CONNECTING || state_ == RUDP_CONNECTED)
	{
		RUDP_INFO("send syn2, rudp socket id = " << rudp_id_);
		send_syn2(0, syn.local_ts_);
	}
	else
	{
		RUDP_DEBUG("send syn2(ERROR_SYN_STATE), rudp socket id = " << rudp_id_);
		send_syn2(ERROR_SYN_STATE, syn.local_ts_);

		RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
		set_state(RUDP_CLOSE);
	}
}

void RUDPSocket::process_syn2(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("recv syn2 from " << remote_addr << ", rudp id = " << rudp_id_);

	PARSE_RUDP_MESSAGE(strm, RUDPSyn2Packet, syn2, "parse syn2 failed!");
	if(state_ == RUDP_CONNECTING)
	{
		if(syn2.syn2_result_ != 0x00) //连接异常
		{
			RUDP_INFO("syn failed! syn2.syn2_result_ = " << (uint16_t)syn2.syn2_result_);

			if(event_handler_ != NULL)
			{
				RUDP_INFO("state = RUDP_FIN2_STATE, rudp id = " << rudp_id_);
				set_state(RUDP_FIN2_STATE);
				event_handler_->rudp_exception_event(rudp_id_);
			}
			else
			{
				RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
				set_state(RUDP_CLOSE);
			}

			return ;
		}
		//设置接收BUFF的初始化
		//ccc_.init(syn2.start_seq_ - 1);
		recv_buffer_.set_first_seq(syn2.start_seq_);
		//设置连接信息
		remote_addr_ = remote_addr;
		remote_rudp_id_ = syn2.local_rudp_id_;

		//计算RTT
		uint64_t now_ts = CBaseTimeValue::get_time_value().msec();
		uint32_t rtt = static_cast<uint32_t>(now_ts > syn2.remote_ts_ ? (now_ts - syn2.remote_ts_) : 5);
		recv_buffer_.set_send_last_ack_ts(now_ts);
		ccc_.set_rtt(rtt);

		RUDP_INFO("syn succ, sart seq = " << syn2.start_seq_ << ", remote_rudp_id = " << remote_rudp_id_ << ", rtt = " << rtt);

		RUDP_INFO("state = RUDP_CONNECTED, rudp id = " << rudp_id_);
		set_state(RUDP_CONNECTED);

		RUDP_DEBUG("send syn ack, rudp socket id = " << rudp_id_);
		send_syn_ack(0, syn2.local_ts_);

		//触发一个写事件
		if(event_handler_ != NULL)
			event_handler_->rudp_output_event(rudp_id_);

		heart_ts_ = now_ts;
	}
	else if(state_ == RUDP_CONNECTED)
	{
		RUDP_DEBUG("send syn ack, rudp socket id = " << rudp_id_);
		send_syn_ack(0, syn2.local_ts_);
	}
}

void RUDPSocket::process_syn_ack(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("recv syn ack from " << remote_addr << ", rudp id = " << rudp_id_);

	RUDP()->delete_peer_index(remote_rudp_id_, remote_addr_);

	RUDP_INFO("recv syn ack from " << remote_addr);
	if(state_ != RUDP_CONNECTED)
	{
		RUDP_FATAL("state != RUDP_CONNECTED");
		return ;
	}

	PARSE_RUDP_MESSAGE(strm, RUDPSyn2AckPacket, ack, "parse syn_ack failed!");
	//计算RTT
	uint64_t now_ts = CBaseTimeValue::get_time_value().msec();
	uint32_t rtt = static_cast<uint32_t>(now_ts > ack.remote_ts_ ? (now_ts - ack.remote_ts_) : 5);
	ccc_.set_rtt(rtt);

	RUDP_INFO("rtt = " << rtt << ", rudp socket id = " << rudp_id_);

	//触发一个写事件
	if(event_handler_ != NULL)
		event_handler_->rudp_output_event(rudp_id_);
}

void RUDPSocket::process_fin(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("recv fin from " << remote_addr << ", rudp id = " << rudp_id_);

	RUDP_INFO("send fin2, rudp socket id = " << rudp_id_);
	send_fin2();

	if(state_ == RUDP_CONNECTING || state_ == RUDP_CONNECTED)
	{
		RUDP_INFO("state = RUDP_FIN2_STATE, rudp id = " << rudp_id_);
		set_state(RUDP_FIN2_STATE);

		if(event_handler_ != NULL)
			event_handler_->rudp_close_event(rudp_id_);
		else
		{
			RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
			set_state(RUDP_CLOSE);
		}
	}
	else
	{
		RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
		set_state(RUDP_CLOSE);
	}
	
}

void RUDPSocket::process_fin2(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("recv fin2 from " << remote_addr << ", rudp id = " << rudp_id_);

	if(event_handler_ != NULL)
	{
		event_handler_->rudp_close_event(rudp_id_);
	}
	else
	{
		RUDP_INFO("state = RUDP_CLOSE, rudp id = " << rudp_id_);
		set_state(RUDP_CLOSE);
	}
}

void RUDPSocket::process_keeplive(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("keeplive from " << remote_addr << ", rudp id = " << rudp_id_);

	if(state_ != RUDP_CONNECTED)
	{
		return ;
	}
	
	keeplive_count_ = 0;

	PARSE_RUDP_MESSAGE(strm, RDUPKeepLive, body, "parse keeplive failed!");

	send_keeplive_ack(body.timestamp_);
}

void RUDPSocket::process_keeplive_ack(BinStream& strm, const Inet_Addr& remote_addr)
{
	RUDP_INFO("keeplive ack from " << remote_addr << ", rudp id = " << rudp_id_);

	if(state_ != RUDP_CONNECTED)
	{
		RUDP_WARNING("state != RUDP_CONNECTED");
		return ;
	}

	keeplive_count_ = 0;

	PARSE_RUDP_MESSAGE(strm, RDUPKeepLive, ack, "parse keeplive ack failed!");

	//计算RTT
	uint64_t now_ts = CBaseTimeValue::get_time_value().msec();
	uint32_t rtt = static_cast<uint32_t>(now_ts > ack.timestamp_ ? (now_ts - ack.timestamp_) : 5);
	ccc_.set_rtt(rtt);

	RUDP_INFO("rtt = " << rtt << ", rudp socket id = " << rudp_id_);
}

void RUDPSocket::heartbeat()
{
	if(state_ != RUDP_CONNECTED)
		return ;

	uint64_t now_ts = CBaseTimeValue::get_time_value().msec();

	//心跳计数
	if(now_ts > heart_ts_ + keeplive_intnal_)
	{
		keeplive_count_ ++;
		if(keeplive_count_ > timeout_count_) //超时
		{
			RUDP_ERROR("keep live timeout, rudp socket id = " << rudp_id_);
			if(event_handler_ != NULL) //通知上层异常
			{
				RUDP_INFO("state = RUDP_FIN2_STATE");
				set_state(RUDP_FIN2_STATE);

				event_handler_->rudp_exception_event(rudp_id_);
				return ;
			}
			else
			{
				RUDP_INFO("state = RUDP_CLOSE");
				set_state(RUDP_CLOSE);

				return ;
			}
		}
		else//发送KEEPLIVE
		{
			RUDP_DEBUG("send keeplive, rudp socket id =" << rudp_id_);
			send_keeplive(now_ts);
		}
	}

	//模块心跳
	ccc_.on_timer(now_ts);
	recv_buffer_.on_timer(now_ts, ccc_.get_rtt_var());
	send_buffer_.on_timer(now_ts);
}

void RUDPSocket::register_event(uint32_t marks)
{
	if(state_ != RUDP_CONNECTED)
		return ;

	if((marks & MASK_READ) == MASK_READ)
	{
		recv_buffer_.check_buffer();
	}

	if((marks & MASK_WRITE) == MASK_WRITE)
	{
		send_buffer_.check_buffer();
	}
}

BASE_NAMESPACE_END_DECL


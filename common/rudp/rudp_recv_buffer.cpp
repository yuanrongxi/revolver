#include "rudp/rudp_recv_buffer.h"
#include "rudp/rudp_log_macro.h"
#include "revolver/base_timer_value.h"

BASE_NAMESPACE_BEGIN_DECL

#define MAX_SEQ_INTNAL		204800

RUDPRecvBuffer::RUDPRecvBuffer() 
: net_channel_(NULL)
, first_seq_(0)
, max_seq_(0)
, last_ack_ts_(0)
, recv_new_packet_(false)
, ok_count_(0)
{
	reset();
}

RUDPRecvBuffer::~RUDPRecvBuffer()
{
	reset();
}

void RUDPRecvBuffer::reset()
{
	first_seq_ = 0;
	max_seq_ = 0;
	last_ack_ts_ = 0;
	recv_new_packet_ = false;
	ok_count_ = 0;

	for(RecvWindowMap::iterator it = recv_window_.begin(); it != recv_window_.end(); ++it)
	{
		RETURN_RECV_SEG(it->second);
	}

	recv_window_.clear();

	for(RecvDataList::iterator it = recv_data_.begin(); it != recv_data_.end(); ++it)
	{
		RETURN_RECV_SEG(*it);
	}
	recv_data_.clear();

	loss_map_.clear();

	bandwidth_ = 0;
	bandwidth_ts_ = CBaseTimeValue::get_time_value().msec();
}

int32_t RUDPRecvBuffer::on_data(uint64_t seq, const uint8_t* data, int32_t data_size)
{
	recv_new_packet_ = true;

	//删除丢包
	loss_map_.erase(seq);

	if(seq > first_seq_ + MAX_SEQ_INTNAL || data_size > MAX_SEGMENT_SIZE)
	{
		//报告异常
		RUDP_RECV_DEBUG("on data exception!!");
		net_channel_->on_exception();
		return -1;
	}

	//RUDP_RECV_DEBUG("on data, seq = " << seq);

	RUDPRecvSegment* seg = NULL;
	if(first_seq_ + 1 == seq)
	{
		//将数据缓冲到队列中
		GAIN_RECV_SEG(seg);
		seg->seq_ = seq;
		seg->data_size_ = data_size;
		memcpy(seg->data_, data, data_size);

		recv_data_.push_back(seg);
		first_seq_ = seq;

		check_recv_window();
		//报告可读
		net_channel_->on_read();

		ok_count_++;
	}
	else if(seq > first_seq_ + 1)
	{
		RecvWindowMap::iterator it = recv_window_.find(seq);
		if(it == recv_window_.end())
		{
			//将数据缓冲到队列中
			GAIN_RECV_SEG(seg);
			seg->seq_ = seq;
			seg->data_size_ = data_size;
			memcpy(seg->data_, data, data_size);

			recv_window_[seq] = seg;
		}

		ok_count_++;
	}

	if (max_seq_ < seq){
		if (max_seq_ > 0){
			for (uint64_t i = max_seq_ + 1; i < seq; i++)
				loss_map_[i] = i;
		}
		max_seq_ = seq;
	}

	if (ok_count_ >= 128){
		if (!check_loss())
			net_channel_->send_ack(first_seq_);

		ok_count_ = 0;
	}

	return 0;
}

void RUDPRecvBuffer::check_buffer()
{

}

void RUDPRecvBuffer::check_recv_window()
{
	if(recv_window_.empty())
		return ;
	//将所有连续的数据片放到数据读取队列
	RecvWindowMap::iterator it = recv_window_.begin();
	while(it != recv_window_.end() && it->first == first_seq_ + 1)
	{
		first_seq_ = it->first;
		bandwidth_ts_ += it->second->data_size_;
		recv_data_.push_back(it->second);

		recv_window_.erase(it ++);

		ok_count_++;
	}
}

bool RUDPRecvBuffer::check_loss()
{
	bool ret = false;

	LossIDArray ids;
	int count = 0;
	for (LossIDTSMap::iterator it = loss_map_.begin(); it != loss_map_.end(); it++)
	{
		if (it->first < first_seq_ + 2048){
			ids.push_back(static_cast<uint32_t>(it->first - first_seq_));
			ret = true;
			if (count++ > 150)
				break;
		}
		else
			break;
	}

	if(ret && net_channel_ != NULL)
		net_channel_->send_nack(first_seq_, ids);

	return ret;
} 

void RUDPRecvBuffer::on_timer(uint64_t now_timer, uint32_t rtc, uint32_t rtt)
{

	if (last_ack_ts_ + rtc + 10 < now_timer){
		if (ok_count_ > 0){
			if (check_loss())
				recv_new_packet_ = false;
			else
				net_channel_->send_ack(first_seq_);
		}
		ok_count_ = 0;
		set_send_last_ack_ts(now_timer);
	}
	//判断是否可以读
	if(!recv_data_.empty() && net_channel_ != NULL)
		net_channel_->on_read();
}

int32_t RUDPRecvBuffer::read(uint8_t* data, int32_t data_size)
{
	int32_t ret = 0;
	uint8_t* pos = data;

	if(recv_data_.empty())
		return ret;

	while(!recv_data_.empty() && ret < data_size)
	{
		RUDPRecvSegment* seg = recv_data_.front();
		//RUDP_RECV_DEBUG("pop data, seq = " << seg->seq_);

		int32_t remain = data_size - ret;
		if(remain < seg->data_size_) //不能完全拷贝下一个数据片
		{
			memcpy(pos, seg->data_, remain);
			//覆盖掉已经拷贝的数据
			memmove(seg->data_, seg->data_ + remain, seg->data_size_ - remain);
			seg->data_size_ = seg->data_size_ - remain;

			pos += remain;
			ret += remain;
		}
		else //足够拷贝下一个数据片
		{
			memcpy(pos, seg->data_, seg->data_size_);
			pos += seg->data_size_;
			ret += seg->data_size_;

			RETURN_RECV_SEG(seg);

			recv_data_.pop_front();
		}
	}

	return ret;
}

uint32_t RUDPRecvBuffer::get_bandwidth()
{
	uint32_t ret = 0;

	uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
	if(cur_ts > bandwidth_ts_)
		ret = static_cast<uint32_t>(bandwidth_ * 1000 / (cur_ts - bandwidth_ts_));
	else
		ret = bandwidth_ * 1000;

	bandwidth_ts_ = cur_ts;
	bandwidth_ = 0;

	return ret;
}

int32_t RUDPRecvBuffer::get_buffer_data_size() 
{
	int32_t ret = 0;

	for(RecvDataList::iterator list_it = recv_data_.begin(); list_it != recv_data_.end(); ++ list_it)
		ret += (*list_it)->data_size_;

	for(RecvWindowMap::iterator map_it = recv_window_.begin(); map_it != recv_window_.end(); ++ map_it)
		ret += map_it->second->data_size_;

	return ret;
}

BASE_NAMESPACE_END_DECL

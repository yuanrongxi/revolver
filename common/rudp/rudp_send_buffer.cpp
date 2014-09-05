#include "rudp/rudp_send_buffer.h"
#include "revolver/base_timer_value.h"
#include "rudp/rudp_log_macro.h"
#include "rudp/rudp_ccc.h"

#include <math.h>

BASE_NAMESPACE_BEGIN_DECL

#define DEFAULT_RUDP_SEND_BUFFSIZE	4096
#define NAGLE_DELAY					100

RUDPSendBuffer::RUDPSendBuffer() 
: net_channel_(NULL)
, buffer_seq_(0)
, dest_max_seq_(0)
, cwnd_max_seq_(0)
, buffer_size_(DEFAULT_RUDP_SEND_BUFFSIZE)
, buffer_data_size_(0)
, nagle_(false)
, ccc_(NULL)
{
	reset();
}

RUDPSendBuffer::~RUDPSendBuffer()
{
	reset();
}

void RUDPSendBuffer::reset()
{
	buffer_seq_ = rand() + 1;
	dest_max_seq_ = buffer_seq_ - 1;
	cwnd_max_seq_ = buffer_seq_;
	buffer_data_size_ = 0;
	buffer_size_ = DEFAULT_RUDP_SEND_BUFFSIZE;
	nagle_ = false;

	loss_set_.clear();

	for(SendWindowMap::iterator it = send_window_.begin(); it != send_window_.end(); ++ it)
		RETURN_SEND_SEG(it->second);

	send_window_.clear();

	for(SendDataList::iterator it = send_data_.begin(); it != send_data_.end(); ++ it)
		RETURN_SEND_SEG(*it);

	send_data_.clear();

	bandwidth_ = 0;
	bandwidth_ts_ = CBaseTimeValue::get_time_value().msec();
}

int32_t RUDPSendBuffer::send(const uint8_t* data, int32_t data_size)
{
	int32_t copy_pos = 0;
	int32_t copy_size = 0;
	uint8_t* pos = (uint8_t *)data;
	uint64_t now_timer = CBaseTimeValue::get_time_value().msec();

	if(!send_data_.empty()) //粘包
	{
		RUDPSendSegment* last_seg = send_data_.back();
		if(last_seg != NULL && last_seg->data_size_ < MAX_SEGMENT_SIZE)
		{
			copy_size = MAX_SEGMENT_SIZE - last_seg->data_size_;
			if( copy_size > data_size) 
				copy_size = data_size;

			memcpy(last_seg->data_ + last_seg->data_size_, pos, copy_size);

			copy_pos += copy_size;
			pos += copy_size;
			last_seg->data_size_ += copy_size;
		}
	}

	//分片
	while(copy_pos < data_size)
	{
		GAIN_SEND_SEG(last_seg);

		//设置初始化的的时刻
		last_seg->push_ts_ = now_timer;
		last_seg->seq_ = buffer_seq_;
		buffer_seq_ ++;

		//RUDP_SEND_DEBUG("put packet, seq = " << buffer_seq_);

		//确定拷贝的块长度
		copy_size = (data_size - copy_pos);
		if(copy_size > MAX_SEGMENT_SIZE)
			copy_size = MAX_SEGMENT_SIZE;

		memcpy(last_seg->data_, pos, copy_size);

		copy_pos += copy_size;
		pos += copy_size;
		last_seg->data_size_ = copy_size;

		send_data_.push_back(last_seg);
	}

	buffer_data_size_ += copy_pos;
	
	//尝试发送,立即发送
	//if(!nagle_)
	attempt_send(now_timer);
	 
	return copy_pos;
}

void RUDPSendBuffer::on_ack(uint64_t seq)
{
	//RUDP_SEND_DEBUG("on ack, seq = " << seq);
	//ID错误
	if(cwnd_max_seq_ < seq)
		return ;

	if(!send_window_.empty())
	{
		//删除窗口的数据片
		SendWindowMap::iterator it = send_window_.begin();
		while(it != send_window_.end() && it->first <= seq)
		{
			//删除丢包信息
			if(!loss_set_.empty())
				loss_set_.erase(it->first);

			//更新数据缓冲的大小
			if(buffer_data_size_ >  it->second->data_size_)
				buffer_data_size_ = buffer_data_size_ - it->second->data_size_;
			else
				buffer_data_size_ = 0;

			bandwidth_ += it->second->data_size_;

			RETURN_SEND_SEG(it->second);
			send_window_.erase(it ++);
		}

		if(dest_max_seq_ < seq)
			dest_max_seq_ = seq;
	}

	//尝试发送
	attempt_send(CBaseTimeValue::get_time_value().msec());

	check_buffer();
}

void RUDPSendBuffer::on_nack(uint64_t base_seq, const LossIDArray& loss_ids)
{
	//增加丢包信息
	for(size_t i = 0; i < loss_ids.size(); ++i)
	{
		if(loss_ids[i] + base_seq < cwnd_max_seq_)
		{
			loss_set_.insert(loss_ids[i] + base_seq);
		}
	}

	on_ack(base_seq);
}

void RUDPSendBuffer::on_timer(uint64_t now_timer)
{
	attempt_send(now_timer);
	check_buffer();
}

void RUDPSendBuffer::check_buffer()
{
	//检查是否可以写
	if(buffer_data_size_ < buffer_size_ && net_channel_ != NULL)
	{
		net_channel_->on_write();
	}
}

//CCC控制清除接口
void RUDPSendBuffer::clear_loss()
{
	loss_set_.clear();
}

void RUDPSendBuffer::attempt_send(uint64_t now_timer)
{
	uint32_t cwnd_size = send_window_.size();
	uint32_t rtt = ccc_->get_rtt();
	uint32_t ccc_cwnd_size = ccc_->get_send_window_size();
	RUDPSendSegment* seg = NULL;
	 
	uint32_t send_packet_number  = 0;
	if(!loss_set_.empty()) //重发丢失的片段
	{
		//发送丢包队列中的报文
		uint64_t loss_last_ts = 0;
		uint64_t loss_last_seq = 0;
		for(LossIDSet::iterator it = loss_set_.begin(); it != loss_set_.end();)
		{
			if(send_packet_number >= ccc_cwnd_size)
				break;

			SendWindowMap::iterator cwnd_it = send_window_.find(*it);
			if(cwnd_it != send_window_.end() && cwnd_it->second->last_send_ts_ + rtt < now_timer)
			{
				seg = cwnd_it->second;

				net_channel_->send_data(0, seg->seq_, seg->data_, seg->data_size_, now_timer);
				if(cwnd_max_seq_ < seg->seq_)
					cwnd_max_seq_ = seg->seq_;

				//判断是否可以更改TS
				if(loss_last_ts < seg->last_send_ts_)
				{
					loss_last_ts = seg->last_send_ts_;
					if(loss_last_seq < *it) 
						loss_last_seq = *it;
				}

				seg->last_send_ts_ = now_timer;
				seg->send_count_ ++;

				send_packet_number ++;

				loss_set_.erase(it ++);

				ccc_->add_resend();
				//RUDP_SEND_DEBUG("resend seq = " << seg->seq_);
			}
			else
				++ it;
		}
		//更新重发包范围内未重发报文的时刻，防止下一次定时器到来时重复发送
		for(SendWindowMap::iterator it = send_window_.begin(); it != send_window_.end(); ++it)
		{
			if(it->second->push_ts_ < loss_last_ts && loss_last_seq >= it->first)
				it->second->last_send_ts_ = now_timer;
			else if(loss_last_seq < it->first)
				break;
		}
	}
	else if(send_window_.size() > 0)//丢包队列为空，重发所有窗口中超时的分片
	{
		uint32_t rtt_threshold = (uint32_t)ceil(rtt * 1.25);
		rtt_threshold = (core_max(rtt_threshold, 30));

		SendWindowMap::iterator end_it = send_window_.end();
		for(SendWindowMap::iterator it = send_window_.begin(); it != end_it; ++it)
		{
			if(send_packet_number >= ccc_cwnd_size || (it->second->push_ts_ + rtt_threshold > now_timer))
				break;

			seg = it->second;

			if(seg->last_send_ts_ + rtt_threshold < now_timer 
				|| (seg->push_ts_ + rtt_threshold * 5 < now_timer && seg->seq_ < dest_max_seq_ + 3 && seg->last_send_ts_ + rtt_threshold / 2 < now_timer))
			{
				net_channel_->send_data(0, seg->seq_, seg->data_, seg->data_size_, now_timer);

				if(cwnd_max_seq_ < seg->seq_)
					cwnd_max_seq_ = seg->seq_;

				seg->last_send_ts_ = now_timer;
				seg->send_count_ ++;

				send_packet_number ++;

				ccc_->add_resend();

				//RUDP_SEND_DEBUG("resend seq = " << seg->seq_);
			}
		}
	}

	//判断是否可以发送新的报文
	if(ccc_cwnd_size > send_packet_number)
	{
		while(!send_data_.empty())
		{
			RUDPSendSegment* seg = send_data_.front();
			//判断NAGLE算法,NAGLE最少需要在100MS凑1024个字节报文
			if(cwnd_size > 0 && nagle_ && seg->push_ts_ + NAGLE_DELAY > now_timer && seg->data_size_ < MAX_SEGMENT_SIZE - 256)
				break;

			//判断发送窗口
			if(cwnd_size < ccc_cwnd_size)
			{
				send_data_.pop_front();
				send_window_.insert(SendWindowMap::value_type(seg->seq_, seg));
				//send_window_[seg->seq_] = seg;
				cwnd_size ++;

				seg->push_ts_ = now_timer;
				seg->last_send_ts_ = now_timer;
				seg->send_count_ = 1;

				net_channel_->send_data(0, seg->seq_, seg->data_, seg->data_size_, now_timer);
				if(cwnd_max_seq_ < seg->seq_)
					cwnd_max_seq_ = seg->seq_;

				//RUDP_SEND_DEBUG("send seq = " << seg->seq_);
			}
			else 
				break;
		}
	}
}

uint32_t RUDPSendBuffer::get_bandwidth()
{
	uint32_t ret = 0;

	uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
	if(cur_ts > bandwidth_ts_)
	{
		ret = static_cast<uint32_t>(bandwidth_ * 1000 / (cur_ts - bandwidth_ts_));
	}
	else
	{
		ret = bandwidth_ * 1000;
	}

	bandwidth_ts_ = cur_ts;
	bandwidth_ = 0;

	return ret;
}


BASE_NAMESPACE_END_DECL

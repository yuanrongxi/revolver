#include "revolver/base_reactor_instance.h"
#include "rudp/rudp_interface.h"
#include "rudp/rudp_adapter.h"
#include "rudp/rudp_packet.h"
#include "rudp/rudp_socket.h"
#include "rudp/rudp_log_macro.h"

#define RUDP_DEFAULT_POOL_SIZE	1024
#define MAX_LOCAL_ADDR_SIZE		256

//RUDP的心跳周期是5毫秒
#define RUDP_TIMER_DELAY		10

//消息解码宏
#define PARSE_RUDP_HEAD(head, info) \
		RUDPHeadPacket head;\
		try{\
			strm >> head;\
		}\
		catch(...){\
			return;\
		}

BASE_NAMESPACE_BEGIN_DECL

RUDPObject::RUDPObject()
{
	listener_ = NULL;
	timer_id_ = 0;

	init();
}

RUDPObject::~RUDPObject()
{
	//destroy();
	clear_timer_events();
}

void RUDPObject::set_timer()
{
	if(timer_id_ == 0)
	{
		timer_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, RUDP_TIMER_DELAY);
	}
}

void RUDPObject::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);

		timer_id_ = 0;
	}
}

int32_t RUDPObject::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id == timer_id_)
	{
		heartbeat();

		timer_id_ = REACTOR_INSTANCE()->set_timer(this, NULL, RUDP_TIMER_DELAY);
	}

	return 0;
}

void RUDPObject::heartbeat()
{
	//扫描所有的RUDP SOCKET对象
	uint32_t id = INVALID_RUDP_HANDLE;
	int32_t size = socket_array_.size();

	for(RUDPHandleSet::iterator it = used_socket_ids_.begin(); it != used_socket_ids_.end();)
	{
		id = *it;

		 ++ it;
		if(id > 0 && id < size && socket_array_[id] != NULL)
		{
			socket_array_[id]->heartbeat();
		}
	}
}

void RUDPObject::init()
{
	RUDP_INFO("init rudp core!");

	listener_ = NULL;
	free_socket_ids_.clear();
	used_socket_ids_.clear();
	peer_socket_ids_.clear();

	for(int32_t i = 0; i < MAX_LOCAL_ADDR_SIZE; i ++)
	{
		adapter_array_.push_back(NULL);
	}

	alloc_sockets();

	set_timer();
}

void RUDPObject::destroy()
{
	cancel_timer();

	int32_t size = socket_array_.size();
	for(int32_t i = 0; i < size; ++i)
	{
		if(socket_array_[i] != NULL)
		{
			socket_array_[i]->force_close();
		}
	}

	free_socket_ids_.clear();
	used_socket_ids_.clear();
	peer_socket_ids_.clear();

	RUDP_INFO("destroy rudp core!");
}

void RUDPObject::alloc_sockets()
{
	int32_t array_size = socket_array_.size();
	socket_array_.resize(array_size + RUDP_DEFAULT_POOL_SIZE);
	for(int32_t i = array_size; i < array_size + RUDP_DEFAULT_POOL_SIZE; ++ i)
	{
		if(i != 0)
			free_socket_ids_.insert(i);

		socket_array_[i] = NULL;
	}

	RUDP_WARNING("alloc " << RUDP_DEFAULT_POOL_SIZE << " rudp socket to pool!");
}

void RUDPObject::free_sockets(int32_t rudp_id)
{
	if(rudp_id > 0 && rudp_id < socket_array_.size() && socket_array_[rudp_id] != NULL)
	{
		RUDPPeerInfo info(socket_array_[rudp_id]->get_remote_rudp_id(), socket_array_[rudp_id]->get_peer_addr());
		peer_socket_ids_.erase(info);

		socket_pool_.push_obj(socket_array_[rudp_id]);
		socket_array_[rudp_id]->reset();
		socket_array_[rudp_id] = NULL;

		free_socket_ids_.insert(rudp_id);
		used_socket_ids_.erase(rudp_id);
	}

	RUDP_INFO("free rudp socket, socket id = " << rudp_id);
}

void RUDPObject::delete_peer_index(int32_t peer_id, const Inet_Addr& addr)
{
	RUDPPeerInfo info(peer_id, addr);
	peer_socket_ids_.erase(info);
}

RUDPSocket* RUDPObject::get_socket(int32_t rudp_id)
{
	RUDPSocket* ret = NULL;
	if(rudp_id > 0 && rudp_id < socket_array_.size())
	{
		ret = socket_array_[rudp_id];
	}

	return ret;
}

int32_t RUDPObject::create_socket()
{
	int32_t ret = INVALID_RUDP_HANDLE;

	//句柄不够
	if(free_socket_ids_.empty())
		alloc_sockets();

	RUDPHandleSet::iterator it = free_socket_ids_.begin();
	ret = *it;
	if(ret < socket_array_.size() && socket_array_[ret] == NULL)
	{
		RUDPSocket* rudp_session = socket_pool_.pop_obj();
		socket_array_[ret] = rudp_session;

		free_socket_ids_.erase(it);
		used_socket_ids_.insert(ret);
		//关联RUDP SOCKET句柄
		rudp_session->open(ret);

		RUDP_INFO("create rudp socket, socket id = " << ret);
	}
	else
	{
		ret = INVALID_RUDP_HANDLE;
		RUDP_INFO("create rdup failed");
	}

	return ret;
}

void RUDPObject::close_socket(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return;
	}
	
	rudp_session->close();
}

void RUDPObject::force_close_socket(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return;
	}

	rudp_session->force_close();
}

int32_t RUDPObject::setoption(int32_t rudp_id, int32_t op_type, int32_t op_value)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	return rudp_session->setoption(op_type, op_value);
}

int32_t RUDPObject::bind(int32_t rudp_id, const Inet_Addr& local_addr)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	for(uint8_t i = 0; i < adapter_array_.size(); ++i)
	{
		if(adapter_array_[i] != NULL && local_addr == adapter_array_[i]->get_local_addr())
		{
			if(rudp_session->bind(i, adapter_array_[i]->get_title()) != 0)//绑定本地的一个UDP发送接口
			{
				RUDP_FATAL("bind failed, rudp socket = " << rudp_id);
				return -1;
			}
			else
				return 0;
		}
	}

	return -1;
}

int32_t RUDPObject::connect(int32_t rudp_id, const Inet_Addr& remote_addr)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	return rudp_session->connect(remote_addr);
}

int32_t RUDPObject::send(int32_t rudp_id, const uint8_t* data, int32_t data_size)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	return rudp_session->send(data, data_size);
}

int32_t RUDPObject::recv(int32_t rudp_id, uint8_t* data, int32_t data_size)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	return rudp_session->recv(data, data_size);
}

int32_t	RUDPObject::get_error(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	return rudp_session->get_error();
}

int32_t RUDPObject::get_local_addr(int32_t rudp_id, Inet_Addr& addr)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	uint8_t index = rudp_session->get_local_index();
	if(adapter_array_[index] != NULL)
	{
		addr = adapter_array_[index]->get_local_addr();
		return 0;
	}
	else
		return -1;
}

int32_t RUDPObject::get_peer_addr(int32_t rudp_id, Inet_Addr& addr)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return -1;
	}

	addr = rudp_session->get_peer_addr();
	return 0;
}

uint32_t RUDPObject::recv_bandwidth(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return 0;
	}

	return rudp_session->recv_bandwidth();
}

uint32_t RUDPObject::send_bandwidth(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return 0;
	}

	return rudp_session->send_bandwidth();
}

int32_t RUDPObject::send_cache_size(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return 0;
	}

	return rudp_session->get_send_cache_size();
}

int32_t RUDPObject::recv_cache_size(int32_t rudp_id)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return 0;
	}

	return rudp_session->get_recv_cache_size();
}

void RUDPObject::bind_event_handle(int32_t rudp_id, RUDPEventHandler* event_handler)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return ;
	}

	return rudp_session->set_event_handler(event_handler);
}

void RUDPObject::register_event(int32_t rudp_id, int32_t marks)
{
	RUDPSocket* rudp_session = get_socket(rudp_id);
	if(rudp_session == NULL)
	{
		RUDP_FATAL("rudp_session is NULL, rudp socket id = " << rudp_id);
		return ;
	}

	rudp_session->register_event(marks);
}

void RUDPObject::process(IRUDPAdapter* adapter, BinStream& strm, const Inet_Addr& remote_addr)
{
	uint8_t index = INVALID_ADAPTER_INDEX; 
	if(adapter != NULL)
		index = adapter->get_index();

	if(adapter_array_[index] == NULL)
	{
		RUDP_FATAL("adapter is not attach!!!");
		return ;
	}

	PARSE_RUDP_HEAD(rudp_head, "parse rudp head failed!!");

	RUDPSocket *rudp_session = get_socket(rudp_head.remote_rudp_id_);
	if(rudp_session != NULL)
	{
		rudp_session->process(rudp_head.msg_id_, rudp_head.check_sum_, strm, remote_addr);
		return ;
	}

	//处理SYN连接请求
	if(rudp_head.msg_id_ == RUDP_SYN && rudp_head.remote_rudp_id_ == INVALID_RUDP_HANDLE)
	{
		PARSE_RUDP_MESSAGE(strm, RUDPSynPacket, syn, "parse syn failed!");

		//在半开连接中找
		rudp_session = find_by_peer_id(syn.local_rudp_id_, remote_addr);
		if(rudp_session == NULL)
		{
			int32_t new_rudp_id = create_socket();
			if(new_rudp_id == INVALID_RUDP_HANDLE)
			{
				send_syn2(rudp_head.remote_rudp_id_, SYSTEM_SYN_ERROR, rudp_head.check_sum_, remote_addr, adapter);
				RUDP_INFO("send syn2 to " << remote_addr);
				return ;
			}

			if(this->bind(new_rudp_id, adapter->get_local_addr()) != 0)
			{
				free_sockets(new_rudp_id);

				send_syn2(rudp_head.remote_rudp_id_, SYSTEM_SYN_ERROR, rudp_head.check_sum_, remote_addr, adapter);
				RUDP_INFO("send syn2 to " << remote_addr);
				return ;
			}

			//建立远程SOCKET ID与本地SOCKET ID的关系，防止重复建立连接
			RUDPPeerInfo info(syn.local_rudp_id_, remote_addr);
			peer_socket_ids_[info] = new_rudp_id;

			rudp_session = get_socket(new_rudp_id);
			rudp_session->set_check_sum(rudp_head.check_sum_);

			//提示上层EVENT HANDLE进行ACCEPT，可以在上层进行SOCKET属性设置
			if(listener_ != NULL)
				listener_->rudp_accept_event(new_rudp_id);
		}

		//进行消息处理
		rudp_session->process_syn(syn, remote_addr);
	}
	else
	{
		RUDP_WARNING("error msg, msg id =  " << (uint16_t)rudp_head.msg_id_ << ", rudp id = " << rudp_head.remote_rudp_id_ << ", remote addr = "<< remote_addr);
	}
}

RUDPSocket* RUDPObject::find_by_peer_id(int32_t peer_rudp_id, const Inet_Addr& peer_addr)
{
	RUDPSocket* ret = NULL;

	RUDPPeerInfo info(peer_rudp_id, peer_addr);
	RUDPHandleMap::iterator it = peer_socket_ids_.find(info);
	if(it != peer_socket_ids_.end())
	{
		ret = get_socket(it->second);
	}

	return ret;
}

void RUDPObject::send_udp(uint8_t index, BinStream& strm, const Inet_Addr& remote_addr)
{
	if(adapter_array_[index] != NULL)
		adapter_array_[index]->send(strm, remote_addr);
}

void RUDPObject::attach(IRUDPAdapter* adapter)
{
	if(adapter == NULL)
	{
		RUDP_WARNING("adapter is NULL");
		return ;
	}

	for(uint8_t i = 0; i < adapter_array_.size(); i ++) //关联本地的网络适配器
	{
		if(adapter_array_[i] == adapter || i >= INVALID_ADAPTER_INDEX) //重复ATTACH
		{
			RUDP_WARNING("repeat attach adapter!!");
			break;
		}

		//绑定
		if(adapter_array_[i] == NULL)
		{
			adapter->set_index(i);
			adapter_array_[i] = adapter;

			RUDP_INFO("attach adapter, id = " << (uint16_t)i << ", local_addr = " << adapter->get_local_addr());
			break;
		}
	}
}

void RUDPObject::unattach(IRUDPAdapter* adapter)
{
	if(adapter != NULL && adapter->get_index() < INVALID_ADAPTER_INDEX)
	{
		adapter_array_[adapter->get_index()] = NULL;
	}
}

//发送一个握手失败的SYN2
void RUDPObject::send_syn2(uint32_t remote_rudp_id, uint8_t result, uint16_t check_sum, const Inet_Addr& remote_addr, IRUDPAdapter* adapter)
{
	if(adapter == NULL)
		return ;

	RUDPHeadPacket head;
	head.msg_id_ = RUDP_SYN2;
	head.remote_rudp_id_ = remote_rudp_id;
	head.check_sum_ = check_sum;

	RUDPSyn2Packet syn2;
	syn2.version_ = RUDP_VERSION;
	syn2.local_rudp_id_ = 0;
	syn2.local_ts_ = CBaseTimeValue::get_time_value().msec();
	syn2.remote_ts_ = syn2.local_ts_;
	syn2.syn2_result_ = result;
	syn2.max_segment_size_ = MAX_SEGMENT_SIZE;
	syn2.start_seq_ = 0;

	BinStream strm;
	strm.rewind(true);
	strm << adapter->get_title() << head << syn2;

	adapter->send(strm, remote_addr);
}

BASE_NAMESPACE_END_DECL


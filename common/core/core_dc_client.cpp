#include "revolver/base_reactor_instance.h"
#include "core/core_connection_manager.h"
#include "core/core_packet.h"
#include "core/core_dc_client.h"
#include "core/core_local_info.h"
#include "revolver/lzo_object.h"
#include "revolver/base64.h"

#define DCENTERDELAY	40000
#define PHP_DELAY		30

#define ERASE_REQUEST(x) \
	if(x->second != NULL)\
		{\
			delete x->second;\
			x->second = NULL;\
		}\
		timer_ids_.erase(x)\


BASE_NAMESPACE_BEGIN_DECL

CCoreDCClient::CCoreDCClient()
{

}

CCoreDCClient::~CCoreDCClient()
{
	clear_timer_events();
}

void CCoreDCClient::release_timer_act(const void* act)
{
	CoreDCParam *param = (CoreDCParam *)act;
	if(param != NULL)
	{
		if(param->conn_ != NULL)
		{
			param->conn_->close();
			param->conn_ = NULL;
		}

		if(!param->release_flag_ && param->event_ != NULL)
			param->event_->release_param(param);
		else
			delete param;
	}
}

void CCoreDCClient::clear()
{
	DCTimerSet::iterator it = timer_ids_.begin();
	while(it != timer_ids_.end())
	{
		uint32_t timer_id = it->first;

		ERASE_REQUEST(it);

		CoreDCParam *param = cancel_dc_request(timer_id); //内部进行释放
		if(param != NULL)
		{
			if(param->conn_ != NULL)
			{
				param->conn_->close();
				param->conn_ = NULL;
			}

			if(!param->release_flag_ && param->event_ != NULL)
				param->event_->release_param(param);
			else
				delete param;
		}

		it = timer_ids_.begin();
	}
}

uint32_t CCoreDCClient::set_timer(CoreDCParam* param, PHPExcRequst* req, uint32_t delay)
{
	uint32_t timer_id = REACTOR_INSTANCE()->set_timer(this, param, delay);
	DCTimerSet::iterator it = timer_ids_.find(timer_id);
	if(it == timer_ids_.end())
	{
		timer_ids_[timer_id] = req;
	}
	else
	{
		if(it->second != NULL)
		{
			delete it->second;
		}

		it->second = req;
	}

	return timer_id;
}

CoreDCParam* CCoreDCClient::cancel_timer(uint32_t timer_id)
{
	CoreDCParam* param = NULL;

	DCTimerSet::iterator it = timer_ids_.find(timer_id);
	if(it != timer_ids_.end())
	{
		ERASE_REQUEST(it);

		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id, &act);

		param = (CoreDCParam *)act;
		if(param->conn_ != NULL) //关闭连接
		{
			param->conn_->set_conn_id(0);
			param->conn_->close();
			param->conn_ = NULL;
		}
	}

	if(param != NULL && param->release_flag_)
	{
		delete param;
		return NULL;
	}
	else
		return param;
}

int32_t CCoreDCClient::handle_timeout(const void *act, uint32_t timer_id)
{
	DCTimerSet::iterator it = timer_ids_.find(timer_id);
	if(it != timer_ids_.end())
	{
		uint8_t ack = 0;
		if(it->second != NULL)
			ack = it->second->exc_ret;

		ERASE_REQUEST(it);

		CoreDCParam* param = (CoreDCParam*)act;
		if(param != NULL)
		{
			if(param->conn_ != NULL) //超时
			{
				param->conn_->set_conn_id(0);
				param->conn_->close();
				param->conn_ = NULL;
			}

			//判断超时事件，如果不需要返回，直接释放参数
			if(!param->release_flag_ && param->event_ != NULL)
			{
				if(ack == 1)
					param->event_->dc_timeout(param, timer_id);
				else
					param->event_->release_param(param);
			}
			else
				delete param;
		}
	}

	return 0;
}

uint32_t CCoreDCClient::post_dc_request(uint32_t dc_sid, CoreDCParam* param, const string& php, const string& php_param, 
								   bool read_flag, bool ack /* = true */)
{
	PHPExcRequst *body = new PHPExcRequst;
	if(param == NULL)
	{
		param = new CoreDCParam();
		param->release_flag_ = true;
		param->event_ = NULL;
	}
	else
		param->release_flag_ = false;

	uint32_t exc_id = set_timer(param, body, read_flag ? DCENTERDELAY : 15000);

	body->exc_id = exc_id;
	body->exc_ret = ack ? 1 : 0;
	body->php_file = php;
	body->php_param = Base64::encode(php_param);
	body->time_delay = PHP_DELAY;
	body->exc_type = read_flag ? 1 : 2;
	
	Server_Node_t server_node = CONN_MANAGER()->find_server_info(dc_sid);
	if(server_node.server_id != 0)
	{
		ShortConnection* conn = SHORTCONN_POOL.pop_obj();
		if(conn != NULL)
		{
			Inet_Addr src_addr, dst_addr;
			CONN_MANAGER()->get_address_pair(server_node, src_addr, dst_addr);

			src_addr.set_port(0);
			//随机一个端口作为客户机端口
			if(src_addr.get_ip() != INADDR_ANY)
			{
				src_addr.set_port(30000 + rand() % 20000);
			}

			conn->set_proc(this);
			if(conn->connect(src_addr, dst_addr) != 0)
			{
				cancel_timer(exc_id);

				conn->reset();
				SHORTCONN_POOL.push_obj(conn);

				return 0;
			}
			else
			{
				conn->set_conn_id(exc_id);
				if(param != NULL)
				{
					param->conn_ = conn;
				}
			}
		}
	}

	return exc_id;
}

CoreDCParam* CCoreDCClient::cancel_dc_request(uint32_t exc_id)
{
	return cancel_timer(exc_id);
}

int32_t CCoreDCClient::on_php_response(CBasePacket* packet, uint32_t sid)
{
	PHPExcResponse* res = (PHPExcResponse *)packet;

	//判断解压
	if(res->zip == 1)
	{
		GAIN_BINSTREAM(strm);
		strm->resize(res->src_size);

		uint32_t uncomp_size = 0;
		if(LZO()->uncompress((uint8_t *)res->result.data(), res->result.size(), strm->get_wptr(), uncomp_size) == 0 && uncomp_size > 0)
		{
			strm->set_used_size(uncomp_size);
			strm->bin_to_string(res->result);
		}

		RETURN_BINSTREAM(strm);
	}

	DCTimerSet::iterator it = timer_ids_.find(res->exc_id);
	if(it != timer_ids_.end())
	{
		CoreDCParam* param = cancel_dc_request(res->exc_id);
		if(param != NULL)
		{
			//回执一个结果！！！
			if(param->event_ != NULL && !param->release_flag_)
				param->event_->dc_response(param, res->result, res->exc_id);
			else
				delete param;
		}
	}

	return 0;
}

int32_t CCoreDCClient::on_short_connected(ShortConnection *conn)
{
	uint32_t exc_id = conn->get_conn_id();
	if(exc_id > 0)
	{
		DCTimerSet::iterator it = timer_ids_.find(exc_id);
		if(it != timer_ids_.end() && it->second != NULL)
		{
			INIT_CORE_REQUEST(packet, PHP_EXC_REQUEST);
			packet.set_body(*(it->second));

			conn->send(packet);
		}
	}

	return 0;
}

//不做任何处理，如果是请求中，等待其超时，主动关闭timer_ids_中无conn id!!!
int32_t CCoreDCClient::on_short_disconnected(ShortConnection *conn)
{
	uint32_t exc_id = conn->get_conn_id();
	if(exc_id > 0)
	{
		conn->set_conn_id(0);

		DCTimerSet::iterator it = timer_ids_.find(exc_id);
		if(it != timer_ids_.end())
		{
			uint8_t ack = 0;
			if(it->second != NULL)
				ack = it->second->exc_ret;

			ERASE_REQUEST(it);

			//只通知上层超时，不做连接关闭
			const void* act = NULL;
			REACTOR_INSTANCE()->cancel_timer(exc_id, &act);

			CoreDCParam* param = (CoreDCParam*)act;
			if(param != NULL)
			{
				//判断超时事件，如果不需要返回，直接释放参数
				if(!param->release_flag_ && param->event_ != NULL)
				{
					if(ack == 1)
						param->event_->dc_request_exception(param, exc_id);
					else
						param->event_->release_param(param);
				}
				else
					delete param;
			}
		}
	}

	return 0;
}

int32_t CCoreDCClient::on_message(CCorePacket &packet, BinStream& istrm, ShortConnection* conn)
{
	if(packet.msg_id_ == PHP_EXC_RESPONSE)
	{
		PHPExcResponse res;
		istrm >> res;
		
		//消息处理
		on_php_response(&res, 0);
	}

	return 0;
}

BASE_NAMESPACE_END_DECL


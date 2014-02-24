BASE_NAMESPACE_BEGIN_DECL

#define 	FIRST_ROUND   16777216
#define		SECOND_ROUND  65536
#define		THIRD_ROUND	  256

template <class HANDLER> 
BaseTimerNode_T<HANDLER>::BaseTimerNode_T(void) 
: act_(NULL), timer_id_(0xffffffff)
, timeout_stamp_(0), internal_(0), handler_(0)
{

}

template <class HANDLER> 
BaseTimerNode_T<HANDLER>::~BaseTimerNode_T(void)
{

}

template <class HANDLER> 
void BaseTimerNode_T<HANDLER>::set(HANDLER handler, const void* act,
									   uint32_t timeout_stamp, uint32_t intval, uint32_t timer_id)
{
	handler_ = handler;
	act_ = act;
	timeout_stamp_ = timeout_stamp;
	internal_ = intval;
	timer_id_ = timer_id;
}

template <class HANDLER> 
void BaseTimerNode_T<HANDLER>::get_dispatch_info(BaseTimerDispathInfo_T<HANDLER> &info)
{
	info.act_ = act_;
	info.handler_ = handler_;
	info.recurring_ = internal_ > 0 ? true : false;
}

template <class HANDLER> 
void BaseTimerNode_T<HANDLER>::get_revolver_pos(uint8_t& first, uint8_t &second, uint8_t& third, uint8_t& fourth) const
{
	first = (uint8_t)(timeout_stamp_ / FIRST_ROUND);
	second = (uint8_t)((timeout_stamp_ % FIRST_ROUND) / SECOND_ROUND);
	third =  (uint8_t)((timeout_stamp_ % SECOND_ROUND) / THIRD_ROUND);
	fourth = (uint8_t) (timeout_stamp_ % THIRD_ROUND);
}
BASE_NAMESPACE_END_DECL

#ifndef CORE_MESSAGE_TEMPLATE_H
#define CORE_MESSAGE_TEMPLATE_H

#include "revolver/base_typedef.h"
#include "revolver/base_inet_addr.h"

BASE_NAMESPACE_BEGIN_DECL

class BaseMsg
{
public:
	BaseMsg(){};
	virtual ~BaseMsg(){};

public:
	uint32_t msg_type;
};

template <class T, uint32_t MSGTYPE>
class TypedBaseMsg : public BaseMsg 
{
public:
	TypedBaseMsg() { msg_type = MSGTYPE;}
	virtual ~TypedBaseMsg() {};
	const T& data() const { return data_; }
	T& data() { return data_; }
private:
	T data_;
};

BASE_NAMESPACE_END_DECL

#endif

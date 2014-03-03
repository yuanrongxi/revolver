#include "base_timer_value.h"

#ifdef WIN32
#include "gettimeofday.h"
#include <stdio.h>
#endif

BASE_NAMESPACE_BEGIN_DECL

CBaseTimeValue::CBaseTimeValue()
{
	this->set(0, 0);
}

CBaseTimeValue::CBaseTimeValue(time_t s, time_t us)
{
	this->set(s, us);
}


CBaseTimeValue::CBaseTimeValue(const struct timeval& tv)
{
	this->set(tv);
}

CBaseTimeValue::~CBaseTimeValue()
{
}

void CBaseTimeValue::set(time_t s, time_t us)
{
	tv_.tv_sec = static_cast<long>(s);
	tv_.tv_usec = static_cast<long>(us);
}

void CBaseTimeValue::set(const struct timeval &tv)
{
	tv_ = tv;
}

uint64_t CBaseTimeValue::msec() const
{
	uint64_t ms = uint64_t(this->tv_.tv_sec);
	ms *= 1000;
	ms += (this->tv_.tv_usec / 1000);

	return ms;
}

uint64_t CBaseTimeValue::sec() const 
{
	return tv_.tv_sec;
}

uint32_t CBaseTimeValue::usec() const
{
	return tv_.tv_usec;
}

void CBaseTimeValue::set_msec(uint64_t ms)
{
	uint64_t secs = ms / 1000;
	this->tv_.tv_sec = static_cast<long> (secs);
	this->tv_.tv_usec = static_cast<long>((ms % 1000) * 1000);
}

void CBaseTimeValue::set_sec(uint64_t s)
{
	this->tv_.tv_sec = static_cast<long>(s);
	this->tv_.tv_usec = 0;
}

uint64_t CBaseTimeValue::to_usec() const
{
	uint64_t ret = (uint64_t)(tv_.tv_sec) * ONE_SECOND_IN_USECS;
	ret = ret + tv_.tv_usec;

	return ret;
}

void CBaseTimeValue::get(struct timeval& tv) const
{
	tv = tv_;
}

CBaseTimeValue CBaseTimeValue::get_time_value()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv;
}

CBaseTimeValue& CBaseTimeValue::operator +=(const CBaseTimeValue &time_value)
{
	uint64_t s = this->sec() + time_value.sec();
	uint64_t us = this->usec() + this->usec();

	s += us / ONE_SECOND_IN_USECS;
	us = us % ONE_SECOND_IN_USECS;
	set(s, us);

	return *this;
}

CBaseTimeValue& CBaseTimeValue::operator -=(const CBaseTimeValue& time_value)
{
	if(this->to_usec() < time_value.to_usec())
	{
		tv_.tv_sec = 0;
		tv_.tv_usec = 0;
	}
	else
	{
		uint64_t us = this->to_usec();
		uint64_t us2 = time_value.to_usec();
		us = us - us2;

		tv_.tv_sec = static_cast<long>(us / ONE_SECOND_IN_USECS);
		tv_.tv_usec = static_cast<long>(us % ONE_SECOND_IN_USECS);
	}
	return *this;
}

CBaseTimeValue& CBaseTimeValue::operator =(const CBaseTimeValue& time_value)
{
	this->tv_ = time_value.tv_;

	return *this;
}

CBaseTimeValue& CBaseTimeValue::operator =(const struct timeval& tv)
{
	this->tv_ = tv;

	return *this;
}

CBaseTimeValue& CBaseTimeValue::operator ++()
{
	uint64_t us = this->to_usec();
	us ++;
	tv_.tv_sec = static_cast<long>(us / ONE_SECOND_IN_USECS);
	tv_.tv_usec = static_cast<long>(us % ONE_SECOND_IN_USECS);

	return *this;
}

CBaseTimeValue& CBaseTimeValue::operator --()
{
	uint64_t us = this->to_usec();
	if(us > 0)
		us --;

	tv_.tv_sec = static_cast<long>(us / ONE_SECOND_IN_USECS);
	tv_.tv_usec = static_cast<long>(us % ONE_SECOND_IN_USECS);

	return *this;
}

bool operator==(const CBaseTimeValue& time_value1, const CBaseTimeValue& time_value2)
{
	if(time_value1.tv_.tv_sec == time_value2.tv_.tv_sec
		&& time_value1.tv_.tv_usec == time_value2.tv_.tv_usec)
		return true;
	else
		return false;
}

bool operator!=(const CBaseTimeValue& time_value1, const CBaseTimeValue time_value2)
{
	if(time_value1 == time_value2)
		return false;
	else
		 return true;
}

bool operator<(const CBaseTimeValue& t1, const CBaseTimeValue& t2)
{
	if(t1.to_usec() < t2.to_usec())
		return true;
	else 
		return false;
}

bool operator<=(const CBaseTimeValue& t1, const CBaseTimeValue& t2)
{
	if(t1.to_usec() <= t2.to_usec())
		return true;
	else
		return false;
}

bool operator>(const CBaseTimeValue& t1, const CBaseTimeValue& t2)
{
	if(t1.to_usec() > t2.to_usec())
		return true;
	else 
		return false;
}

bool operator>=(const CBaseTimeValue& t1, const CBaseTimeValue& t2)
{
	if(t1.to_usec() >= t2.to_usec())
		return true;
	else
		return false;
}
BASE_NAMESPACE_END_DECL


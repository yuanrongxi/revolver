/*
 * data_time.cpp
 *
 *  Created on: 2013-8-6
 *      Author: luochangjun
 */

#include "date_time.h"

BASE_NAMESPACE_BEGIN_DECL

DateTime::DateTime(const time_t seconds)
{
	this->seconds = seconds;
	this->date = *localtime(&this->seconds);
}
DateTime::DateTime(int year, int month, int day)
{
	tm t;
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = 0;
	t.tm_min = 0;
	t.tm_sec = 0;
	this->seconds = mktime(&t);
	this->date = *localtime(&this->seconds);

}
DateTime::DateTime(int year, int month, int day, int hour, int minute,
		int second)
{
	tm t;
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	this->seconds = mktime(&t);
	this->date = *localtime(&this->seconds);

}

DateTime::DateTime(std::string datetimeStr) //日期字符串格式 月/日/年 时:分:秒 例:02/03/2008 9:30:20 出错返回 01/01/1970 00:00:00
{
	this->InitByStr(datetimeStr, "%d/%d/%d %d:%d:%d");
}
DateTime::DateTime(std::string datetimeStr, std::string formaterStr)
{
	this->InitByStr(datetimeStr, formaterStr);
}

DateTime::~DateTime(void)
{

}

DateTime DateTime::Parse(std::string datetimeStr)
{
	DateTime datetime(datetimeStr);
	return datetime;
}
DateTime DateTime::Parse(const std::string datetimeStr,
		const std::string formaterStr)
{
	DateTime datetime(datetimeStr, formaterStr);
	return datetime;
}

void DateTime::InitByStr(std::string dateTimeStr, std::string formaterStr)
{
	int year, month, day, hour, minutes, seconds;
	sscanf(dateTimeStr.c_str(), formaterStr.c_str(), &year, &month, &day, &hour,
			&minutes, &seconds);
	if (year < 1900)
		year = 1970;
	if (month < 0)
		month = 1;
	if (day < 0)
		day = 1;
	if (hour < 0)
		hour = 0;
	if (minutes < 0)
		minutes = 0;
	if (seconds < 0)
		seconds = 0;
	tm t;
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minutes;
	t.tm_sec = seconds;
	this->seconds = mktime(&t);
	this->date = *localtime(&this->seconds);
}
int DateTime::DayOfWeek()
{
	return this->date.tm_wday;
}
int DateTime::DayOfYear()
{
	return this->date.tm_yday;
}
int DateTime::DaysInMonth(const int year, const int months)
{
	return 0;
}

bool DateTime::Equals(const Object *object)
{
	DateTime *dateTime = (DateTime *) object;
	if (this->seconds == dateTime->seconds)
		return true;
	return false;
}

bool DateTime::Equals(const DateTime *dateTime)
{
	return this->Equals((Object *) dateTime);
}

bool DateTime::Equals(const DateTime *value1, const DateTime *value2)
{
	if (value1->seconds == value2->seconds)
		return true;
	return false;
}
int DateTime::GetDay()
{
	return this->date.tm_mday;
}

int DateTime::GetHour()
{
	return this->date.tm_hour;
}
int DateTime::GetMinute()
{
	return this->date.tm_min;
}
int DateTime::GetMonth()
{
	return this->date.tm_mon;
}

DateTime DateTime::GetNow()
{
	DateTime datetime(time(0));
	return datetime;
}

int DateTime::GetSecond()
{
	return this->date.tm_sec;
}

int DateTime::GetYear()
{
	return this->date.tm_year + 1900;
}

void DateTime::AddYears(const time_t years)
{
	this->date.tm_year = this->date.tm_year + years;
	this->seconds = mktime(&this->date);
}
void DateTime::AddMonths(const time_t months)
{
	int a = (int) ((this->date.tm_mon + months) / 12);

	this->date.tm_year = this->date.tm_year + a;
	this->date.tm_mon = this->date.tm_mon
			+ (int) ((this->date.tm_mon + months) % 12) - 1;
	this->seconds = mktime(&this->date);
}

void DateTime::AddDays(const time_t days)
{
	this->AddHours(days * 24);
}
void DateTime::AddHours(const time_t hours)
{
	this->AddMinutes(hours * 60);
}
void DateTime::AddMinutes(const time_t minutes)
{
	this->AddSeconds(minutes * 60);
}

void DateTime::AddSeconds(const time_t seconds)
{
	this->seconds = this->seconds + seconds;
	this->date = *localtime(&this->seconds);
}
void DateTime::AddWeeks(const time_t weeks)
{
	this->AddDays(weeks * 7);
}

int DateTime::Compare(const DateTime *t1, const DateTime *t2)
{
	if (t1->seconds == t2->seconds)
		return 0;
	if (t1->seconds < t2->seconds)
		return -1;
	return 1;

}

int DateTime::CompareTo(const Object *value)
{
	DateTime * dateTime = (DateTime *) value;
	if (dateTime->seconds == this->seconds)
		return 0;
	if (this->seconds < dateTime->seconds)
		return -1;
	return 1;
}

int DateTime::CompareTo(const DateTime *value)
{
	if (this->seconds == value->seconds)
		return 0;
	if (this->seconds < value->seconds)
		return -1;
	return 1;
}

std::string DateTime::ToShortDateString()
{
	return this->ToString("%Y-%m-%d");
}

std::string DateTime::ToString()
{
	return this->ToString("%Y-%m-%d %H:%M:%S");
}
std::string DateTime::ToString(const std::string formaterStr)
{
	//strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", localtime(&t));
	char s[256];
	strftime(s, sizeof(s), formaterStr.c_str(), &this->date);
	std::string str(s);
	return str;

}

bool DateTime::operator ==(DateTime &dateTime)
{
	return this->Equals(&dateTime);

}
bool DateTime::operator !=(DateTime &datetime)
{
	if (this->seconds != datetime.seconds)
		return true;
	return false;
}
bool DateTime::operator >(DateTime &dateTime)
{
	if (this->seconds > dateTime.seconds)
		return true;
	return false;
}
bool DateTime::operator <(DateTime &datetime)
{
	if (this->seconds < datetime.seconds)
		return true;
	return false;
}
bool DateTime::operator >=(DateTime &datetime)
{
	if (this->seconds >= datetime.seconds)
		return true;
	return false;
}

bool DateTime::operator <=(DateTime &datetime)
{
	if (this->seconds <= datetime.seconds)
		return true;
	return false;
}

BASE_NAMESPACE_END_DECL

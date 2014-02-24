
#ifndef __DATA_TIME_H
#define __DATA_TIME_H

#include <string>
#include <stdio.h>
#include <time.h>
#include "base_typedef.h"
#include "base_namespace.h"

BASE_NAMESPACE_BEGIN_DECL

using namespace std;

class Object
{
public:
	Object(void){};
	virtual bool Equals(const Object *object) = 0;
	virtual int CompareTo(const Object *value) = 0;
	virtual std::string ToString() = 0;

public:
	~Object(void){};
};

class DateTime: public Object
{
public:
	DateTime(time_t seconds);
	DateTime(int year, int month, int day);
	DateTime(int year, int month, int day, int hour, int minute, int second);
	DateTime(std::string datetimeStr); //日期字符串格式 年/月/日 时:分:秒 例:2008/02/03 9:30:20 出错返回 01/01/1970 00:00:00
	DateTime(std::string datetimeStr, std::string formaterStr);
public:
	~DateTime(void);
public:
	void AddYears(const time_t years); //将指定的年份数加到此实例的值上。
	void AddMonths(const time_t Months); //将指定的月份数加到此实例的值上。
	void AddDays(const time_t days); //将指定的天数加到此实例的值上。
	void AddHours(const time_t hours); //将指定的小时数加到此实例的值上。
	void AddMinutes(const time_t minutes); //将指定的分钟数加到此实例的值上。
	void AddSeconds(const time_t seconds); //将指定的秒数加到此实例的值上。
	void AddWeeks(const time_t weeks); //将指定的周数加到些实上的值上。
	static int Compare(const DateTime *value1, const DateTime *value2); //对两个 DateTime 的实例进行比较，并返回一个指示第一个实例是早于、等于还是晚于第二个实例的整数。  返回值：小于零 value1 小于 value2。 零 value1 等于 value2。 大于零 value1 大于 value2。
	int CompareTo(const Object *value); //已重载。 将此实例的值与指定的 DateTime 值相比较，并指示此实例是早于、等于还是晚于指定的 DateTime 值。
	int CompareTo(const DateTime *value); //小于零 此实例小于 value。 零 此实例等于 value。 大于零 此实例大于 value。

	int DaysInMonth(const int year, const int months); //返回指定年和月中的天数。
	bool Equals(const Object *object);
	bool Equals(const DateTime *dateTime);
	static bool Equals(const DateTime *value1, const DateTime *value2);
	static DateTime Parse(std::string datetimeStr); //日期字符串格式 月/日/年 时:分:秒 例:02/03/2008 9:30:20 出错返回 01/01/1970 00:00:00
	static DateTime Parse(std::string dateTimeStr, std::string formaterStr);
	std::string ToShortDateString(); //将当前 DateTime 对象的值转换为其等效的短日期字符串表示形式。
	std::string ToString();
	std::string ToString(const std::string formaterStr); //formaterStr = "%Y-%m-%d %H:%M:%S" %Y=年 %m=月 %d=日 %H=时 %M=分 %S=秒
public:
	int GetYear(); //获取此实例所表示日期的年份部分。
	int GetMonth(); //获取此实例所表示日期的年份部分。
	int GetDay(); // 获取此实例所表示的日期为该月中的第几天。
	int GetHour(); //获取此实例所表示日期的小时部分。
	int GetMinute(); //获取此实例所表示日期的分钟部分
	int GetSecond(); //获取此实例所表示日期的秒部分。
	int DayOfWeek(); //获取此实例所表示的日期是星期几。
	int DayOfYear(); //记录今天是一年里面的第几天,从1月1日起,0-365

	int64_t	GetSeconds(){return seconds;}
	static DateTime GetNow(); //返回当前日期是间
public:
	bool operator ==(DateTime &dateTime);
	bool operator >(DateTime &dateTime);
	bool operator <(DateTime &DateTime);
	bool operator >=(DateTime &DateTime);
	bool operator <=(DateTime &DateTime);
	bool operator !=(DateTime &DateTime);

private:
	void InitByStr(std::string dateTimeStr, std::string formaterStr);

private:
	time_t seconds; //自1970起的秒数
	tm date;
};

BASE_NAMESPACE_END_DECL

#endif


/*************************************************************************************
*filename:	base_log.h
*
*to do:		定义log日志文件系统
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_LOG_H
#define __BASE_LOG_H

#include "revolver/base_typedef.h"

#include <iostream>
#include <fstream>
#include <sstream>
//#include <strstream>
#include <string>
#include <vector>

#include "revolver/base_os.h"
#include "revolver/base_singleton.h"
#include "revolver/base_thread_mutex.h"
#include "revolver/base_guard.h"

using namespace std;
using namespace BASE_NAMEPSACE_DECL;

//日志文件
class BaseLog
{
public:
	BaseLog(const char* _pfile_name);
	virtual ~BaseLog();
	
	//写入日志
	void		write_log(const string& str_log);
	void		flush();
	void		change_path(const std::string& _str_path);
	void		change_log_file(const string& filename);
protected:
	void		init_trace_file();
	const char* get_fullexepath(std::string& _str_path) const;
	bool		create_tracedir(const char* _pdir, const char* _pparent) const;
	string		get_log_name(bool cur_flag = false);

private:
	BaseLog& operator = (const BaseLog& _other);
	BaseLog(const BaseLog& _other);

protected:
	ofstream		m_of_file;
	string			m_file_path;
	string			m_filename;
	string			rename_filename;
	int32_t			m_line_count;
	int32_t			m_file_count;

	int32_t			day_;
	bool			wait_flush_;
};

//日志管理和索引
typedef vector<BaseLog*>	BaseLogVector;

class BaseLogManager
{
public:
	BaseLogManager();
	~BaseLogManager();

	int32_t			create_base_log(const char* _pfile_name);
	BaseLog*		get_log_handler(int32_t log_index);
	void			flush();

private:
	BaseLogVector	m_log_vector;
	uint64_t		flush_ts_;
};	

#define LOG_CREATE		CSingleton<BaseLogManager>::instance
#define LOG_INSTANCE	CSingleton<BaseLogManager>::instance
#define LOG_DESTROY		CSingleton<BaseLogManager>::destroy

//日志流
class BaseLogStreamInterface
{
public:
	enum TRACE_LEVEL 
	{
		fatal	= 0,
		error	= 1,
		warning = 2,
		general = 3,
		debug	= 4,
	};

public:
	BaseLogStreamInterface(){};
	virtual ~BaseLogStreamInterface(){};

	virtual std::ostream&	dump_trace(int32_t _level) = 0;
	virtual std::ostream&	get_ostream() = 0;
	virtual void			put_log(int32_t level) = 0;

	bool					is_enable_trace(int32_t level) const { return (level <= m_level); }
	void					set_trace_level(int32_t level) { m_level = level; }

protected:
	int32_t					m_level;

public:
	BaseThreadMutex			mutex_;
};

class BaseLogStream : public BaseLogStreamInterface
{
public:
	BaseLogStream(const char* pfile_name, int32_t level);
	~BaseLogStream();

public:
	std::ostream&			dump_trace(int32_t _level);
	std::ostream&			get_ostream();
	void					put_log(int32_t level);
	//这个模式下不能改变路径
	void					change_path(const std::string& path){};

private:
	int32_t					m_log_index;
	std::string				m_strFileName;
	ostringstream			m_strm;
};

class SingleLogStream : public  BaseLogStreamInterface,
						public  BaseLog
{
public:
	SingleLogStream(const char* pfile_name, int32_t level);
	~SingleLogStream();

public:
	std::ostream&			dump_trace(int32_t _level);
	std::ostream&			get_ostream();
	void					put_log(int32_t level);
};

//日志宏, 
#define BASE_TRACE(ofTrace, level, arg)	\
	if (ofTrace.is_enable_trace(level))	\
	{\
		BaseGuard<BaseThreadMutex> cf_mon(ofTrace.mutex_);\
		ofTrace.dump_trace(level) << "[" << get_file_name(__FILE__) << ":" << __LINE__ << "] " << arg << std::endl; \
		ofTrace.put_log(level);\
	}\

#define FATAL_TRACE(ofTrace, arg) \
	BASE_TRACE(ofTrace, BaseLogStreamInterface::fatal, arg);

#define ERROR_TRACE(ofTrace, arg) \
	BASE_TRACE(ofTrace, BaseLogStreamInterface::error, arg);


#define WARNING_TRACE(ofTrace, arg) \
	BASE_TRACE(ofTrace, BaseLogStreamInterface::warning, arg);

#define INFO_TRACE(ofTrace, arg) \
	BASE_TRACE(ofTrace, BaseLogStreamInterface::general, arg);

#define DEBUG_TRACE(ofTrace, arg) \
	BASE_TRACE(ofTrace, BaseLogStreamInterface::debug, arg);

#endif

/************************************************************************************/

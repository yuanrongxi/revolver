////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#endif

#include "base_os.h"
#include "gettimeofday.h"
#include "base_log_thread.h"
#include "base_log.h"
#include "date_time.h"
#include "base_timer_value.h"

#define _MAX_LOGLINE    250000
#define MAX_PATH 1024

const char* title_str[] = {
			"[fatal]",
			"[error]",
			"[warning]",
			"[info]",
			"[debug]"
};

const char* get_time_str(char *date_str)
{
	struct tm tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t now = tv.tv_sec;
#ifdef WIN32
	::localtime_s(&tm_now, &now);
#else
	::localtime_r(&now, &tm_now);
#endif

	sprintf(date_str, "%04d-%02d-%02d %02d:%02d:%02d.%3ld", tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
		tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, tv.tv_usec /1000);

	return date_str;
}

BaseLog::BaseLog(const char *_pfile_name) :m_line_count(0), m_file_count(0), wait_flush_(false) 
{
	string str_exepath;

#ifdef WIN32
	create_tracedir("\\log", get_fullexepath(str_exepath));
#else
	create_tracedir("/log", get_fullexepath(str_exepath));
#endif

	char buffer[MAX_PATH]= {0};

#ifdef WIN32
	char *path = _getcwd(buffer, MAX_PATH);
#else
	char* path = getcwd(buffer, MAX_PATH);
#endif

	m_filename = _pfile_name;

	m_file_path = buffer;

#ifdef WIN32
	m_file_path +=  "\\log\\";
#else
	m_file_path +=  "/log/";
#endif
//	m_file_path += m_filename;

	rename_filename = "";
	day_ = DateTime::GetNow().GetDay();

	init_trace_file();
}

BaseLog::~BaseLog()
{
	if (m_of_file.is_open())
	{
		m_of_file.flush();
		m_of_file.close();
	}
}
string BaseLog::get_log_name(bool cur_flag)
{
	string final_file_name = m_filename;
	if(!cur_flag)
	{
		final_file_name += "-";
		char date[20] = {0}; 
		sprintf(date, "%d-%d", DateTime::GetNow().GetMonth() + 1, DateTime::GetNow().GetDay());
		final_file_name += date;
	}

	final_file_name +=".log";

	return final_file_name;
}

void BaseLog::write_log(const string& str_log)
{
	if (m_of_file.is_open())
	{
		m_line_count ++;
		if (m_line_count > _MAX_LOGLINE || day_ != DateTime::GetNow().GetDay()) //过了晚上12点，换文件
		{
			this->flush();
			init_trace_file();
			day_ = DateTime::GetNow().GetDay();
		}

		char info[64] = {0};
		m_of_file << get_time_str(info); 
		m_of_file << str_log;

		wait_flush_ = true;
	}
}

void BaseLog::flush()
{
	if(m_of_file.is_open() && wait_flush_)
	{
		m_of_file.flush();
		wait_flush_ = false;
	}
}

void BaseLog::init_trace_file()
{	
	char newFile[1024] = {0};

	string rename_path = m_file_path + get_log_name();

	sprintf(newFile, "%s.%d", rename_path.data(), m_file_count++);
	if (m_of_file.is_open())
	{
		m_of_file.close();
	}

	string str_logfile_path = m_file_path + get_log_name(true);
	if(m_line_count > _MAX_LOGLINE || day_ != DateTime::GetNow().GetDay()) //文件满，清空文件
	{
		rename(str_logfile_path.data(), rename_filename.c_str());
		m_of_file.open(str_logfile_path.c_str(), std::ios_base::trunc|std::ios_base::out);
	}
	else //刚开始，直接追加到后面
	{
		m_of_file.open(str_logfile_path.c_str(), std::ios_base::app|std::ios_base::out);
	}

	rename_filename = newFile;

	m_line_count = 0;

}

const char* BaseLog::get_fullexepath(string& _str_path) const
{
	char buffer[MAX_PATH]= {0};
#ifdef WIN32
	char *path = _getcwd(buffer, MAX_PATH);
#else
	char* path = getcwd(buffer, MAX_PATH);
#endif
	_str_path = path;

	return _str_path.c_str();
}

bool BaseLog::create_tracedir(const char* _pdir, const char* _pparent) const
{
	if (_pparent == NULL || strlen(_pparent) == 0)
		return false;

	string str_dir = _pparent;
	str_dir += _pdir;
#ifdef WIN32
	_mkdir(str_dir.c_str());
#else 
	mkdir(str_dir.c_str(), S_IRWXU);
#endif

	return true;
}

void BaseLog::change_path(const std::string& _str_path)
{
	if (m_of_file.is_open())
		m_of_file.close();

	m_file_path = _str_path;

#ifdef WIN32
	m_file_path +=  "\\";
#else
	m_file_path +=  "/";
#endif

	init_trace_file();
}

void BaseLog::change_log_file(const string& filename)
{
	if (m_of_file.is_open())
		m_of_file.close();

	m_filename = filename;

	init_trace_file();
}

////////////////////////////////////////////////////////////////////////////////////////
BaseLogManager::BaseLogManager()
{
	flush_ts_ = CBaseTimeValue::get_time_value().msec();
}

BaseLogManager::~BaseLogManager()
{
	//释放文件
	for(size_t i = 0; i < m_log_vector.size(); ++ i)
	{
		delete m_log_vector[i];
	}
}

int32_t BaseLogManager::create_base_log(const char *pfile_name)
{
	int32_t index = -1;
	
	BaseLog *file_log = new BaseLog(pfile_name);
	index = m_log_vector.size();
	m_log_vector.push_back(file_log);

	return index;
}

BaseLog* BaseLogManager::get_log_handler(int32_t index)
{	
	if(index < 0 || index > m_log_vector.size())
		return NULL;
	else
		return m_log_vector[index];
}

void BaseLogManager::flush()
{
	uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
	if(cur_ts > flush_ts_ + 200)
	{
		for(int32_t i = 0; i < m_log_vector.size(); ++i)
			m_log_vector[i]->flush();

		flush_ts_ = cur_ts;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
BaseLogStream::BaseLogStream(const char* pfile_name, int32_t level)
{
	 m_level = level;
	m_log_index = LOG_INSTANCE()->create_base_log(pfile_name);
	m_strFileName = pfile_name;
}

BaseLogStream::~BaseLogStream()
{

}

ostream& BaseLogStream::dump_trace(int32_t _level)
{	 
	if(_level > 0 && _level < 5)
	{
		m_strm << title_str[_level] << "\t";
	}

	return m_strm;
}

std::ostream& BaseLogStream::get_ostream()
{
	return m_strm;
}

void BaseLogStream::put_log(int32_t level)
{
	//写入LOG线程
	LogInfoData* data = LOGPOOL.pop_obj();
	if(data != NULL)
	{
		data->index = m_log_index;
		data->level = level;
		data->str_log = m_strm.str();
		
		LOG_THREAD_INSTANCE()->put_log(data);

		m_strm.str("");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
SingleLogStream::SingleLogStream(const char* pfile_name, int32_t level) : BaseLog(pfile_name)
{
	m_level = level;
}

SingleLogStream::~SingleLogStream()
{

}


std::ostream& SingleLogStream::dump_trace(int32_t _level)
{
	if(_level > 0 && _level < 5)
	{
		write_log(title_str[_level]);
		m_of_file << "\t";
	}

	return m_of_file;
}

std::ostream& SingleLogStream::get_ostream()
{
	return m_of_file;
}


void SingleLogStream::put_log(int32_t level)
{
	flush();
}



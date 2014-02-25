#include <stdio.h>
#include "base_file.h"

BASE_NAMESPACE_BEGIN_DECL
BaseFile::BaseFile() : file_(NULL)
{

}

BaseFile::~BaseFile()
{
	BaseFile::close();
}

uint8_t BaseFile::get_state() const
{
	return file_ == NULL ?  SS_CLOSED : SS_OPEN;
}

bool BaseFile::open(const string& filename, const char* mode)
{
	BaseFile::close();

	file_ = fopen(filename.c_str(), mode);

	return (file_ != NULL);
}

void BaseFile::close()
{
	if(file_ != NULL)
	{
		fclose(file_);
		file_ = NULL;
	}
}

bool BaseFile::disable_buffering()
{
	if (file_ != NULL)
		return (setvbuf(file_, NULL, _IONBF, 0) == 0);
	else
		return true;
}

BaseFileResult BaseFile::read(void* buffer, size_t buffer_len, size_t& read_size, int32_t& error)
{
	if (file_ == NULL)
		return SR_EOS;

	read_size = fread(buffer, 1, buffer_len, file_);
	if((read_size == 0) && (buffer_len > 0)) 
	{
		if (feof(file_))
			return SR_EOS;

		error = error_no();
		return SR_ERROR;
	}

	return SR_SUCCESS;
}

BaseFileResult BaseFile::write(const void* data, size_t data_len, size_t& writte_size, int& error)
{
	if (file_ == NULL)
		return SR_EOS;

	writte_size = fwrite(data, 1, data_len, file_);
	if ((writte_size == 0) && (data_len > 0)) 
	{
		error = error_no();
		return SR_ERROR;
	}

	fflush(file_);

	return SR_SUCCESS;
}

void BaseFile::seek(size_t pos)
{
	if(file_ != NULL)
	{
		fseek(file_, pos, SEEK_SET);
	}
}

void BaseFile::flush()
{
	if(file_ != NULL)
	{
		fflush(file_);
	}
}

bool BaseFile::set_postion(size_t pos)
{
	if(file_ != NULL)
	{
		return (fseek(file_, pos, SEEK_SET) == 0);
	}

	return false;
}

bool BaseFile::get_postion(size_t& pos) const 
{
	bool ret = false;

	pos = 0;

	if (file_ != NULL)
	{
		long result = ftell(file_);
		if (result >= 0)
		{
			pos = result;
			ret = true;
		}
	}

	return ret;
}

BASE_NAMESPACE_END_DECL

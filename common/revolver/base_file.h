#ifndef __BASE_FILE_H
#define __BASE_FILE_H

#include <string>
#include "revolver/base_typedef.h"
#include "revolver/base_namespace.h"
#include "revolver/base_os.h"

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

enum BaseFileState 
{ 
	SS_CLOSED, 
	SS_OPENING, 
	SS_OPEN 
};

enum BaseFileResult 
{ 
	SR_ERROR, 
	SR_SUCCESS, 
	SR_BLOCK, 
	SR_EOS 
};

enum BaseFileEvent 
{ 
	SE_OPEN = 1, 
	SE_READ = 2, 
	SE_WRITE = 4, 
	SE_CLOSE = 8 
};


class BaseFile
{
public:
	BaseFile();
	~BaseFile();

	bool			open(const string& filename, const char* mode);
	void			close();

	bool			disable_buffering();


	uint8_t			get_state() const;

	BaseFileResult	read(void* buffer, size_t buffer_len, size_t& read_size, int32_t& error);
	BaseFileResult	write(const void* data, size_t data_len, size_t& writte_size, int& error);

	void			seek(size_t pos);
	void			flush();

	bool			set_postion(size_t pos);
	bool			get_postion(size_t& pos) const;

protected:
	  FILE*			file_;
};
BASE_NAMESPACE_END_DECL

#endif

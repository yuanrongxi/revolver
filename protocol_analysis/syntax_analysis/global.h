#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "base_translator.h"
#include <string>

using std::string;

#define NONE	  "none"//表示没有默认值

#define KEYWORDNUM		6
#define BASEDATATYPENUM 11

extern char* g_keywords[KEYWORDNUM];
extern char* g_basedatatype[BASEDATATYPENUM];

CBaseTranslator* GenerateTranslator(string& str_objlang);

enum BaseType//g_basedatatype的类型映射,值与数组索引相关
{
	enm_type_obj  = -2,
	enm_type_none = -1,
	enm_type_int8 = 0,
	enm_type_int16,
	enm_type_int32,
	enm_type_int64,
	enm_type_uint8,
	enm_type_uint16,
	enm_type_uint32,
	enm_type_uint64,
	enm_type_char,
	enm_type_string,
	enm_type_array,
};
#endif //GLOBAL_H_
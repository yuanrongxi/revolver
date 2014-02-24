#include "global.h"
#include "cpp_translator.h"
#include "lua_translator.h"
#include "as_translator.h"

char* g_keywords[KEYWORDNUM] = {
	"filetitle",
	"namespace",
	"var",
	"datatype",
	"typedef",
	"message"
};
char* g_basedatatype[BASEDATATYPENUM] = {
	"int8",
	"int16",
	"int32",
	"int64",
	"uint8",
	"uint16",
	"uint32",
	"uint64",
	"char",
	"string",
	"array"
};

CBaseTranslator* GenerateTranslator(string& str_objlang)
{
	if (str_objlang == "c++" || str_objlang == "cpp")
	{
		return new CCppTranslator;
	}
	else if (str_objlang == "lua")
	{
		return new CLuaTranslator;
	}
	else if (str_objlang == "as")
	{
		return new CASTranslator;
	}
	else
	{
		return NULL;
	}
}


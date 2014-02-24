#include "lua_translator.h"
#include "base_line_handler.h"
#include <cctype>
#include <stdlib.h>

CLuaTranslator::CLuaTranslator()
{

}

CLuaTranslator::~CLuaTranslator()
{

}

void CLuaTranslator::TranslateProtocol()
{
	OpenObjFile();

	AddFileDescToFile();
	AddUserClassDef();
	AddMsgBodyDef();

	CloseObjFile();
}

void CLuaTranslator::OpenObjFile()
{
	string str_ftitle = GET_FILEDESCMAP()->FindDescInfo(string(FILE_TITLE));
	CBaseLineHandler::StringToLower(str_ftitle);
	str_ftitle += ".lua";

	if (out_file_.is_open())
	{
		out_file_.flush();
		out_file_.close();
		out_file_.clear();
	}

	out_file_.open(str_ftitle.c_str(), ofstream::out);
}

void CLuaTranslator::CloseObjFile()
{
	out_file_.close();
}

void CLuaTranslator::AddFileDescToFile()
{
	string str_ftitle = GET_FILEDESCMAP()->FindDescInfo(string(FILE_TITLE));
	CBaseLineHandler::StringToUpper(str_ftitle);

	out_file_ << "require\"pack\"" << endl; 
	InsertSeprator(out_file_);		  
	out_file_ << "local table = require\"table\"" << endl 
			  << "local string = require\"string\"" << endl 
			  << endl
			  << "local bpack = string.pack" << endl
			  << "local bunpack = string.unpack" << endl
			  << "local setmetatable = _G.setmetatable" << endl
			  << endl
			  << "local type = _G.type" << endl
			  << "local error = _G.error" << endl
			  << endl
			  << "module(\"" << str_ftitle << "\")" << endl;
}

void CLuaTranslator::InsertSeprator(ofstream& out)
{
	out << endl << "------------------------------------------" << endl;
}

void CLuaTranslator::AddUserClassDef()
{
	InsertSeprator(out_file_);

	const UserClassDescMap& classdesc_map = CVistor::GetUserClassDescMap();
	const UserClassOrder&	class_order = CVistor::GetUserClassOrder();

	for (UserClassOrder::const_iterator itr = class_order.begin();
		itr != class_order.end(); ++itr)
	{
		UserClassDescMap::const_iterator m_itr = classdesc_map.find(*itr);

		InitUserClass(*itr, m_itr->second);

		GenarateObjNew(*itr);
		GenarateObjPackFn(*itr, m_itr->second);
		GenarateObjUnpackFn(*itr, m_itr->second);
	}
}

void CLuaTranslator::AddMsgBodyDef()
{
	const MsgBodyDefMap& msgbody_map = CVistor::GetMsgBodyMap();
	const MsgBodyOrder& msgbody_order = CVistor::GetMsgBodyOrder();

	for (MsgBodyOrder::const_iterator itr = msgbody_order.begin();
		itr != msgbody_order.end(); ++itr)
	{
		string msgid = *itr;
		MsgBodyDefMap::const_iterator m_itr = msgbody_map.find(CMsgDefDesc(msgid, string()));

		InitUserClass(m_itr->first.msg_cname_, m_itr->second);

		GenarateObjNew(m_itr->first.msg_cname_);
		GenarateObjPackFn(m_itr->first.msg_cname_, m_itr->second);
		GenarateObjUnpackFn(m_itr->first.msg_cname_, m_itr->second);

		string msgid_num = GET_VARDEFMAP()->fetch_value(msgid);
		GenarateInterfaceFile(msgid_num);
	}
}

void CLuaTranslator::InitUserClass(const string& str_name, const MemItemVec& mem_set)
{
	out_file_ << str_name << " = {";

	size_t count = 0;
	for (MemItemVec::const_iterator c_ite = mem_set.begin();
		c_ite != mem_set.end(); c_ite++)
	{
		out_file_ << c_ite->m_name;
		if (c_ite->m_value != NONE)
		{
			out_file_ << " = " << c_ite->m_value;
		}
		if (count < mem_set.size() - 1)
		{
			out_file_ << ",";
		}
		++count;
	}
	out_file_ << "}" << endl;

	InsertSeprator(out_file_);
}

void CLuaTranslator::GenarateObjNew(const string& str_name)
{
	out_file_ << "function " << str_name << ".new()" << endl
			  << "\tlocal o = {}" << endl
			  << "\tsetmetatable(o, " <<str_name << ")" << endl
			  << "\t" << str_name << ".__index = " << str_name << endl
			  << "\treturn o" << endl << "end";
	InsertSeprator(out_file_);
}

void CLuaTranslator::GenarateObjPackFn(const string& str_name, const MemItemVec& mem_set)
{
	out_file_ << "function " << str_name << ":pack()" << endl
			  << "\tlocal bin = {}" << endl << endl;

	for (MemItemVec::const_iterator c_ite = mem_set.begin();
		c_ite != mem_set.end(); c_ite++)
	{
		BaseType l_type = CBaseLineHandler::AnalysisValueType(c_ite->m_type);

		if (l_type == enm_type_obj)
		{
			GenaratePackObjCode(c_ite->m_name);
		}
		else if (l_type == enm_type_array)
		{
			GenaratePackArrayCode(c_ite->m_type, c_ite->m_name);
		}
		else
		{
			LUA_IF_VALUE_NIL(c_ite->m_name);
			GenaratePackValueCode(c_ite->m_type, c_ite->m_name, l_type);
			LUA_END_IF;
		}
 	}

	out_file_ << "\treturn table.concat(bin, \"\")" << endl
			  << "end";
	InsertSeprator(out_file_);
}

void CLuaTranslator::GenarateObjUnpackFn(const string& str_name, const MemItemVec& mem_set)
{
	out_file_ << "function " << str_name << ":unpack(data, pos)" << endl
			  << "\tif type(data) ~= \"string\" then" << endl
			  << "\t\terror(\"need a string\")" << endl
			  << "\tend" << endl
			  << "\tlocal size = string.len(data)" << endl
			  << "\tif pos >= size then" << endl
			  << "\t\treturn size" << endl
			  << "\tend" << endl
			  << "\tlocal val" << endl
			  << "\tlocal next = pos" << endl;

	for (MemItemVec::const_iterator c_ite = mem_set.begin();
		c_ite != mem_set.end(); c_ite++)
	{
		BaseType l_type = CBaseLineHandler::AnalysisValueType(c_ite->m_type);

		if (l_type == enm_type_obj)
		{
			GenarateUnpackObjCode(c_ite->m_type, c_ite->m_name);
		}
		else if (l_type == enm_type_array)
		{
			GenarateUnpackArrayCode(c_ite->m_type, c_ite->m_name);
		}
		else
		{
			GenarateUnpackValueCode(c_ite->m_type, c_ite->m_name, l_type);	
		}
	}
	
	out_file_ << "\treturn next" << endl << "end" << endl;
	InsertSeprator(out_file_);
}

void CLuaTranslator::GenaratePackObjCode(const string& val_name)
{
	out_file_ << "\tif self." << val_name << " ~= nil then" << endl
		      << "\t\tbin[#bin + 1] = bpack(\">A\", self." << val_name << ":pack()" << endl
			  << "\tend" << endl << endl;
}

void CLuaTranslator::GenaratePackArrayCode(const string& val_type, const string& val_name)
{
	LUA_IF_VALUE_NIL(val_name);
	out_file_ << "\t\tbin[#bin + 1] = bpack(\">I\", #self." << val_name << ")" << endl;
			  
	LUA_END_IF;
	out_file_ << "\tfor i = 1, #self." << val_name << " do" << endl;

	string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(val_type);
	
	if (src_type.empty())
	{
		src_type = val_type;
	}

	StringArray elems;
	CBaseLineHandler::SplitComplexType(src_type, elems);

	if (elems.size() != 2)
	{
		return;
	}
	BaseType l_type = CBaseLineHandler::AnalysisValueType(elems[1]);

	if (l_type == enm_type_obj)
	{
		GenaratePackObjCode(val_name + "[i]");
	}
	else
	{
		GenaratePackValueCode(src_type, val_name + "[i]", l_type);
	}
	
	out_file_ << "\tend" << endl;
}

void CLuaTranslator::GenaratePackValueCode(const string& val_type, const string& val_name, BaseType t)
{
	string fomat = GetLuaFormat(t);
	
	if (t == enm_type_string)
	{
		out_file_ << "\t\tbin[#bin + 1] = bpack(\">I\", #self." << val_name << ")" << endl;
	}

	if (fomat != "")
	{
		out_file_ << "\t\tbin[#bin + 1] = bpack(\"" << fomat << "\", self." << val_name << ")" << endl;
	}
}

void CLuaTranslator::GenarateUnpackObjCode(const string& val_type, const string& val_name)
{
	out_file_ << "\tlocal t = " << val_type << ".new()" << endl
			  << "\tnext = t:unpack(data, next)" << endl
			  << "\tself." << val_name << "= t:" << endl
			  << "\tnext = self." << val_name << ":unpack(data, next)" << endl;
	LUA_CHECK_DECODE_POS;
}

void CLuaTranslator::GenarateUnpackValueCode(const string& val_type, const string& val_name, BaseType t)
{
	string fomat = GetLuaFormat(t);
	
	if (t == enm_type_string)
	{
		out_file_ << "\tnext, val = bunpack(data, \">I\", next)" << endl;
		LUA_CHECK_DECODE_POS;
		out_file_ << "\tnext, val = bunpack(data, \">A\"..val, next)" << endl
				  << "\tself." << val_name << " = val" << endl;
		LUA_CHECK_DECODE_POS;
	}
	else
	{
		out_file_ << "\tnext, val = bunpack(data, \"" << fomat << "\", next)" << endl
				  << "\tself." << val_name << " = val" << endl;
		LUA_CHECK_DECODE_POS;
	}
}

void CLuaTranslator::GenarateUnpackArrayCode(const string& val_type, const string& val_name)
{
	out_file_ << "\tlocal arr_size" << endl
			  << "\tnext, val = bunpack(data, \">I\", next)" << endl
			  << "\tif next >= size then" << endl
			  << "\t\treturn next" << endl
			  << "\tend" << endl << endl
			  << "\tarr_size = val" << endl
			  << "\tif val ~= 0 then" << endl;

	string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(val_type);

	if (src_type.empty())
	{
		src_type = val_type;
	}

	StringArray elems;
	CBaseLineHandler::SplitComplexType(src_type, elems);

	if (elems.size() != 2)
	{
		return;
	}
	BaseType l_type = CBaseLineHandler::AnalysisValueType(elems[1]);

	if (l_type == enm_type_obj)
	{
		out_file_ << "\tfor i = 1, arr_size do" << endl;

		GenarateUnpackObjCode(val_type, val_name + "[i]");
	}
	else
	{
		out_file_ << "\tself." << val_name << " = {}" << endl
				  << "\tfor i = 1, arr_size do" << endl;

		GenarateUnpackValueCode(src_type, val_name + "[i]", l_type);
	}
	
	out_file_ << "\t\tend" << endl
			  << "\tend" << endl;			 
}

void CLuaTranslator::GenarateInterfaceFile(string& msgid)
{
	CBaseLineHandler::StringToLower(msgid);
	string f_name("lua_msgid_");
	f_name = f_name + msgid + ".lua";

	ofstream out;

	ifstream fin;
	fin.open(f_name.c_str(), ifstream::in);
	if (!fin)
	{
		out.open(f_name.c_str(), ofstream::app);
	}
	else
	{
		out.open(f_name.c_str(), ofstream::out);
		out << "";
		out.clear();
		out.close();
		out.open(f_name.c_str(), ofstream::app);
	}

	CBaseLineHandler::StringToLower(msgid);
	string fn_name("Handle_Msg_");
	fn_name += msgid;

	InsertSeprator(out);
	out << "--该文件处理消息ID为: " << msgid << "的消息"
		<< "入口函数为Handle_Msg_" << msgid << endl;
	InsertSeprator(out);

	out << "function Handle_Msg_" << msgid << "()" << endl
		<< "end" << endl;
}

string CLuaTranslator::GetLuaFormat(BaseType t)
{
	switch (t)
	{
	case enm_type_string:
		return ">A";
		break;
	case enm_type_int8:
		return ">c";
		break;
	case enm_type_int16:
		return ">h";
		break;
	case enm_type_int32:
		return ">i";
		break;
	case enm_type_int64:
		return ">l";
		break;
	case enm_type_uint8:
		return ">b";
		break;
	case enm_type_uint16:
		return ">H";
		break;
	case enm_type_uint32:
		return ">I";
		break;
	case enm_type_uint64:
		return ">L";
		break;
	case enm_type_char:
		return ">c";
		break;
	default:
		return "";
		break;
	}
}

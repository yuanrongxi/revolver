#include "cpp_translator.h"
#include "base_line_handler.h"
#include <cctype>
#include <stdlib.h>

using namespace std;

char* g_typecomptable[COMPTABLEROW][COMPTABLECOLM] = {
	{
		"int8", "int16", "int32", "int64", 
		"uint8", "uint16", "uint32", "uint64", 
		"array"
	},
	{
		"int8_t", "int16_t", "int32_t", "int64_t",
		"uint8_t", "uint16_t", "uint32_t", "uint64_t",
		"vector"
	}
};

CCppTranslator::CCppTranslator()
{

}

CCppTranslator::~CCppTranslator()
{

}

void CCppTranslator::TranslateProtocol()
{
	OpenObjFile();

	AddFileDescToFile();
	AddVarDefToFile();
	AddTypeDefToFile();
	AddUserClassDef();
	OutputLeftTypedef();

	AddMsgBodyDef();

	OutputLeftFileDesc();
	CloseObjFile();
}

void CCppTranslator::OpenObjFile()
{
	string str_ftitle = GET_FILEDESCMAP()->FindDescInfo(string(FILE_TITLE));
	CBaseLineHandler::StringToLower(str_ftitle);
	str_ftitle += ".h";

	if (out_file_.is_open())
	{
		out_file_.flush();
		out_file_.close();
		out_file_.clear();
	}

	out_file_.open(str_ftitle.c_str(), ofstream::out);
}

void CCppTranslator::CloseObjFile()
{
	out_file_.close();
}

void CCppTranslator::AddFileDescToFile()
{
	string str_ftitle = GET_FILEDESCMAP()->FindDescInfo(string(FILE_TITLE));
	CBaseLineHandler::StringToUpper(str_ftitle);

	out_file_ << "#ifndef " << str_ftitle + "_H__" << endl;
	out_file_ << "#define " << str_ftitle + "_H__" << endl;
	left_file_desc_.push_back("#endif");

	out_file_ << endl;
	out_file_ << "#include \"base_packet.h\"" << endl;
	out_file_ << "#include \"core_server_type.h\"" << endl;
	out_file_ << "#include \"core_message_map_decl.h\"" << endl;
	out_file_ << "#include \"base_inet_addr.h\"" << endl;

	out_file_ << "#include <string>" << endl;
	out_file_ << "using namespace std;" << endl;
	out_file_ << "using namespace BASEOBJECT;" << endl;

	out_file_ << endl;

	string str_ns = GET_FILEDESCMAP()->FindDescInfo(string(NAME_SPACE));
	out_file_ << "namespace " << str_ns << " {" << endl;
	left_file_desc_.push_back("}");

	out_file_ << endl;
}

void CCppTranslator::AddVarDefToFile()
{
	const VarDefMap& var_map = CVistor::GetVarMap();
	const VarOrder&	var_order = CVistor::GetVarOrder();

	for (VarOrder::const_iterator itr = var_order.begin();
		itr != var_order.end(); itr++)
	{
		out_file_ << "#define " << *itr << "\t";
		const VarDefMap::const_iterator m_itr = var_map.find(*itr);
		out_file_ << m_itr->second << endl;
	}

	out_file_ << endl;
}

void CCppTranslator::AddTypeDefToFile()
{
	const TypeDefSet& typedef_set = CVistor::GetTypedefSet();
	for (TypeDescSetIte c_ite = typedef_set.begin();
		c_ite != typedef_set.end(); c_ite++)
	{
		StringArray v_elem;
		CBaseLineHandler::SplitComplexType((*c_ite).type_, v_elem);
		if (v_elem.empty())
		{
			if (GET_USERCLASSMAP()->IsUserDataType((*c_ite).type_))
			{
				left_typedef_.push_back(string("typedef ") + 
					GET_TRANSTABLE()->GetCorrespondType((*c_ite).type_) +
					+ "\t\t" + (*c_ite).alias_ + ";");
				continue;
			}
		}
		else
		{
			if (GET_USERCLASSMAP()->IsUserDataType(v_elem[1]))
			{
				left_typedef_.push_back(string("typedef ") 
					+ GET_TRANSTABLE()->GetCorrespondType(v_elem[0]) + "<"
					+ GET_TRANSTABLE()->GetCorrespondType(v_elem[1]) + ">"
					+ "\t\t" + (*c_ite).alias_ + ";");
				continue;
			}
		}
		out_file_ << *c_ite << endl;
	}
}

void CCppTranslator::AddUserClassDef()
{
	out_file_ << endl;

	const UserClassDescMap& classdesc_map = CVistor::GetUserClassDescMap();
	const UserClassOrder&	class_order = CVistor::GetUserClassOrder();

	for (UserClassOrder::const_iterator itr = class_order.begin();
		itr != class_order.end(); ++itr)
	{
		UserClassDescMap::const_iterator m_itr = classdesc_map.find(*itr);
		out_file_ << "class " << *itr << endl;
		out_file_ << "{" << endl;
	 
		FillConstructorAndDataMem(*itr, m_itr->second);
		GenerateBinStreamFunction(*itr, m_itr->second, string("<<"));
	 	GenerateBinStreamFunction(*itr, m_itr->second, string(">>"));
	 	GeneratePrintFunction(*itr, m_itr->second);
	 	GenerateOstreamFunction(*itr, m_itr->second);
		 
		out_file_ << "};" << endl;
	}
}

void CCppTranslator::FillConstructorAndDataMem(const string& str_name, const MemItemVec& mem_set)
{
	out_file_ << "public:" << endl;
	out_file_ << "\t" << str_name << "()" << endl;
	out_file_ << "\t{" << endl;

	MemItemVec v_mem;
	for (MemItemVec::const_iterator c_ite = mem_set.begin();
		c_ite != mem_set.end(); c_ite++)
	{
		v_mem.push_back(*c_ite);
		if (c_ite->m_value != NONE)
		{
			out_file_ << "\t\t"; 
			out_file_ << (*c_ite).m_name;
			out_file_ << " = " << c_ite->m_value << ";";
			out_file_ << endl;
		}
	}

	out_file_ << "\t}" << endl;

	out_file_ << "\t~" << str_name << "()" << endl;
	out_file_ << "\t{" << endl;
	out_file_ << "\t}" << endl;
	out_file_ << endl;

	out_file_ << "public:" << endl;
	for (MemItemVec::iterator ite = v_mem.begin();
		ite != v_mem.end(); ite++)
	{
		out_file_ << "\t";
		out_file_ << *ite << ";" << endl;
	}

	out_file_ << endl;
}

void CCppTranslator::AddMsgBodyDef()
{
	out_file_ << endl;

	const MsgBodyDefMap& msgbody_map = CVistor::GetMsgBodyMap();
	const MsgBodyOrder& msgbody_order = CVistor::GetMsgBodyOrder();
	for (MsgBodyOrder::const_iterator itr = msgbody_order.begin();
		itr != msgbody_order.end(); ++itr)
	{
		string msgid = *itr;
		MsgBodyDefMap::const_iterator m_itr = msgbody_map.find(CMsgDefDesc(msgid, string()));

		out_file_ << "class ";
		out_file_ << m_itr->first << " : public CBasePacket" << endl;
		out_file_ << "{" << endl;

		FillConstructorAndDataMem(m_itr->first.msg_cname_, m_itr->second);

		out_file_ << "public:" << endl;
		FillEncodeFunction(m_itr->second);
		FillDecodeFunction(m_itr->second);
		GeneratePrintFunction(m_itr->first.msg_cname_, m_itr->second);

		out_file_ << "};" << endl;
	}

	out_file_ << endl;
}

void CCppTranslator::FillEncodeFunction(const MemItemVec& mem_set)
{
	out_file_ << "\tvoid	Pack(BinStream& strm) const" << endl;
	out_file_ << "\t{" << endl;

	int count = 0;
	for (MemItemVec::const_iterator ite = mem_set.begin();
		ite != mem_set.end(); ite++)
	{
		count++;

		string str_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(ite->m_type);
		if (str_type.empty())
		{
			out_file_ << "\t\tstrm << " << ite->m_name << ";" << endl;
		}
		else
		{
			if (str_type.find("array") == string::npos)
			{
				out_file_ << "\t\tstrm << " << ite->m_name << ";" << endl;
			}
			else
			{
				OutputArrayToFile(ite->m_name, string("<<"));
			}
		}
	}
	
	// out_file_ << ";" << endl;
	out_file_ << "\t}" << endl;
}

void CCppTranslator::FillDecodeFunction(const MemItemVec& mem_set)
{
	out_file_ << "\tvoid	UnPack(BinStream& strm)" << endl;
	out_file_ << "\t{" << endl;

	int count = 0;
	for (MemItemVec::const_iterator ite = mem_set.begin();
		ite != mem_set.end(); ite++)
	{
		count++;
		
		string str_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(ite->m_type);
		if (str_type.empty())
		{
			out_file_ << "\t\tstrm";
			out_file_ << " >> " << ite->m_name << ";" << endl;
		}
		else
		{
			if (str_type.find("array") == string::npos)
			{
				out_file_ << "\t\tstrm >> " << ite->m_name << ";" << endl;
			}
			else
			{
				OutputArrayToFile(ite->m_name, string(">>"));
			}
		}
	}

	// out_file_ << endl;
	out_file_ << "\t}" << endl;
}

void CCppTranslator::OutputArrayToFile(const string& str_name, const string& str_op, const string& strm_name)
{
	static int suffix_num = 0;
	char str[10];
	memset(str, '\0', sizeof(str));
	_itoa_s(suffix_num, str, 10);
	string temp_count(string("count_") + string(str));
	suffix_num++;

	out_file_ << endl;
	out_file_ << "\t\tint32_t " << temp_count << " = ";

	if (str_op == "<<")
	{
		out_file_ << str_name << ".size();" << endl;
		out_file_ << "\t\t"<< strm_name << "<< " << temp_count << ";" << endl; 
	}
	else if (str_op == ">>")
	{
		out_file_ << "0;" << endl;
		out_file_ << "\t\t" << strm_name << ">> " << temp_count << ";" << endl;
		out_file_ << "\t\t" << str_name << ".resize(" << temp_count << ");" << endl;
	}

	out_file_ << "\t\tfor (int32_t i = 0; i < " << temp_count << ";" << " i++)" << endl;
	out_file_ << "\t\t{" << endl;
	out_file_ << "\t\t\t"<< strm_name << " " << str_op << " " << str_name << "[i];" << endl;
	out_file_ << "\t\t}";
	out_file_ << endl;
}

void CCppTranslator::OutputLeftFileDesc()
{
	out_file_ << endl;
	
	const MsgBodyDefMap& msgbody_map = CVistor::GetMsgBodyMap();

	if (!msgbody_map.empty())
	{
		string f_title = GET_FILEDESCMAP()->FindDescInfo(string(FILE_TITLE));
		CBaseLineHandler::StringToUpper(f_title);
		out_file_ << "MESSAGEMAP_DECL_BEGIN(" << f_title << ")" << endl; 

		const MsgBodyOrder& msgbody_order = CVistor::GetMsgBodyOrder();
		for (MsgBodyOrder::const_iterator itr = msgbody_order.begin();
			itr != msgbody_order.end(); ++itr)
		{
			string msgid = *itr;
			MsgBodyDefMap::const_iterator m_itr = msgbody_map.find(CMsgDefDesc(msgid, string()));
			out_file_ << "MESSAGEMAP_REGISTER(" << msgid ;
			out_file_ << ", new " << m_itr->first.msg_cname_ << "())";
			out_file_ << endl;
		}
	
		out_file_ << "MESSAGEMAP_DECL_END()" << endl;
		out_file_ << endl;
	}

	//注意：若条件为i>=0, size_type为正数，当i自减小于0后，会变成一个大的正数，导致数组越界
	for (StringArray::size_type i = left_file_desc_.size() - 1; i > 0; i--)
	{
// 		cout << "i" << i << endl;
		out_file_ << left_file_desc_[i] << endl;
	}
	//处理 i==0 的情况
	out_file_ << left_file_desc_[0] << endl;
}

void CCppTranslator::OutputLeftTypedef()
{
	out_file_ << endl;

	for (StringArray::iterator ite = left_typedef_.begin();
		ite != left_typedef_.end(); ite++)
	{
		out_file_ << *ite << endl;
	}
}

void CCppTranslator::OutputInetAddrType(const string& str_vname, const string& str_op)
{
	static int suffix_num = 0;

	if (str_op == "<<")
	{
		out_file_ << "\t\tstrm << " << str_vname << ".get_ip() << " 
			<< str_vname << ".get_port();" << endl;
	}
	else if (str_op == ">>")
	{
		char str[10];
		memset(str, '\0', sizeof(str));
		_itoa_s(suffix_num, str, 10);
		string temp_ip(string("ip_") + string(str));
		string temp_port(string("port_") + string(str));
		suffix_num++;

		out_file_ << "\t\tuint32_t " << temp_ip << ";" << endl 
			<< "\t\tuint16_t " << temp_port << ";" << endl;
		out_file_ << "\t\tstrm" << " >> " << temp_ip
			      <<  " >> " << temp_port << endl;
		out_file_ << "\t\t" << str_vname << ".set_ip(" << temp_ip << ");" << endl;
		out_file_ << "\t\t" << str_vname << ".set_port(" << temp_port << ");" << endl;
	}
}

void CCppTranslator::GenerateBinStreamFunction(const string& str_type, const MemItemVec& v_mem, const string& str_op)
{
	out_file_ << endl;
	out_file_ << "\tfriend BinStream& operator" << str_op << "(BinStream& strm, ";
	if (str_op == "<<")
	{
		out_file_ << "const " << str_type;
	}
	else
	{
		out_file_ << str_type;
	}

	out_file_ << "& packet)" << endl << "\t{" << endl;
	out_file_ << "\t\tBinStream bstrm;" << endl;
	out_file_ << "\t\tstring data_str;" << endl;

	if(str_op == ">>")
	{
		out_file_ << "\t\tstrm >> data_str;" << endl << "\t\tbstrm = data_str;" << endl;
	}

	for (MemItemVec::const_iterator c_ite = v_mem.begin();
		c_ite != v_mem.end(); c_ite++)
	{
		string str_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(c_ite->m_type);
		if (str_type.empty())
		{
			out_file_ << "\t\tbstrm " << str_op << " packet." << c_ite->m_name << ";" << endl;
		}
		else
		{
			if (str_type.find("array") == string::npos)
			{
				out_file_ << "\t\tbstrm " << str_op << " packet." << c_ite->m_name << ";" << endl;
			}
			else
			{
				OutputArrayToFile(string("packet.") + c_ite->m_name, str_op, "bstrm");
			}
		}
	}

	if(str_op == "<<")
	{
		out_file_ << "\t\tbstrm.bin_to_string(data_str);" << endl << "\t\tstrm << data_str;" << endl;
		
	}

	out_file_ << "\t\treturn strm;" << endl << "\t}" << endl;
}


void CCppTranslator::GeneratePrintFunction(const string& str_type, const MemItemVec& v_mem)
{
	out_file_ << endl;
	out_file_ << "\tvoid  Print(std::ostream& os) const" << endl;
	out_file_ << "\t{" << endl;
	out_file_ << "\t\tos << \"" << str_type << ", {\";" << endl;

	for (MemItemVec::const_iterator c_ite = v_mem.begin();
		c_ite != v_mem.end(); c_ite++)
	{
		//处理二进制数据BIN_DATA，不需要dump
		if (c_ite->m_type == "BIN_DATA")
		{
			out_file_ << "\t\tos << \"" << c_ite->m_name << "\'s size = \"";
			out_file_ << " << " << c_ite->m_name << ".size();" << endl;
			continue;
		}

		string type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(c_ite->m_type);

		if (type.empty())
		{
			out_file_ << "\t\tos << \"" << c_ite->m_name << " = \" << ";
			Output8to16Convert(c_ite->m_type);
			out_file_ << c_ite->m_name << " << \",\";" << endl;
		}
		else
		{
			if (type.find("array") == string::npos)
			{
				out_file_ << "\t\tos << \"" << c_ite->m_name << " = \" << ";
				Output8to16Convert(c_ite->m_type);
				out_file_ << c_ite->m_name << " << \",\";" << endl;
			}
			else
			{
				out_file_ << "\t\tos << \"" << c_ite->m_name << " = \"" << ";" << endl;
				out_file_ << "\t\tfor (uint32_t i = 0; i < " << c_ite->m_name << ".size()" << ";" << " i++)" << endl;
				out_file_ << "\t\t{" << endl;
				out_file_ << "\t\t\tos << ";
				StringArray v_elem;
				CBaseLineHandler::SplitComplexType(type, v_elem);
				if (v_elem.size() == 2)
				{
					Output8to16Convert(v_elem[1]);
				}

				out_file_ << c_ite->m_name << "[i] << \",\";" << endl;
				out_file_ << "\t\t}";
				out_file_ << endl;
			}
		}
	}

	out_file_ << "\t\tos << \"}\";" << endl;
	out_file_ << "\t}" << endl;
}

void CCppTranslator::GenerateOstreamFunction(const string& str_type, const MemItemVec& v_mem)
{
	out_file_ << endl;
	out_file_ << "\tfriend ostream& operator<<(ostream& os, const " << str_type << "& packet)" << endl;
	out_file_ << "\t{" << endl;
	out_file_ << "\t\tpacket.Print(os);" << endl;
	out_file_ << "\t\treturn os;" << endl;
	out_file_ << "\t}" << endl;
}

void CCppTranslator::Output8to16Convert(const string& type)
{
	if (type == "int8")
	{
		out_file_ << "(int16_t)";
	}
	else if (type == "uint8")
	{
		out_file_ << "(uint16_t)";
	}
}

CTypeTransTable::CTypeTransTable()
{
	for (int i = 0; i < COMPTABLECOLM; i++)
	{
		typetrans_table_.insert(make_pair(g_typecomptable[0][i], g_typecomptable[1][i]));
	}
}

CTypeTransTable::~CTypeTransTable()
{

}

string CTypeTransTable::GetCorrespondType(const string& str_type)
{
	TypeTransTable::iterator ite = typetrans_table_.find(str_type);
	if (ite != typetrans_table_.end())
	{
		return ite->second;
	} 
	else
	{
		return str_type;
	}
}

ofstream& operator <<(ofstream& fout, const CMsgDefDesc &msgdesc)
{
	fout << msgdesc.msg_cname_;
	return fout;
}

ofstream& operator <<(ofstream& fout, const CTypedefDesc& typedefdesc)
{
	fout << "typedef ";

	StringArray v_type;
	CBaseLineHandler::SplitComplexType(typedefdesc.type_, v_type);
	if (v_type.empty())
	{
		fout << GET_TRANSTABLE()->GetCorrespondType(typedefdesc.type_);
	}
	else
	{
		fout << GET_TRANSTABLE()->GetCorrespondType(v_type[0]);
		fout << "<";
		fout << GET_TRANSTABLE()->GetCorrespondType(v_type[1]);
		fout << ">";
	}
	 
	fout << "\t\t" << typedefdesc.alias_
		 << ";";
	return fout;
}

ofstream& operator <<(ofstream& fout, const CMemItem& item)
{
	fout << GET_TRANSTABLE()->GetCorrespondType(item.m_type)
		 << "\t\t" << item.m_name;
	return fout;
}

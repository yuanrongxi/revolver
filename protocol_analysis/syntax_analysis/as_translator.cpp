#include "as_translator.h"
#include "base_line_handler.h"

void CASTranslator::TranslateProtocol()
{
	const MsgBodyDefMap& msgbody_map = CVistor::GetMsgBodyMap();

	for (MsgBodyDefMap::const_iterator itr = msgbody_map.begin();
		itr != msgbody_map.end(); ++itr)
	{
		string msgid = GET_VARDEFMAP()->fetch_value(itr->first.msg_id_);
		generate_package(itr->second, itr->first.msg_cname_, msgid);
	}

	const UserClassDescMap& user_class_map = CVistor::GetUserClassDescMap();
	for (UserClassDescMap::const_iterator itr = user_class_map.begin();
		itr != user_class_map.end(); ++itr)
	{
		generate_package(itr->second, itr->first);
	}
}

ofstream& CASTranslator::open_file(const string& fname)
{
	if (out_file_.is_open())
	{
		out_file_.flush();
		out_file_.close();
		out_file_.clear();
	}

	out_file_.open(fname.c_str(), ofstream::out);
	return out_file_;
}

void CASTranslator::generate_package(const MemItemVec& mem_vec, string cname, string msgid /*= ""*/)
{
	string f_name = cname + ".as";
	ofstream& fout = open_file(f_name);

	ASPacketGenerator package(cname, msgid, new ASClassFunctions);
	package.code_generate(fout, mem_vec);
	fout.close();
}

void ASPacketGenerator::code_generate(ofstream& o_file, const MemItemVec& mem_vec)
{
	o_file << "package notifications" << endl
		<< "{" << endl
		<< "\timport flash.utils.ByteArray;" << endl
		<< "\timport flash.utils.Endian;" << endl
		<< "\tpublic class " << class_name_ << endl
		<< "\t{" << endl;
	if (!msg_id_.empty())
	{
		o_file << "\t\tstatic public const MSGID : uint = " << msg_id_ 
			   << GET_VARDEFMAP()->fetch_value(msg_id_) << ";" << endl << endl;
	}

	generate_members(o_file, mem_vec);
	func_constructor(o_file, mem_vec);

	if (generator_ != NULL)
	{
		generator_->code_generate(o_file, mem_vec);
	}

	o_file << "\t}" << endl
		<< "}" << endl;
}

void ASPacketGenerator::func_constructor(ofstream& o_file, const MemItemVec& mem_vec)
{
	o_file << "\t\tpublic function " << class_name_ << "()" << endl
		<< "\t\t{" << endl;

	for (MemItemVec::const_iterator c_ite = mem_vec.begin();
		c_ite != mem_vec.end(); c_ite++)
	{
		if (c_ite->m_value != NONE)
		{
			o_file << "\t\t\t"; 
			o_file << (*c_ite).m_name;
			o_file << " = " << c_ite->m_value << ";";
			o_file << endl;
		}
		if (c_ite->m_type == "BIN_DATA" || c_ite->m_type == "string")
		{
			o_file << "\t\t\t"; 
			o_file << (*c_ite).m_name << " = \"\"" << ";";
			o_file << endl;
		}
	}
	o_file << "\t\t}" << endl;
}

void ASPacketGenerator::generate_members(ofstream& o_file, const MemItemVec& mem_vec)
{
	for (MemItemVec::const_iterator itr = mem_vec.begin();
		itr != mem_vec.end(); ++itr)
	{
		o_file << "\t\tpublic var " << itr->m_name << " : " << generate_memtype(itr->m_type) << ";" << endl;
	}
	o_file << endl;
}
	
string ASPacketGenerator::generate_memtype(const string& m_type)
{
	if (m_type == "BIN_DATA")
	{
		return "ByteArray = new ByteArray";
	}
	BaseType base_type = CBaseLineHandler::AnalysisValueType(m_type);

	if (base_type == enm_type_obj)
	{
		return m_type + string(" = new ") + m_type;
	}
	else if (base_type == enm_type_array)
	{
		string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(m_type);
		if (src_type.empty())
		{
			src_type = m_type;
		}

		StringArray vec;
		CBaseLineHandler::SplitComplexType(src_type, vec);
		if (vec.size() != 2)
		{
			return "";
		}
		string vec_elem = GET_TYPEDEFSET()->FindSrcTypeFromAlias(vec[1]);
		if (vec_elem.empty())
		{
			vec_elem = vec[1];
		}
		string as_type = get_as_type(vec_elem);
		return string("Vector.<") + as_type + string("> = new Vector.<") + as_type + string(">");
	}
	else
		return get_as_type(base_type);
}

void ASClassFunctions::code_generate(ofstream& o_file, const MemItemVec& mem_vec)
{
	func_pack(o_file, mem_vec);
	func_unpack(o_file, mem_vec);
}

void ASClassFunctions::func_pack(ofstream& o_file, const MemItemVec& mem_vec)
{
	o_file << "\t\tpublic function Pack(strm : ByteArray) : void" << endl
		   << "\t\t{" << endl
		   << "\t\t\tstrm.endian = Endian.BIG_ENDIAN;" << endl;

	for (MemItemVec::const_iterator itr = mem_vec.begin();
		itr != mem_vec.end(); ++itr)
	{
		generate_packcode(o_file, itr->m_type, itr->m_name, "\t\t\t");
	}

	o_file << "\t\t}" << endl;
}

void ASClassFunctions::func_unpack(ofstream& o_file, const MemItemVec& mem_vec)
{
	o_file << "\t\tpublic function UnPack(strm : ByteArray) : void" << endl
		   << "\t\t{" << endl;

	for (MemItemVec::const_iterator itr = mem_vec.begin();
		itr != mem_vec.end(); ++itr)
	{
		generate_unpackcode(o_file, itr->m_type, itr->m_name, "\t\t\t");
	}

	o_file << "\t\t}" << endl;
}

void ASClassFunctions::generate_packcode(ofstream& o_file, const string& type, const string& name, const string& space)
{
	if (type == "BIN_DATA")
	{
		o_file << space << "strm.writeUnsignedInt(" << name << ".length);" << endl
			   << space << "strm.writeBytes(" << name << ", 0, " << name << ".length);" << endl;
		return;
	}
	BaseType base_type = CBaseLineHandler::AnalysisValueType(type);
	
	switch (base_type)
	{
	case enm_type_obj:
		o_file << space << name << ".Pack(strm);" << endl;
		break;
	case enm_type_string:
		{
			packstring_code(o_file, name, space);
		}
		break;
	case enm_type_array:
		{
			packarray_code(o_file, type, name, space);
		}
		break;
	case enm_type_uint8:
	case enm_type_int8:
		{
			o_file << space << "strm.writeByte(" << name << ");" << endl;
		}
		break;
	case enm_type_uint16:
		o_file << space << "strm.writeShort(" << name << ");" << endl;
		break;
	case enm_type_uint32:
		o_file << space << "strm.writeUnsignedInt(" << name << ");" << endl;
		break;
	case enm_type_int16:
		o_file << space << "strm.writeShort(" << name << ");" << endl;
		break;
	case enm_type_int32:
		o_file << space << "strm.writeInt(" << name << ");" << endl;
		break;
	default:
		break;
	}
	o_file << endl;
}

void ASClassFunctions::packstring_code(ofstream& o_file, const string& name, const string& space)
{
	static string prefix = "a";
	string strm_name = prefix + "Bytes";
	prefix[0]++;

	o_file << space << "var " << strm_name << " : ByteArray = new ByteArray();" << endl
		<< space << strm_name << ".endian = Endian.BIG_ENDIAN;" << endl
		<< space << strm_name << ".writeUTFBytes(" << name << ");" << endl
		<< space << "strm.writeUnsignedInt(" << strm_name << ".length);" << endl
		<< space << "strm.writeBytes(" << strm_name << ", 0, " << strm_name << ".length);" 
		<< endl;
}

void ASClassFunctions::packarray_code(ofstream& o_file, const string& type, 
									  const string& name, const string& space)
{
	o_file << space << "strm.writeUnsignedInt(" << name << ".length);" << endl
		<< space << "for(var i : uint = 0; i < " << name << ".length; i+=1)" << endl
		<< space << "{" << endl;
	StringArray v_type;

	string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(type);
	if (src_type.empty())
	{
		src_type = type;
	}

	CBaseLineHandler::SplitComplexType(src_type, v_type);
	if (v_type.size() != 2)
	{
		return;
	}
	BaseType src_btype = CBaseLineHandler::AnalysisValueType(v_type[1]);
	if (src_btype == enm_type_array)
	{
		return;
	}
	else
	{
		generate_packcode(o_file, v_type[1], name + "[i]", "\t\t\t\t");
	}
	o_file << space << "}" << endl;
}

void ASClassFunctions::generate_unpackcode(ofstream& o_file, const string& type, 
										   const string& name, const string& space)
{
	if (type == "BIN_DATA")
	{
		o_file << space << "var " << name << "_len : uint = strm.readUnsignedInt();" << endl
			   << space << "if (" << name << "_len > 0)" << endl
			   << space << "{"
			   << space << "\tstrm.readBytes(" << name << ", 0, " << name << "_len);" << endl
			   << space << "}" << endl;
		return;
	}
	BaseType base_type = CBaseLineHandler::AnalysisValueType(type);

	switch (base_type)
	{
	case enm_type_obj:
		o_file << space << name << ".UnPack(strm);" << endl;
		break;
	case enm_type_string:
		{
			unpackstring_code(o_file, name, space);
		}
		break;
	case enm_type_array:
		{
			unpackarray_code(o_file, type, name, space);
		}
		break;
	case enm_type_uint8:
	case enm_type_int8:
		{
			o_file << space << name << " = " << "strm.readByte();" << endl;
		}
		break;
	case enm_type_uint16:
		o_file << space << name << " = " << "strm.readUnsignedShort();" << endl;
		break;
	case enm_type_uint32:
		o_file << space << name << " = " << "strm.readUnsignedInt();" << endl;
		break;
	case enm_type_int16:
		o_file << space << name << " = " << "strm.readShort();" << endl;
		break;
	case enm_type_int32:
		o_file << space << name << " = " << "strm.readInt();" << endl;
		break;
	default:
		break;
	}
	o_file << endl;
}

void ASClassFunctions::unpackstring_code(ofstream& o_file, const string& name, const string& space)
{
	static string prefix = "a";
	string byteslen = prefix + "BytesLen";
	string bytesname = prefix + "Bytes";
	prefix[0]++;

	o_file << space << "var " << byteslen << " : uint;" << endl
		   << space << byteslen << " = strm.readUnsignedInt();" << endl
		   << space << "if (" << byteslen << " > 0)" << endl
		   << space << "{" << endl
		   << space << "\tvar " << bytesname << " : ByteArray = new ByteArray();" << endl
		   << space << "\tstrm.readBytes(" << bytesname << ", 0, " << byteslen << ");" << endl
		   << space << "\t" << name << " = " << bytesname << ".readUTFBytes(" << byteslen << ");" << endl
		   << space << "}" << endl;
}

void ASClassFunctions::unpackarray_code(ofstream& o_file, const string& type, 
										const string& name, const string& space)
{
	static string prefix = "a";

	string vec_len = prefix + "VecLen";
	prefix[0]++;

	o_file << space << "var " << vec_len << " : uint;" << endl
		   << space << vec_len << " = strm.readUnsignedInt();" << endl
		   << space << "for(var i : uint = 0; i < " << vec_len << "; i+=1)" << endl
		   << space << "{" << endl;
	StringArray v_type;

	string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(type);
	if (src_type.empty())
	{
		src_type = type;
	}

	CBaseLineHandler::SplitComplexType(src_type, v_type);
	if (v_type.size() != 2)
	{
		return;
	}
	BaseType src_btype = CBaseLineHandler::AnalysisValueType(v_type[1]);
	if (src_btype == enm_type_array)
	{
		return;
	}
	else if (src_btype == enm_type_obj)
	{
		o_file << space << "\tvar " << name + "item" << " : " << get_as_type(v_type[1])
			   << " = new " << v_type[1] << ";" << endl;
		generate_unpackcode(o_file, v_type[1], name + "item", "\t\t\t\t");
		o_file << space << "\t" << name << ".push(" << (name + "item") << ");" << endl;
	}
	else
	{
		o_file << space << "\tvar " << (name + "_temp") << " : " << get_as_type(v_type[1]) << ";" << endl;
		generate_unpackcode(o_file, v_type[1], name + "_temp", "\t\t\t\t");
		o_file << space << "\t" << name << ".push(" << (name + "_temp") << ");" << endl;
	}
	o_file << space << "}" << endl;
}

string get_as_type(string& src_type)
{
	BaseType basetype = CBaseLineHandler::AnalysisValueType(src_type);
	
	switch (basetype)
	{
	case enm_type_uint8:
	case enm_type_uint16:
	case enm_type_uint32:
		return "uint";
		break;
	case enm_type_int8:
	case enm_type_int16:
	case enm_type_int32:
		return "int";
		break;
	case enm_type_string:
		return "String";
		break;
	default:
		return src_type;
		break;
	}
}

string get_as_type(BaseType base_type)
{
	switch (base_type)
	{
	case enm_type_uint8:
	case enm_type_uint16:
	case enm_type_uint32:
		return "uint";
		break;
	case enm_type_int8:
	case enm_type_int16:
	case enm_type_int32:
		return "int";
		break;
	case enm_type_string:
		return "String";
		break;
	default:
		return "";
		break;
	}
}
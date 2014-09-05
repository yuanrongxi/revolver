#ifndef JSON_STREAM_H__
#define JSON_STREAM_H__

#include <string>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "revolver/base_typedef.h"

#define BEGIN_JSON_NS namespace	JSON_NS{
#define END_JSON_NS	  }	

BEGIN_JSON_NS

#define JSON_SUCCESS	0
#define JSON_PARSE_ERR	1
#define JSON_DECODE_ERR	2

typedef std::string	JsonErr;

typedef rapidjson::SizeType													Json_Size_T;
typedef rapidjson::Value													Json_Value_T;
typedef rapidjson::Document													Document_T;
typedef Document_T::AllocatorType											Doc_Allocator_T;
typedef rapidjson::StringBuffer												String_Buf_T;
typedef rapidjson::Writer<String_Buf_T, rapidjson::UTF8<>, Doc_Allocator_T>	Writer_T;

class JsonInterface;

template<typename Allocator>
class JsonStream
{
public:
	void encode(const char* m_name, Json_Value_T& jobj, JsonInterface& t);
	void encode(const char* m_name, Json_Value_T& jobj, int32_t dest);
	void encode(const char* m_name, Json_Value_T& jobj, uint32_t dest);
	void encode(const char* m_name, Json_Value_T& jobj, int64_t dest);
	void encode(const char* m_name, Json_Value_T& jobj, uint64_t dest);
	void encode(const char* m_name, Json_Value_T& jobj, std::string& in);

	template<typename T>
	void encode(const char* m_name, Json_Value_T& jobj, std::vector<T>& list);

public:
	void decode(const char* m_name, Json_Value_T& jobj, JsonInterface& t);
	void decode(const char* m_name, Json_Value_T& jobj, int32_t& dest);
	void decode(const char* m_name, Json_Value_T& jobj, uint32_t& dest);
	void decode(const char* m_name, Json_Value_T& jobj, int64_t& dest);
	void decode(const char* m_name, Json_Value_T& jobj, uint64_t& dest);
	void decode(const char* m_name, Json_Value_T& jobj, std::string& out);

	template<typename T>
	void decode(const char* m_name, Json_Value_T& jobj, std::vector<T>& list);

public:
	JsonStream(Allocator* alloc)
	{
		assert(alloc != NULL);
		allocator = alloc;
	}

private:
	//禁止在vector中保存指针
	template<typename T>
	void encode(const char* m_name, Json_Value_T& jobj, std::vector<T*>& list);

	//禁止在vector中保存指针
	template<typename T>
	void decode(const char* m_name, Json_Value_T& jobj, std::vector<T*>& list);

	void convert_2_jsonvalue(Json_Value_T& value, int32_t t) {value = t;}
	void convert_2_jsonvalue(Json_Value_T& value, std::string& str)
	{
		value.SetString(str.c_str(), str.size());
	}
	
	void convert_2_jsonvalue(Json_Value_T& value, JsonInterface& json_obj);

	void jsonvalue_2_T(Json_Value_T& value, int32_t& t)
	{
		t = value.GetUint();
	}

	void jsonvalue_2_T(Json_Value_T& value, std::string& str)
	{
		str = value.GetString();
	}

	void jsonvalue_2_T(Json_Value_T& value, JsonInterface& json_obj);

private:
	Allocator*	allocator;
};

typedef JsonStream<Doc_Allocator_T>		Json_Strm_T;

class JsonInterface 
{
	friend class JsonStream<Doc_Allocator_T>;

public:
	virtual void	Serialize(std::string& out)
	{
		String_Buf_T strbuf;
		Doc_Allocator_T	allocator;
		Writer_T writer(strbuf, &allocator);

		Json_Strm_T jstrm(&allocator);

		Json_Value_T	ret_obj(rapidjson::kObjectType);
		try
		{
			encode(jstrm, ret_obj);
		}
		catch(...)
		{
			out.clear();
			return;
		}
		
		ret_obj.Accept(writer);

		out = strbuf.GetString();
	}

	virtual int32_t	UnSerialize(const std::string& in, std::string& error)//0 is success
	{
		Document_T	document;
		
		if (document.Parse<0>(in.c_str()).HasParseError())
		{
			error = "json obj has parse error";
			return JSON_PARSE_ERR;
		}

		Doc_Allocator_T	allocator;
		Json_Strm_T jstrm(&allocator);
		try
		{
			decode(jstrm, document);
		}
		catch(const JsonErr& err)
		{
			error = err;
			return JSON_DECODE_ERR;
		}
		return JSON_SUCCESS;
	}

protected:
	virtual Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj) = 0;
	virtual Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj) = 0;
};


template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, JsonInterface& t)
{
	Json_Value_T	t_obj(rapidjson::kObjectType);

	t.encode(*this, t_obj);

	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, t_obj, *allocator);
	}
	else 
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
}

template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, int32_t dest)
{
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, dest, *allocator);
	}
	else
	{
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, uint32_t dest)
{
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, dest, *allocator);
	}
	else
	{
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, int64_t dest)
{
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, dest, *allocator);
	}
	else
	{
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, uint64_t dest)
{
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, dest, *allocator);
	}
	else
	{
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, std::string& in)
{
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, in.c_str(), *allocator);
	}
	else
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
}

template<typename Allocator>
template<typename T>
void JsonStream<Allocator>::encode(const char* m_name, Json_Value_T& jobj, std::vector<T>& list)
{
	Json_Value_T	t_obj(rapidjson::kArrayType);

	for (typename std::vector<T>::size_type i = 0; i < list.size(); i++)
	{
		Json_Value_T t;
		convert_2_jsonvalue(t, list[i]);

		t_obj.PushBack(t, *allocator);
	}
	if (jobj.IsObject())
	{
		jobj.AddMember(m_name, t_obj, *allocator);
	}
	else
		throw JsonErr("encode error: jobj is not a obj, key is ") + m_name;
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, JsonInterface& t)
{
	if (!jobj.HasMember(m_name))
		throw JsonErr("json has not a member of ") + m_name;

	if (!jobj[m_name].IsObject())
		throw JsonErr("json wrong fomat object: ") + m_name;
	
	t.decode(*this, jobj[m_name]);
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, int32_t& dest)
{
	if (!jobj.HasMember(m_name))
	{
		throw JsonErr("json has not a member of ") + m_name;
		return ;
	}
	
	if (jobj[m_name].IsInt())
	{
		dest = jobj[m_name].GetInt();
	}
	else
	{
		dest = 0;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, uint32_t& dest)
{
	if (!jobj.HasMember(m_name))
	{
		throw JsonErr("json has not a member of ") + m_name;
		return;
	}

	if (jobj[m_name].IsUint())
	{
		dest = jobj[m_name].GetUint();
	}
	else
	{
		dest = 0;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, int64_t& dest)
{
	if (!jobj.HasMember(m_name))
	{
		throw JsonErr("json has not a member of ") + m_name;
		return;
	}

	if (jobj[m_name].IsInt64())
	{
		dest = jobj[m_name].GetInt64();
	}
	else
	{
		dest = 0;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, uint64_t& dest)
{
	if (!jobj.HasMember(m_name))
	{
		throw JsonErr("json has not a member of ") + m_name;
		return;
	}

	if (jobj[m_name].IsUint64())
	{
		dest = jobj[m_name].GetUint64();
	}
	else
	{
		dest = 0;
	}
}

template<typename Allocator>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, std::string& out)
{
	if (!jobj.HasMember(m_name))
	{
		//throw JsonErr("json has not a member of ") + m_name;
		return ;
	}
	
	if (jobj[m_name].IsString())
	{
		out = jobj[m_name].GetString();
	}
	else
		throw JsonErr(m_name) + " is not a string";
}

template<typename Allocator>
template<typename T>
void JsonStream<Allocator>::decode(const char* m_name, Json_Value_T& jobj, std::vector<T>& list)
{
	if (!jobj.HasMember(m_name))
		throw JsonErr("json has not a member of ") + m_name;
	
	if (jobj[m_name].IsArray())
	{
		T t;

		for (Json_Size_T i = 0; i < jobj[m_name].Size(); i++)
		{
			jsonvalue_2_T(jobj[m_name][i], t);
			list.push_back(t);
		}
	}
	else
		throw JsonErr(m_name) + " is not a array";
}

template<typename Allocator>
void JsonStream<Allocator>::convert_2_jsonvalue(Json_Value_T& value, JsonInterface& json_obj)
{
	value.SetObject();
	json_obj.encode(*this, value);
}

template<typename Allocator>
void JsonStream<Allocator>::jsonvalue_2_T(Json_Value_T& value, JsonInterface& json_obj)
{
	json_obj.decode(*this, value);
}

END_JSON_NS
#endif

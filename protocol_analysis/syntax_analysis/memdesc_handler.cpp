#include "memdesc_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "base_error.h"

CMemItem CMemDescHandler::CheckMemDesc(string& str_line, int line_num)
{
	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);

	if (v_elem.size() != 2 && v_elem.size() != 4)
	{
		THROW_ERROR(line_num, "Error:wrong form, expected 'type var_name [= [val]]'");
	}

	if (!(GET_DATATYPESET()->IsContainType(v_elem[0])))
	{
		THROW_ERROR(line_num, string("Error:type ") + v_elem[0] + "not defined");
	}

	if (!CBaseLineHandler::IsVarNameLegal(v_elem[1]))
	{
		THROW_ERROR(line_num, string("Error:") + v_elem[1] + "is bad name");
	}

	CMemItem mem_item;
	mem_item.m_type = v_elem[0];
	mem_item.m_name = v_elem[1];
	mem_item.m_value = NONE;

	if (v_elem.size() == 4)
	{
		if (v_elem[2] != "=")
		{
			THROW_ERROR(line_num, "Error:bad syntax, expected a '='");
		}
		mem_item.m_value = v_elem[3];
	}

	return mem_item;
}

bool CMemDescHandler::IsMemNameNotUsed(string& str_name, MemItemVec& v_mem, int line_num)
{
	for (MemItemVec::iterator ite = v_mem.begin();
		ite != v_mem.end(); ++ite)
	{
		if (ite->m_name == str_name)
		{
			THROW_ERROR(line_num, string("Error:Redefined var name ") + str_name);
		}
	}
	return true;
}
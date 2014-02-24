/*************************************************************************************
*filename:	base_nodes_load.h
*
*to do:		定义网络节点负载管理与选取算法
*Create on: 2012-04
*Author:	zerok
*check list:主要采用随机掉落选取算法，再通过2分查找法进行选取
*************************************************************************************/
#ifndef __BASE_NODES_LOAD_H
#define __BASE_NODES_LOAD_H

#include "base_namespace.h"
#include "base_typedef.h"

#include <map>
#include <set>
#include <vector>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

const int32_t MAX_LOAD_VALUE = 95;

typedef set<uint32_t>	SERVER_ID_SET;
typedef vector<uint32_t> SERVER_ID_ARRAY;

typedef struct NodeLoadInfo
{
	uint32_t	node_id;		//节点ID
	uint16_t	node_load;		//节点负载值，0到100,100表示负载最大

	NodeLoadInfo()
	{
		node_id = 0;
		node_load = 100;
	};
}NodeLoadInfo;



typedef map<uint32_t, NodeLoadInfo>	NodeLoadInfoMap;

typedef struct NodeRange
{
	int32_t		min_value;
	int32_t		max_value;
	uint32_t	node_id;

	NodeRange()
	{
		min_value = 0;
		max_value = 0;
		node_id = 0;
	};
}NodeRange;

typedef vector<NodeRange>	NodeRangeArray;

class CNodeLoadManager
{
public:
	CNodeLoadManager();
	~CNodeLoadManager();

	void			add_node(const NodeLoadInfo& info);
	void			update_node(const NodeLoadInfo& info);
	void			del_node(uint32_t node_id);

	uint32_t		select_node();
	uint32_t		select_node(const SERVER_ID_SET& ids);
	bool			select_node(NodeLoadInfo& info);
	uint32_t		size() const {return node_info_map_.size();};
private:
	uint32_t		locate_server(int32_t region);

private:
	NodeLoadInfoMap	node_info_map_;		//节点负载表
	NodeRangeArray	node_ranges_;		//一定周期的概率区间表
	
	bool			create_range_;		//是否需要重建概率选取表
	int32_t			region_;			//概率全区间
};

BASE_NAMESPACE_END_DECL
#endif

/************************************************************************************/

#include <stdlib.h>
#include "base_nodes_load.h"

BASE_NAMESPACE_BEGIN_DECL

CNodeLoadManager::CNodeLoadManager()
{
	create_range_ = true;
	region_ = 0;
}

CNodeLoadManager::~CNodeLoadManager()
{
	node_info_map_.clear();
	node_ranges_.clear();
}

void CNodeLoadManager::add_node(const NodeLoadInfo& info)
{
	NodeLoadInfoMap::iterator it = node_info_map_.find(info.node_id);
	if(it != node_info_map_.end())
	{
		it->second = info;
	}
	else
	{
		node_info_map_.insert(NodeLoadInfoMap::value_type(info.node_id, info));
	}

	create_range_ = true;
}

void CNodeLoadManager::del_node(uint32_t node_id)
{
	node_info_map_.erase(node_id);
	create_range_ = true;
}

void CNodeLoadManager::update_node(const NodeLoadInfo& info)
{
	add_node(info);
}

uint32_t CNodeLoadManager::locate_server(int32_t region)
{
	uint32_t ret = 0;

	int32_t seed = rand() % region;
	
	int32_t ranges_size = node_ranges_.size();

	int32_t max_pos = ranges_size;
	int32_t min_pos = 0;
	int32_t pos = 0;

	//2分法定位选取的节点
	do 
	{
		pos = (min_pos + max_pos) / 2;
		if(pos > ranges_size - 1)
		{
			ret = node_ranges_[ranges_size - 1].node_id;
			break;
		}

		if(pos < 0)
		{
			ret = node_ranges_[0].node_id;
			break;
		}

		if(node_ranges_[pos].max_value >= seed && node_ranges_[pos].min_value <= seed)
		{
			ret = node_ranges_[pos].node_id;
			break;
		}
		else if(node_ranges_[pos].max_value < seed)
		{
			min_pos = pos + 1;
		}
		else if(node_ranges_[pos].min_value > seed)
		{
			max_pos = pos - 1;
		}
	} while (true);

	return ret;
}

uint32_t CNodeLoadManager::select_node()
{
	uint32_t ret = 0;

	if(create_range_)
	{
		node_ranges_.clear();
		region_ = 0;

		int32_t interval = 0;
		NodeRange range;
		for(NodeLoadInfoMap::iterator it = node_info_map_.begin(); it != node_info_map_.end(); ++it)
		{
			if(it->second.node_load <= MAX_LOAD_VALUE) //计算负载区间
			{
				interval = MAX_LOAD_VALUE - it->second.node_load + 1;

				range.node_id = it->second.node_id;
				range.min_value = region_;
				region_ += interval;
				range.max_value = region_ - 1;

				node_ranges_.push_back(range);
			}
		}

		create_range_ = false;
	}

	if(region_ > 0) //随机概率选取
	{
		ret = locate_server(region_);
	}

	return ret;
}

uint32_t CNodeLoadManager::select_node(const SERVER_ID_SET& ids)
{
	uint32_t ret = 0;

	node_ranges_.clear();
	region_ = 0;

	int32_t interval = 0;
	NodeRange range;
	for(NodeLoadInfoMap::iterator it = node_info_map_.begin(); it != node_info_map_.end(); ++it)
	{
		SERVER_ID_SET::const_iterator set_it = ids.find(it->second.node_id);
		if(set_it != ids.end())
			continue;

		if(it->second.node_load <= MAX_LOAD_VALUE) //计算负载区间
		{
			interval = MAX_LOAD_VALUE - it->second.node_load + 1;

			range.node_id = it->second.node_id;
			range.min_value = region_;
			region_ += interval;
			range.max_value = region_ - 1;

			node_ranges_.push_back(range);
		}
	}

	create_range_ = false;

	if(region_ > 0) //随机概率选取
	{
		ret = locate_server(region_);
	}

	return ret;
}

bool CNodeLoadManager::select_node(NodeLoadInfo& info)
{
	uint32_t node_id = select_node();
	if(node_id <= 0)
	{
		info.node_id = node_id;
		return false;
	}

	NodeLoadInfoMap::iterator it = node_info_map_.find(node_id);
	if(it != node_info_map_.end())
	{
		info = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

BASE_NAMESPACE_END_DECL


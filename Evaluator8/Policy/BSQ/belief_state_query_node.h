#ifndef BELIEF_STATE_QUERY_NODE_EVAL8
#define BELIEF_STATE_QUERY_NODE_EVAL8

#include "../../query_instance.h"
#include<vector>

struct BSQNode {
	BSQNode();
	BSQNode(const BSQNode&) = delete;
	BSQNode operator= (const BSQNode&) = delete;
	~BSQNode();
	bool is_and = true;
	bool is_satisfiable = true;
	bool has_value = false;
	BSQNode* parent = nullptr;
	std::vector< BSQNode*> children;
	QueryInstance* instance_check = nullptr;
};

#endif
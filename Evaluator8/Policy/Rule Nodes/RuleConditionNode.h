#ifndef EVAL8_RULE_CONDITION_NODE
#define EVAL8_RULE_CONDITION_NODE

#include<vector>
#include "parameter_value_inequality.h"

struct RuleConditionNode {
	RuleConditionNode();
	RuleConditionNode(const BSQNode&) = delete;
	RuleConditionNode operator= (const RuleConditionNode&) = delete;
	~RuleConditionNode();
	bool is_and = true;
	bool is_satisfiable = true;
	RuleConditionNode* parent = nullptr;
	std::vector< RuleConditionNode*> children;
	bool has_value = false;
	BeliefStateQuery* belief_state_query = nullptr;
	QueryInstance* fully_observable_query_instance = nullptr;
	ParamValIneq* parameter_val_inequality = nullptr;
	OrthotopeList scratchwork;
};

#endif
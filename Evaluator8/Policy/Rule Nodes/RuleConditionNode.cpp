#include "RuleConditionNode.h"

RuleConditionNode::RuleConditionNode()
{

}

RuleConditionNode::~RuleConditionNode()
{
	if (belief_state_query) {
		delete belief_state_query;
		belief_state_query = nullptr;
	}
	if (fully_observable_query_instance) {
		delete fully_observable_query_instance;
		fully_observable_query_instance = nullptr;
	}
	if (parameter_val_inequality) {
		delete parameter_val_inequality;
		parameter_val_inequality = nullptr;
	}
	for (std::vector< RuleConditionNode*>::iterator it = children.begin(); it != children.end(); ++it) {
		delete* it;
		*it = nullptr;
	}
	children.clear();
	parent = nullptr;
}

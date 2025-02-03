#ifndef EVAL8_RULE_H
#define EVAL8_RULE_H

#include "Rule Nodes/RuleConditionNode.h"
#include "../action.h"

struct PolicyRule {
	PolicyRule(std::string&, const Problem&, const std::map<std::string, ParameterInfo>&);
	PolicyRule(const PolicyRule&);
	~PolicyRule();
	PolicyRule& operator=(const PolicyRule&);
	void update_constraint(const StaticState&, const std::list<DynamicState>&, const std::list<float>&, OrthotopeList&) const;
	void print() const;
	void print(unsigned int) const;

	RuleConditionNode* root = nullptr;
	Action rule_action;
};

#endif
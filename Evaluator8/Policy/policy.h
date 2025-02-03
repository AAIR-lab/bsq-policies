#ifndef EVAL8_POLICY_H
#define EVAL8_POLICY_H

#include <list>
#include "rule.h"

//TODO: eventually extend policy to FSC and non-deterministic actions.

class Policy{
public:
    Policy(const std::string&, const Problem&);
    Policy(const Policy&);
    void build_policy_distribution(const StaticState&, const std::list<DynamicState>&, const std::list<float>&, std::list<OrthotopeList>&, std::list<Action>&, Action&) const;
    void get_parameter_info(std::map<std::string, ParameterInfo>&) const;
    void get_full_parameter_range(OrthotopeList&) const;
    void print();
    void select_action(const StaticState&, const std::list<DynamicState>&, const std::list<float>&, const std::vector<float>&, Action&);
private:
    OrthotopeList full_parameter_range;
    std::map<std::string, ParameterInfo> parameter_information;
    std::vector<PolicyRule> policy_rules;
    Action else_action;
};

#endif
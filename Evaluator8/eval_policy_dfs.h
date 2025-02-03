#ifndef EVAL7_EVAL_POLICY_DFS_H
#define EVAL7_EVAL_POLICY_DFS_H

#include "belief_state.h"
#include "Policy/policy.h"
#include "orthotope_list.h"


float evalPolicyDFS(BeliefState*, const Policy&, const Problem&, const std::vector<float>&, unsigned int, float,
    OrthotopeList&);

float evalPolicyDFS(BeliefState*, const Policy&, const Problem&, const std::vector<float>&, unsigned int, float);

void evalPolicyDFS_multi(BeliefState*, const Policy&, const Problem&, const std::vector<float>&, unsigned int, float,
    OrthotopeList&, std::list<float>&, float&, unsigned int);

#endif

#ifndef EVAL7_POLICY_ANALYSIS_H
#define EVAL7_POLICY_ANALYSIS_H

#include "../belief_state.h"
#include "../Policy/policy.h"
#include "../orthotope_list.h"

#include <mutex>

void parse_results_file(const std::string&, std::vector<std::vector<std::vector<float>>>&);
void policy_analysis(BeliefState*, const Policy&, const Problem&, const std::string&, unsigned int, float, unsigned int, std::string, unsigned int);

void policy_analysis_parrallel(BeliefState*, const Policy&, const Problem&, const std::string&, unsigned int, float, unsigned int, std::string, unsigned int, unsigned int);
void random_policy_analysis_parrallel(BeliefState*, const Policy&, const Problem&, unsigned int, float, unsigned int, std::string, unsigned int, unsigned int, unsigned int);
void policy_analysis_worker(BeliefState*, const Policy*, const Problem*, unsigned int, float, int, std::mutex*, std::mutex*, std::list<DynamicState>*, std::list<float>*, std::list<float>*, std::list<float>*, bool*, std::vector<std::vector<float>>*);
#endif
#ifndef COMPLIANCE_ANALYSIS_EVAL8
#define COMPLIANCE_ANALYSIS_EVAL8

#include "../Policy/policy.h"
#include "../belief_state.h"
#include "../domain.h"
#include "../problem.h"

void read_in_space_actions(const std::string& path, std::vector<std::list<Action>> &output_action);
void read_in_city_actions(const std::string& path, std::vector<std::list<Action>>& output_action);
void read_in_rock_actions(const std::string& path, std::vector<std::list<Action>>& output_action);
void read_in_space_observations(const std::string& path, std::vector<std::list<Observation>> &output_observations);
void read_in_city_observations(const std::string& path, std::vector<std::list<Observation>>& output_observations);
void read_in_rock_observations(const std::string& path, std::vector<std::list<Observation>>& output_observations);
bool trajectory_was_compliant(BeliefState&, const Problem&, Policy&, const std::list<Action>&, const std::list<Observation>&, const OrthotopeList&);

#endif
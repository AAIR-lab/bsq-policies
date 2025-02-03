#include "compliance_analysis.h"
#include <sstream>
#include <iostream>

inline void find_orthotope_for_rule(const std::list<OrthotopeList>& rules, const std::list<Action> actions, const Action & other_action, const OrthotopeList &parameter_interval, const Action& target, OrthotopeList &output) {
	bool first_occurrance = true;
	std::list<OrthotopeList>::const_iterator rule_it = rules.cbegin();
	std::list<Action>::const_iterator action_it = actions.cbegin();
	OrthotopeList uncovered_interval = parameter_interval, temp_interval;
	output.clear();
	for (; rule_it != rules.cend(); ++rule_it, ++action_it) {
		if (!rule_it->is_satisfiable()) {
			continue;
		}
		if (target == *action_it) {
			if (first_occurrance) {
				output = *rule_it;
				output.and_update(uncovered_interval);
				first_occurrance = false;
			}
			else {
				temp_interval = *rule_it;
				temp_interval.and_update(uncovered_interval);
				output.or_update(uncovered_interval);
			}
		}
		uncovered_interval.not_update(*rule_it);
	}
	if (other_action == target) {
		
		if (first_occurrance) {
			output = uncovered_interval;
		}
		else {
			output.or_update(uncovered_interval);
		}
	}
}

void read_in_space_actions(const std::string& path, std::vector<std::list<Action>> &output_action)
{
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_action.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_action.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_action[current_trajectory].emplace_back();
			if (word == "fix_robot") {
				output_action[current_trajectory].back().action_name = "fix";
				output_action[current_trajectory].back().integer_params.emplace_back(0);
			}
			else if (word == "fix_ship") {
				output_action[current_trajectory].back().action_name = "fix";
				output_action[current_trajectory].back().integer_params.emplace_back(1);
			}
			else {
				output_action[current_trajectory].back().action_name = "wait";
			}
		}
		++current_trajectory;
	}
	input_file.close();
}

void read_in_city_actions(const std::string& path, std::vector<std::list<Action>>& output_action)
{
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_action.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_action.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_action[current_trajectory].emplace_back();
			if (word == "North") {
				output_action[current_trajectory].back().action_name = "Up";
			}
			else if (word == "South") {
				output_action[current_trajectory].back().action_name = "Down";
			}
			else if (word == "West") {
				output_action[current_trajectory].back().action_name = "Left";
			}
			else if (word == "East") {
				output_action[current_trajectory].back().action_name = "Right";
			}
			else if (word == "Scan") {
				output_action[current_trajectory].back().action_name = "Scan";
			}
			else {
				output_action[current_trajectory].back().action_name = "Visit";
			}
		}
		++current_trajectory;
	}
	input_file.close();
}

void read_in_rock_actions(const std::string& path, std::vector<std::list<Action>>& output_action)
{
	std::cout << path << std::endl;
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_action.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_action.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_action[current_trajectory].emplace_back();
			if (word == "goto_start") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(0);
			}
			else if (word == "goto_r1") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(1);
			}
			else if (word == "goto_r2") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(2);
			}
			else if (word == "goto_r3") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(3);
			}
			else if (word == "goto_r4") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(4);
			}
			else if (word == "goto_m1") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(5);
			}
			else if (word == "goto_m2") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(6);
			}
			else if (word == "goto_dropoff") {
				output_action[current_trajectory].back().action_name = "goto";
				output_action[current_trajectory].back().integer_params.push_back(7);
			}
			else if (word == "scan_r1") {
				output_action[current_trajectory].back().action_name = "scan";
				output_action[current_trajectory].back().integer_params.push_back(1);
			}
			else if (word == "scan_r2") {
				output_action[current_trajectory].back().action_name = "scan";
				output_action[current_trajectory].back().integer_params.push_back(2);
			}
			else if (word == "scan_r3") {
				output_action[current_trajectory].back().action_name = "scan";
				output_action[current_trajectory].back().integer_params.push_back(3);
			}
			else if (word == "scan_r4") {
				output_action[current_trajectory].back().action_name = "scan";
				output_action[current_trajectory].back().integer_params.push_back(4);
			}
			else if (word == "sample") {
				output_action[current_trajectory].back().action_name = "sample";
			}
			else {
				std::cout << word << std::endl;
				break;
			}
		}
		++current_trajectory;
	}
	input_file.close();
}

void read_in_space_observations(const std::string& path, std::vector<std::list<Observation>> &output_observations)
{
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_observations.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_observations.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_observations[current_trajectory].emplace_back();
			if (word == "broken_broken") {
				output_observations[current_trajectory].back().bool_params.push_back(true);
				output_observations[current_trajectory].back().bool_params.push_back(true);
			}
			else if (word == "broken_normal") {
				output_observations[current_trajectory].back().bool_params.push_back(true);
				output_observations[current_trajectory].back().bool_params.push_back(false);
			}
			else if (word == "normal_broken") {
				output_observations[current_trajectory].back().bool_params.push_back(false);
				output_observations[current_trajectory].back().bool_params.push_back(true);
			}
			else {
				output_observations[current_trajectory].back().bool_params.push_back(false);
				output_observations[current_trajectory].back().bool_params.push_back(false);
			}
		}
		++current_trajectory;
	}
	input_file.close();
}

void read_in_city_observations(const std::string& path, std::vector<std::list<Observation>>& output_observations)
{
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_observations.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_observations.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_observations[current_trajectory].emplace_back();
			if (word == "None") {
				continue;
			}
			output_observations[current_trajectory].back().integer_params.push_back(int(word[0] - 48) + (6 * (int(word[2]) - 48)));
		}
		++current_trajectory;
	}
	input_file.close();
}

void read_in_rock_observations(const std::string& path, std::vector<std::list<Observation>>& output_observations)
{
	std::ifstream input_file;
	input_file.open(path);
	std::string line, word;
	output_observations.clear();
	int current_trajectory = 0;
	while (std::getline(input_file, line)) {
		output_observations.emplace_back();
		std::istringstream current_stream(line);
		while (std::getline(current_stream, word, ' ')) {
			output_observations[current_trajectory].emplace_back();
			if (word == "obs_worth") {
				output_observations[current_trajectory].back().bool_params.push_back(true);
			}
			else if (word == "obs_not") {
				output_observations[current_trajectory].back().bool_params.push_back(false);
			}
		}
		++current_trajectory;
	}
	input_file.close();
}

bool trajectory_was_compliant(BeliefState &belief_state, const Problem &problem_ref, 
	Policy &policy_ref, const std::list<Action> &trajectory_actions, const std::list<Observation> &trajectory_observations,
	const OrthotopeList &parameter_intervals)
{
	belief_state.build_initial_belief(problem_ref);
	std::list<Action>::const_iterator current_action = trajectory_actions.cbegin();
	std::list<Observation>::const_iterator current_observation = trajectory_observations.cbegin();
	
	std::list<OrthotopeList> rule_list;
	std::list<Action> action_list;
	Action other_action;

	OrthotopeList trajectory_interval, rule_interval;
	policy_ref.build_policy_distribution(belief_state.static_state, belief_state.state_list, belief_state.probabilities, rule_list, action_list, other_action);
	find_orthotope_for_rule(rule_list, action_list, other_action, parameter_intervals, *current_action, trajectory_interval);
	belief_state.apply_action(*current_action);
	belief_state.apply_observation(*current_action,*current_observation);
	++current_action;
	++current_observation;
	std::cout << trajectory_interval << std::endl;
	while (current_action != trajectory_actions.end() && trajectory_interval.is_satisfiable()) {
		if (belief_state.state_list.empty()) {
			return false;
		}
		policy_ref.build_policy_distribution(belief_state.static_state, belief_state.state_list, belief_state.probabilities, rule_list, action_list, other_action);
		//trajectory_interval.clear();
		find_orthotope_for_rule(rule_list, action_list, other_action, parameter_intervals, *current_action, rule_interval);
		trajectory_interval.and_update(rule_interval);
		belief_state.apply_action(*current_action);
		belief_state.apply_observation(*current_action, *current_observation);
		++current_action;
		++current_observation;
		std::cout << trajectory_interval << std::endl;
	}

	std::cout << trajectory_interval.is_satisfiable() << std::endl;
	return trajectory_interval.is_satisfiable();

}
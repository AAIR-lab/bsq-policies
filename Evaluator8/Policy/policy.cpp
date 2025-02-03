#include "policy.h"
#include <fstream>
#include <stdexcept>

Policy::Policy(const std::string& file_name, const Problem& problem_ref)
{
    std::ifstream input_file(file_name);
    std::string current_line, else_action_name, parameter_name;
    size_t found_index;
    bool started_rules = false, ended_rules = false;
    unsigned int found_parameters = 0u;
    std::map<std::string, action_function_data>::const_iterator found_action;
    Orthotope temp_orthotope;
    while (std::getline(input_file, current_line)) {
        if (current_line.starts_with("//")) {
            continue;
        }
        if (current_line.size() < 2) {
            continue;
        }
        if (ended_rules) {
            throw std::invalid_argument("Else rule already found in the file above.");
        }
        if (started_rules) {
            if (current_line.starts_with("if ")) {
                throw std::invalid_argument("Different if-else blocks currently not supported.");
            }
            else if (current_line.starts_with("elif ")) {
                policy_rules.emplace_back(current_line, problem_ref, parameter_information);
            }
            else if (current_line.starts_with("else ")) {
                current_line.erase(0, 5);
                found_index = current_line.find_first_of('(');
                if (found_index == std::string::npos) {
                    throw std::invalid_argument("Failed to parse policy: else action");
                }
                else_action_name = current_line.substr(0, found_index);
                found_action = problem_ref.action_information.find(else_action_name);
                if (found_action == problem_ref.action_information.end()) {
                    throw std::invalid_argument("Invalid action name: " + else_action_name);
                }
                found_action->second.build_action(current_line, else_action);
                ended_rules = true;
            }
            else {
                throw std::invalid_argument("Unrecognize parse line.");
            }
        }
        else if (current_line.starts_with("if ")) {
            started_rules = true;
            policy_rules.emplace_back(current_line, problem_ref, parameter_information);
        }
        else {
            found_index = current_line.find_first_of(" = ");
            if (found_index == std::string::npos) {
                throw std::invalid_argument("Unrecognize parse line.");
            }
            parameter_name = current_line.substr(0, found_index);
            parameter_information[parameter_name].parameter_name = parameter_name;
            parameter_information[parameter_name].parameter_index = found_parameters;
            ++found_parameters;
            current_line.erase(0, found_index + 3);
            if (current_line[0] == '[') {
                parameter_information[parameter_name].include_lower = true;
            }
            else if (current_line[0] == '(') {
                parameter_information[parameter_name].include_lower = false;
            }
            else {
                throw std::invalid_argument("Failed to parse lower parameter range");
            }
            current_line.erase(0, 1);
            found_index = current_line.find_first_of(",");
            if (found_index == std::string::npos) {
                throw std::invalid_argument("Parameter range in wrong format.");
            }
            parameter_information[parameter_name].lower_bound = std::stof(current_line.substr(0, found_index));
            current_line.erase(0, found_index + 1);
            if (current_line.back() == ']') {
                parameter_information[parameter_name].include_upper = true;
            }
            else if (current_line.back() == ')') {
                parameter_information[parameter_name].include_lower = true;
            }
            else {
                throw std::invalid_argument("Failed to parse upper parameter range");
            }
            current_line.pop_back();
            parameter_information[parameter_name].upper_bound = std::stof(current_line);
            temp_orthotope.dimensions.emplace_back();
            temp_orthotope.dimensions.back().include_lower = parameter_information[parameter_name].include_lower;
            temp_orthotope.dimensions.back().include_upper = parameter_information[parameter_name].include_upper;
            temp_orthotope.dimensions.back().lower_bound = parameter_information[parameter_name].lower_bound;
            temp_orthotope.dimensions.back().upper_bound = parameter_information[parameter_name].upper_bound;
        }
    }
    temp_orthotope.update_volume();
    if (!temp_orthotope.is_satisfiable()) {
        throw std::invalid_argument("Given parameter ranges are unsatisfiable.");
    }
    full_parameter_range.or_update(temp_orthotope);
}

Policy::Policy(const Policy &ref):full_parameter_range(ref.full_parameter_range),parameter_information(ref.parameter_information),policy_rules(ref.policy_rules),else_action(ref.else_action)
{

}

void Policy::get_full_parameter_range(OrthotopeList& out) const
{
    out = full_parameter_range;
}

void Policy::get_parameter_info(std::map<std::string, ParameterInfo> &out) const
{
    out = parameter_information;
}

void Policy::build_policy_distribution(const StaticState &static_ref, const std::list<DynamicState> &dynamic_ref, const std::list<float> &prob_ref, std::list<OrthotopeList> &out_constraints, std::list<Action> &out_actions, Action &out_else_action) const
{
    out_else_action = else_action;
    out_constraints.clear();
    out_actions.clear();
    for (std::vector<PolicyRule>::const_iterator it = policy_rules.cbegin(); it != policy_rules.cend(); ++it) {
        out_constraints.emplace_back(full_parameter_range);
        it->update_constraint(static_ref, dynamic_ref, prob_ref, out_constraints.back());
        out_actions.emplace_back(it->rule_action);
    }
}

void Policy::select_action(const StaticState& static_state, const std::list<DynamicState>& belief_states, const std::list<float>& belief_probs,
    const std::vector<float>& parameter_values, Action& out) {

    std::list<OrthotopeList> constraint_dist;
    std::list<Action> action_dist;
    Action otherwise_action;
    build_policy_distribution(static_state, belief_states, belief_probs, constraint_dist, action_dist, otherwise_action);

    std::list<OrthotopeList>::iterator constraint_it = constraint_dist.begin();
    std::list<Action>::iterator action_it = action_dist.begin();

    for (; constraint_it != constraint_dist.end() && action_it != action_dist.end(); ++constraint_it, ++action_it) {
        if (constraint_it->is_satisfied(parameter_values)) {
            out = *action_it;
            return;
        }
    }
    out = otherwise_action;
}

void Policy::print()
{

}
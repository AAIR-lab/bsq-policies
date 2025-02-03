#include "belief_state_query.h"
#include "../../utility.h"
#include <stdexcept>
#include <string>
#include <string.h>
#include <iostream>

BeliefStateQuery::BeliefStateQuery(std::string &parse_target, const Problem &problem_ref, const std::map<std::string, ParameterInfo> &parameter_ref): root(nullptr),comparison(eq),no_parameter(true),
	query_value(0.0f),parameter_index(0u)
{
	if (parse_target.size() < 7) {
		throw std::invalid_argument("BSQ \"" + parse_target + "\" is incorrect format.");
	}
	//The input will take the form P[...] o x where o is an inequality operator and x is either a float or a parameter.
	//Ignores "P[" and start at the 2nd index
	parse_target.erase(0, 2);

	parse_ineq(parse_target, problem_ref, parameter_ref);

	root = new BSQNode();

	BSQNode* current_position = root, *temp_ptr = nullptr;
	size_t found_index;
	std::string sub_str;
	std::map<std::string, function_data>::const_iterator found_function;
	std::vector<BSQNode*>::iterator children_it;
	
	while (!parse_target.empty()) {
		if (!current_position) {
			throw std::invalid_argument("Failed to parse.");
		}
		if (parse_target[0] == ',') {
			if (current_position->children.empty()) {
				throw std::invalid_argument("Misplaced comma.");
			}
			current_position->children.push_back(new BSQNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			parse_target.erase(0, 1);
		}
		else if (parse_target[0] == ')') {
			if (current_position->children.empty()) {
				throw std::invalid_argument("Misplaced closing paranthesis.");
			}
			children_it = current_position->children.begin();
			if (current_position->is_and) {
				while (children_it != current_position->children.end()) {
					if ((*children_it)->has_value) {
						if ((*children_it)->is_satisfiable) {
							delete* children_it;
							children_it = current_position->children.erase(children_it);
						}
						else {
							current_position->has_value = true;
							current_position->is_satisfiable = false;
							break;
						}
					}
					else {
						//TODO: add check for a child and to combine.
						++children_it;
					}
				}
				if (current_position->children.empty() && !current_position->has_value) {
					current_position->has_value = true;
					current_position->is_satisfiable = true;
				}
			}
			else {
				while (children_it != current_position->children.end()) {
					if ((*children_it)->has_value) {
						if ((*children_it)->is_satisfiable) {
							current_position->has_value = true;
							current_position->is_satisfiable = true;
							break;
						}
						else {
							delete* children_it;
							children_it = current_position->children.erase(children_it);
						}
					}
					else {
						//TODO: add check for a child or to combine.
						++children_it;
					}
				}
				if (current_position->children.empty() && !current_position->has_value) {
					current_position->has_value = true;
					current_position->is_satisfiable = false;
				}
			}
			if (current_position->children.size() == 1 && current_position->parent) {
				temp_ptr = current_position->children.back();
				temp_ptr->parent = current_position->parent;
				current_position->parent->children.back() = temp_ptr;
				current_position->children.clear();
				delete current_position;
				current_position = temp_ptr;
			}
			current_position = current_position->parent;
			parse_target.erase(0, 1);
		}
		else if (parse_target.starts_with("(AND ")) {
			if (!current_position->children.empty()) {
				throw std::invalid_argument("Missing comma.");
			}
			current_position->is_and = true;
			current_position->children.push_back(new BSQNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			parse_target.erase(0, 5);
		}
		else if (parse_target.starts_with("(OR ")) {
			current_position->is_and = false;
			current_position->children.push_back(new BSQNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			parse_target.erase(0, 4);
		}
		else {
			//Only legal option is a function [function_name](p1,...,pn)
			found_index = parse_target.find_first_of('(');
			if (found_index == std::string::npos) {
				throw std::invalid_argument("Start of parse string unrecognizable.");
			}
			sub_str = parse_target.substr(0, found_index);
			found_function = problem_ref.function_information.find(sub_str);
			if (found_function == problem_ref.function_information.end()) {
				throw std::invalid_argument("Function \"" + sub_str + "\" isn't a define function.");
			}
			found_index = parse_target.find_first_of(')');
			if (found_index == std::string::npos) {
				throw std::invalid_argument("Failed to find closing paranthesis.");
			}
			sub_str = parse_target.substr(0, found_index + 1);
			current_position->instance_check = found_function->second.build_instance(sub_str, problem_ref);
			if (!current_position->instance_check) {
				throw std::invalid_argument("Failed to parse function " + sub_str + ".");
			}
			if (!found_function->second.dynamic) {
				current_position->has_value = true;
				current_position->is_satisfiable = current_position->instance_check->evaluate(problem_ref.actual_static_state);
			}
			parse_target.erase(0, found_index + 1);
			current_position = current_position->parent;
		}
	}
	if (current_position) {
		throw std::invalid_argument("Failed to parse: early exit.");
	}
}
BeliefStateQuery::BeliefStateQuery(const BeliefStateQuery &ref) : root(nullptr), comparison(ref.comparison), no_parameter(ref.no_parameter), query_value(ref.query_value), parameter_index(ref.parameter_index)
{
	if (!ref.root) {
		return;
	}
	root = new BSQNode;
	std::list<std::pair<BSQNode*, BSQNode const*>> copy_stack;
	copy_stack.emplace_back(root, ref.root);
	BSQNode* current = nullptr;
	BSQNode const* ref_current = nullptr;
	while (!copy_stack.empty()) {
		current = copy_stack.back().first;
		ref_current = copy_stack.back().second;
		copy_stack.pop_back();
		current->has_value = ref_current->has_value;
		current->is_and = ref_current->is_and;
		current->is_satisfiable = ref_current->is_satisfiable;
		if (ref_current->instance_check) {
			current->instance_check = ref_current->instance_check->create_copy();
		}
		if (!ref_current->children.empty()) {
			for (std::vector<BSQNode*>::const_iterator it = ref_current->children.begin(); it != ref_current->children.end(); ++it) {
				current->children.push_back(new BSQNode);
				current->children.back()->parent = current;
				copy_stack.emplace_back(current->children.back(), *it);
			}
		}
	}
}

BeliefStateQuery::~BeliefStateQuery()
{
	if (!root) {
		return;
	}
	delete root;
	root = nullptr;
}

BeliefStateQuery& BeliefStateQuery::operator=(const BeliefStateQuery &ref)
{
	if (this == &ref) {
		return *this;
	}
	comparison = ref.comparison;
	no_parameter = ref.no_parameter;
	query_value = ref.query_value;
	parameter_index = ref.parameter_index;
	if (root) {
		delete root;
		root = nullptr;
	}
	if (!ref.root) {
		return *this;
	}
	root = new BSQNode;
	std::list<std::pair<BSQNode*, BSQNode const*>> copy_stack;
	copy_stack.emplace_back(root, ref.root);
	BSQNode* current = nullptr;
	BSQNode const* ref_current = nullptr;
	while (!copy_stack.empty()) {
		current = copy_stack.back().first;
		ref_current = copy_stack.back().second;
		copy_stack.pop_back();
		current->has_value = ref_current->has_value;
		current->is_and = ref_current->is_and;
		current->is_satisfiable = ref_current->is_satisfiable;
		if (ref_current->instance_check) {
			current->instance_check = ref_current->instance_check->create_copy();
		}
		if (!ref_current->children.empty()) {
			for (std::vector<BSQNode*>::const_iterator it = ref_current->children.begin(); it != ref_current->children.end(); ++it) {
				current->children.push_back(new BSQNode);
				current->children.back()->parent = current;
				copy_stack.emplace_back(current->children.back(), *it);
			}
		}
	}
	return *this;
}

void BeliefStateQuery::print() const
{
	print(0u);
}

void BeliefStateQuery::print(unsigned int current_offset) const
{
	if (!root) {
		return;
	}
	std::list<std::pair<BSQNode*, unsigned int>> print_stack;
	print_stack.emplace_back(root, current_offset);
	BSQNode* current = nullptr;
	while (!print_stack.empty()) {
		current = print_stack.back().first;
		current_offset = print_stack.back().second;
		print_stack.pop_back();
		for (unsigned int i = 0u; i < current_offset; ++i) {
			std::cout << " ";
		}
		if (current->children.empty()) {
			std::cout << "Instance(" << current->instance_check << "): " << current->instance_check->dynamic << " " << current->instance_check->full_observable << " " << current->instance_check->negated << " " << current->instance_check->target_index << " ";
			current->instance_check->print_value();
			std::cout << std::endl;
		}
		else if(current->is_and){
			for (std::vector<BSQNode*>::const_reverse_iterator it = current->children.crbegin(); it != current->children.crend(); ++it) {
				print_stack.emplace_back(*it, current_offset + 1u);
			}
			std::cout << "AND:\n";
		}
		else {
			for (std::vector<BSQNode*>::const_reverse_iterator it = current->children.crbegin(); it != current->children.crend(); ++it) {
				print_stack.emplace_back(*it, current_offset + 1u);
			}
			std::cout << "OR:\n";
		}
	}

}

void BeliefStateQuery::update_constraint(const StaticState &static_ref,const std::list<DynamicState> &belief_states, const std::list<float> &belief_probs, OrthotopeList &parameter_constraints) const
{
	if (!root) {
		throw std::invalid_argument("Can't evaluate an empty belief state query");
	}
	float belief_value = 0.0f;
	std::list<DynamicState>::const_iterator state_it = belief_states.cbegin();
	std::list<float>::const_iterator prob_it = belief_probs.cbegin();
	std::list<unsigned int> index_tracker;
	bool currently_satisfied = true, backtracked = false;
	BSQNode* current = nullptr;
	if (root->has_value) {
		if (root->is_satisfiable) {
			belief_value = 1.0f;
		}
		else {
			belief_value = 0.0f;
		}
	}
	else {
		//Calculate belief value.
		for (; state_it != belief_states.cend(); ++state_it, ++prob_it) {
			current = root;
			backtracked = false;
			while (current) {
				if (backtracked) {
					//means this either an and or or node.
					if (current->is_and && !currently_satisfied) {
						index_tracker.pop_back();
						current = current->parent;
					}
					else if (!current->is_and && currently_satisfied) {
						index_tracker.pop_back();
						current = current->parent;
					}
					else if (index_tracker.back() < current->children.size() - 1) {
						++index_tracker.back();
						backtracked = false;
						current = current->children[index_tracker.back()];
					}
					else {
						index_tracker.pop_back();
						current = current->parent;
					}
				}
				else {
					//first visit to this node.
					if(current->children.empty()){
						currently_satisfied = current->instance_check->evaluate(static_ref, *state_it);
						backtracked = true;
						current = current->parent;
					}
					else {
						index_tracker.push_back(0u);
						current = current->children[0];
					}
				}
			}
			if (currently_satisfied) {
				belief_value += *prob_it;
			}
		}
	}
	if (no_parameter) {
		switch (comparison) {
		case lt:
			currently_satisfied = belief_value < query_value;
			break;
		case gt:
			currently_satisfied = belief_value > query_value;
			break;
		case eq:
			currently_satisfied = belief_value == query_value;
			break;
		case neq:
			currently_satisfied = belief_value != query_value;
			break;
		case leq:
			currently_satisfied = belief_value <= query_value;
			break;
		case geq:
			currently_satisfied = belief_value >= query_value;
			break;
		}
		if (!currently_satisfied) {
			//No parameter ranges satisfy this constraint given, by default, it's unsatisfiable.
			parameter_constraints.clear();
		}
	}
	else {
		//Note that the operator is flipped (gt --> lt)
		switch (comparison) {
		case lt:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 2u);
			break;
		case gt:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 3u);
			break;
		case eq:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 0u);
			break;
		case neq:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 1u);
			break;
		case leq:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 4u);
			break;
		case geq:
			parameter_constraints.set_parameter_value(parameter_index, belief_value, 5u);
			break;
		}
	}
}

void inline BeliefStateQuery::parse_ineq(std::string &parse_target, const Problem &problem_ref, const std::map<std::string, ParameterInfo> &parameter_ref)
{
	size_t found = parse_target.rfind(']');
	std::string temp_str;
	std::map<std::string, ParameterInfo>::const_iterator map_it;

	if (found == std::string::npos) {
		throw std::invalid_argument("BSQ \"" + parse_target + "\" is incorrect format: missing closing bracket.");
	}
	if (found + 1 == parse_target.length()) {
		throw std::invalid_argument("BSQ \"" + parse_target + "\" is incorrect format: missing inequality");
	}
	temp_str = parse_target.substr(found + 1);
	if (temp_str.starts_with(" > ")) {
		comparison = gt;
		temp_str.erase(0, 3);
	}
	else if (temp_str.starts_with(" >= ")) {
		comparison = geq;
		temp_str.erase(0, 4);
	}
	else if (temp_str.starts_with(" < ")) {
		comparison = lt;
		temp_str.erase(0, 3);
	}
	else if (temp_str.starts_with(" <= ")) {
		comparison = leq;
		temp_str.erase(0, 4);
	}
	else if (temp_str.starts_with(" == ")) {
		comparison = eq;
		temp_str.erase(0, 4);
	}
	else if (temp_str.starts_with(" != ")) {
		comparison = neq;
		temp_str.erase(0, 4);
	}
	else {
		throw std::invalid_argument("BSQ \"" + parse_target + "\" is incorrect format: wrong inequality operator format.");
	}
	map_it = parameter_ref.find(temp_str);
	if (map_it == parameter_ref.end()) {
		//comparison must be to a double.
		no_parameter = true;
		try {
			query_value = std::stod(temp_str);
		}
		catch (...) {
			throw std::invalid_argument("Parameter \"" + temp_str + "\" isn't defined.");
		}
	}
	else {
		no_parameter = false;
		parameter_index = map_it->second.parameter_index;
	}
	parse_target.erase(found, parse_target.size() - found);
}
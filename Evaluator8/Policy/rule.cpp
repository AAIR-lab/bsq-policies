#include "rule.h"
#include <stdexcept>

PolicyRule::PolicyRule(std::string &ref, const Problem &problem_ref, const std::map<std::string, ParameterInfo> &parameter_info)
{
	//Will have the form: if/elif [condition] --> [action_name](parameter 1,...,parameter n)

	if (ref.starts_with("if ")) {
		ref.erase(0, 3);
	}
	else if (ref.starts_with("elif ")) {
		ref.erase(0, 5);
	}
	else {
		throw std::invalid_argument("Rule missing starting if/elif");
	}

	std::size_t found_index = ref.rfind(" --> "), second_found_index;
	std::string temp_string, other_temp_string;
	std::map<std::string, action_function_data>::const_iterator found_action;
	std::map<std::string, function_data>::const_iterator found_function;
	if (found_index == std::string::npos) {
		throw std::invalid_argument("Unable to find the split between condition and action");
	}
	temp_string = ref.substr(found_index + 5);
	ref.erase(found_index, ref.size() - found_index);
	found_index = temp_string.find('(');
	if (found_index == std::string::npos) {
		throw std::invalid_argument("Unable to find action parameter paranthesis.");
	}
	other_temp_string = temp_string.substr(0, found_index);
	found_action = problem_ref.action_information.find(other_temp_string);
	if (found_action == problem_ref.action_information.end()) {
		throw std::invalid_argument("Action isn't recognized: " + other_temp_string);
	}
	found_action->second.build_action(temp_string, rule_action);

	//Remaining is just the condition.
	root = new RuleConditionNode;
	RuleConditionNode* current_position = root, *temp_ptr = nullptr;
	std::vector<RuleConditionNode*>::iterator children_it;
	while (!ref.empty()) {
		if (!current_position) {
			throw std::invalid_argument("Failed to parse: early exit");
		}
		if (ref[0] == ',') {
			if (current_position->children.empty()) {
				throw std::invalid_argument("Misplaced comma.");
			}
			current_position->children.push_back(new RuleConditionNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			ref.erase(0, 1);
		}
		else if (ref[0] == ')') {
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
			ref.erase(0, 1);
		}
		else if (ref.starts_with("(AND ")) {
			if (!current_position->children.empty()) {
				throw std::invalid_argument("Missing comma.");
			}
			current_position->is_and = true;
			current_position->children.push_back(new RuleConditionNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			ref.erase(0, 5);
		}
		else if (ref.starts_with("(OR ")) {
			current_position->is_and = false;
			current_position->children.push_back(new RuleConditionNode());
			current_position->children.back()->parent = current_position;
			current_position = current_position->children.back();
			ref.erase(0, 4);
		}
		else if (ref.starts_with("P[")) {
			//BSQ query with the form P[...] o x 
			found_index = ref.find_first_of(']');
			if (found_index == std::string::npos) {
				throw std::invalid_argument("Missing closing bracket");
			}
			second_found_index = ref.find_first_of(',', found_index);
			found_index = ref.find_first_of(')', found_index);
			if (found_index == std::string::npos) {
				if (second_found_index == std::string::npos) {
					//The entire input must be the BSQ query
					current_position->belief_state_query = new BeliefStateQuery(ref, problem_ref, parameter_info);
					ref.clear();
				}
				else {
					temp_string = ref.substr(0, second_found_index);
					ref.erase(0, second_found_index);
					current_position->belief_state_query = new BeliefStateQuery(temp_string, problem_ref, parameter_info);

				}
			}
			else if (second_found_index == std::string::npos) {
				temp_string = ref.substr(0, found_index);
				ref.erase(0, found_index);
				current_position->belief_state_query = new BeliefStateQuery(temp_string, problem_ref, parameter_info);
			}
			else if (second_found_index < found_index) {
				temp_string = ref.substr(0, second_found_index);
				ref.erase(0, second_found_index);
				current_position->belief_state_query = new BeliefStateQuery(temp_string, problem_ref, parameter_info);
			}
			else {
				temp_string = ref.substr(0, found_index);
				ref.erase(0, found_index);
				current_position->belief_state_query = new BeliefStateQuery(temp_string, problem_ref, parameter_info);
			}
			current_position = current_position->parent;
		}
		else {
			//Has to be either a fully-observable function with or without an inequality.
			found_index = ref.find_first_of('(');
			if (found_index == std::string::npos) {
				throw std::invalid_argument("Unable to parse rule");
			}
			if (ref[0] == '!') {
				temp_string = ref.substr(1, found_index - 1);
			}
			else {
				temp_string = ref.substr(0, found_index);
			}
			found_function = problem_ref.function_information.find(temp_string);
			if (found_function == problem_ref.function_information.end()) {
				found_function = problem_ref.data_function_information.find(temp_string);
				if (found_function == problem_ref.data_function_information.end()) {
					std::cout << temp_string << std::endl;
					throw std::invalid_argument("Unable to find function " + temp_string);
				}
				// data functions have the form function_name(args) o [parameter name]
				found_index = ref.find_first_of(')', found_index);
				if (found_index == std::string::npos) {
					throw std::invalid_argument("Missing closing paranthesis");
				}
				second_found_index = ref.find_first_of(')', found_index + 1);
				found_index = ref.find_first_of(',', found_index);
				if (found_index == std::string::npos) {
					if (second_found_index == std::string::npos) {
						//Must be just a data function.
						current_position->parameter_val_inequality = new ParamValIneq(ref, problem_ref, parameter_info);
						ref.clear();
					}
					else {
						temp_string = ref.substr(0, second_found_index);
						ref.erase(0, second_found_index);
						current_position->parameter_val_inequality = new ParamValIneq(temp_string, problem_ref, parameter_info);
					}
				}
				else if (second_found_index == std::string::npos) {
					temp_string = ref.substr(0, found_index);
					ref.erase(0, found_index);
					current_position->parameter_val_inequality = new ParamValIneq(temp_string, problem_ref, parameter_info);
				}
				else if (found_index < second_found_index) {
					temp_string = ref.substr(0, found_index);
					ref.erase(0, found_index);
					current_position->parameter_val_inequality = new ParamValIneq(temp_string, problem_ref, parameter_info);
				}
				else {
					temp_string = ref.substr(0, second_found_index);
					ref.erase(0, second_found_index);
					current_position->parameter_val_inequality = new ParamValIneq(temp_string, problem_ref, parameter_info);
				}
			}
			else {
				if (!found_function->second.full_observable) {
					throw std::invalid_argument("Function " + found_function->first + " isn't fully observable.");
				}
				second_found_index = ref.find_first_of(')', found_index);
				if (second_found_index == std::string::npos) {
					throw std::invalid_argument("Missing closing paranthesis.");
				}
				temp_string = ref.substr(0, second_found_index + 1);
				ref.erase(0, second_found_index + 1);
				current_position->fully_observable_query_instance = found_function->second.build_instance(temp_string, problem_ref);
				if (!found_function->second.dynamic) {
					//Can be instantly evaluated to simplify.
					current_position->has_value = true;
					current_position->is_satisfiable = current_position->fully_observable_query_instance->evaluate(problem_ref.actual_static_state);
				}
			}
			current_position = current_position->parent;
		}
	}
}

PolicyRule::PolicyRule(const PolicyRule &ref):rule_action(ref.rule_action)
{
	if (!ref.root) {
		return;
	}
	root = new RuleConditionNode();
	std::list<std::pair<RuleConditionNode*, RuleConditionNode const*>> copy_stack;
	RuleConditionNode* current = nullptr;
	RuleConditionNode const* current_ref = nullptr;
	copy_stack.emplace_back(root, ref.root);
	while (!copy_stack.empty()) {
		current = copy_stack.back().first;
		current_ref = copy_stack.back().second;
		copy_stack.pop_back();
		current->has_value = current_ref->has_value;
		current->is_and = current_ref->is_and;
		current->is_satisfiable = current_ref->is_satisfiable;
		if (current_ref->belief_state_query) {
			current->belief_state_query = new BeliefStateQuery(*(current_ref->belief_state_query));
		}
		if (current_ref->fully_observable_query_instance) {
			current->fully_observable_query_instance = current_ref->fully_observable_query_instance->create_copy();
		}
		if (current_ref->parameter_val_inequality) {
			current->parameter_val_inequality = new ParamValIneq(*(current_ref->parameter_val_inequality));
		}
		if (!current_ref->children.empty()) {
			for (std::vector<RuleConditionNode*>::const_iterator it = current_ref->children.begin(); it != current_ref->children.end(); ++it) {
				current->children.push_back(new RuleConditionNode);
				current->children.back()->parent = current;
				copy_stack.emplace_back(current->children.back(), *it);
			}
		}
	}
}

PolicyRule::~PolicyRule()
{
	if (root) {
		delete root;
		root = nullptr;
	}
}

PolicyRule& PolicyRule::operator=(const PolicyRule &ref)
{
	if (&ref == this) {
		return *this;
	}
	rule_action = ref.rule_action;
	if (root) {
		delete root;
		root = nullptr;
	}
	if (!ref.root) {
		return *this;
	}

	root = new RuleConditionNode();
	std::list<std::pair<RuleConditionNode*, RuleConditionNode const*>> copy_stack;
	RuleConditionNode* current = nullptr;
	RuleConditionNode const* current_ref = nullptr;
	copy_stack.emplace_back(root, ref.root);
	while (!copy_stack.empty()) {
		current = copy_stack.back().first;
		current_ref = copy_stack.back().second;
		copy_stack.pop_back();
		current->has_value = current_ref->has_value;
		current->is_and = current_ref->is_and;
		current->is_satisfiable = current_ref->is_satisfiable;
		if (current_ref->belief_state_query) {
			current->belief_state_query = new BeliefStateQuery(*(current_ref->belief_state_query));
		}
		if (current_ref->fully_observable_query_instance) {
			current->fully_observable_query_instance = current_ref->fully_observable_query_instance->create_copy();
		}
		if (current_ref->parameter_val_inequality) {
			current->parameter_val_inequality = new ParamValIneq(*(current_ref->parameter_val_inequality));
		}
		if (!current_ref->children.empty()) {
			for (std::vector<RuleConditionNode*>::const_iterator it = current_ref->children.begin(); it != current_ref->children.end(); ++it) {
				current->children.push_back(new RuleConditionNode);
				current->children.back()->parent = current;
				copy_stack.emplace_back(current->children.back(), *it);
			}
		}
	}
	return *this;
}

void PolicyRule::update_constraint(const StaticState &static_ref, const std::list<DynamicState> &dynamic_ref, const std::list<float> &prob_ref, OrthotopeList &out_parameters) const
{
	std::list<unsigned int> index_tracker;
	bool backtracked = false;
	RuleConditionNode* current = root;
	if (!root) {
		throw std::invalid_argument("Can't evaluate empty rule condition.");
	}
	if (root->has_value) {
		if (!root->is_satisfiable) {
			//no parameters values satisfy this rule.
			out_parameters.clear();
		}
	}
	else {
		root->scratchwork = out_parameters;
		while (current) {
			if (backtracked) {
				//must be and or or node.
				if (current->is_and) {
					current->scratchwork = current->children[index_tracker.back()]->scratchwork;
					if (current->scratchwork.is_satisfiable()) {
						if (current->children.size() - 1 > index_tracker.back()) {
							++index_tracker.back();
							backtracked = false;
							current->children[index_tracker.back()]->scratchwork = current->scratchwork;
							current = current->children[index_tracker.back()];
						}
						else {
							index_tracker.pop_back();
							current = current->parent;
						}
					}
					else {
						index_tracker.pop_back();
						current = current->parent;
					}
				}
				else {
					current->scratchwork.not_update(current->children[index_tracker.back()]->scratchwork);
					if (current->scratchwork.is_satisfiable()) {
						if (current->children.size() - 1 > index_tracker.back()) {
							++index_tracker.back();
							backtracked = false;
							current->children[index_tracker.back()]->scratchwork = current->scratchwork;
							current = current->children[index_tracker.back()];
						}
						else {
							current->scratchwork = current->children[0]->scratchwork;
							for (unsigned int i = 1u; i <= index_tracker.back(); ++i) {
								current->scratchwork.or_update(current->children[i]->scratchwork);
							}
							index_tracker.pop_back();
							current = current->parent;
						}
					}
					else {
						//rebuild the satisfiable portion before backtracking.
						current->scratchwork = current->children[0]->scratchwork;
						for (unsigned int i = 1u; i <= index_tracker.back(); ++i) {
							current->scratchwork.or_update(current->children[i]->scratchwork);
						}
						index_tracker.pop_back();
						current = current->parent;
					}
				}
			}
			else {
				//First visit to this node.
				if (current->children.empty()) {
					if (current->belief_state_query) {
						current->belief_state_query->update_constraint(static_ref, dynamic_ref, prob_ref,current->scratchwork);
					}
					else if (current->fully_observable_query_instance) {
						if (!current->fully_observable_query_instance->evaluate(static_ref, dynamic_ref.front())) {
							//no parameters are satisfiable
							current->scratchwork.clear();
						}
					}
					else {
						current->parameter_val_inequality->update_constraint(static_ref, dynamic_ref, prob_ref,current->scratchwork);
					}
					backtracked = true;
					current = current->parent;
				}
				else {
					index_tracker.push_back(0u);
					current->children[0]->scratchwork = current->scratchwork;
					current = current->children[0];
				}
			}
		}
		out_parameters = root->scratchwork;
	}
}

void PolicyRule::print() const
{
	print(0u);
}

void PolicyRule::print(unsigned int current_offset) const
{
	if (!root) {
		return;
	}
	for (unsigned int i = 0u; i < current_offset; ++i) {
		std::cout << " ";
	}
	std::cout << "Rule Condition: " << std::endl;
	std::list<std::pair<RuleConditionNode*, unsigned int>> print_stack;
	print_stack.emplace_back(root, 0u);
	RuleConditionNode* current = nullptr;
	while (!print_stack.empty()) {
		current = print_stack.back().first;
		current_offset = print_stack.back().second;
		print_stack.pop_back();
		for (unsigned int i = 0u; i < current_offset; ++i) {
			std::cout << " ";
		}
		if (current->fully_observable_query_instance) {
			std::cout << "Instance(" << current->fully_observable_query_instance << "): " << current->fully_observable_query_instance->dynamic << " " << current->fully_observable_query_instance->full_observable << " " << current->fully_observable_query_instance->negated << " " << current->fully_observable_query_instance->target_index << " ";
			current->fully_observable_query_instance->print_value();
			std::cout << std::endl;
		}
		else if (current->parameter_val_inequality) {
			current->parameter_val_inequality->print_value();
		}
		else if (current->belief_state_query) {
			std::cout << "Belief-State Query: " << std::endl;
			current->belief_state_query->print(current_offset + 1u);
		}
		else if (current->children.empty()) {
			std::cout << "ERROR: empty node" << std::endl;
		}
		else {
			if (current->is_and) {
				std::cout << "AND:" << std::endl;
			}
			else {
				std::cout << "OR:" << std::endl;
			}
			for (std::vector<RuleConditionNode*>::const_reverse_iterator it = current->children.crbegin(); it != current->children.crend(); ++it) {
				print_stack.emplace_back(*it, current_offset + 1u);
			}
		}
	}
	for (unsigned int i = 0u; i < current_offset; ++i) {
		std::cout << " ";
	}
	std::cout << "Rule Action: " << rule_action.action_name << std::endl;
}
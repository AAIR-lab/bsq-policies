#include "parameter_value_inequality.h"
#include <iostream>
#include <stdexcept>

ParamValIneq::ParamValIneq(std::string &ref, const Problem &problem_ref, const std::map<std::string, ParameterInfo> &parameter_mapping):comparison(eq),target_index(0u), comparison_value(0.0), parameter_index(0u), has_comparison_value(false), is_double(false)
{
	//Assume the string looks like [value/function] o [parameter]
	std::size_t found_index = ref.find_first_of(' ');
	std::string temp_str, temp_func_name;
	std::map<std::string, function_data>::const_iterator function_found;
	std::map<std::string, unsigned int>::const_iterator found_constant;
	std::map<std::string, ParameterInfo>::const_iterator found_parameter;
	unsigned int offset_multiplier = 1u;
	if (found_index == std::string::npos) {
		throw std::invalid_argument(ref + " isn't recognized.");
	}
	if (found_index == 0) {
		throw std::invalid_argument("misplaced space.");
	}
	temp_str = ref.substr(0, found_index);
	ref.erase(0, found_index + 1);
	found_index = temp_str.find_first_of('(');
	if (found_index == std::string::npos) {
		comparison_value = std::stod(temp_str);
		has_comparison_value = true;
	}
	else {
		temp_func_name = temp_str.substr(0,found_index);
		function_found = problem_ref.data_function_information.find(temp_func_name);
		if (function_found == problem_ref.data_function_information.end()) {
			throw std::invalid_argument(temp_func_name + " is not a recognized function.");
		}
		is_double = function_found->second.function_type == type_double;
		has_comparison_value = false;
		temp_str.erase(0, found_index + 1);
		for (int i = 0; i < function_found->second.parameter_ranges.size(); ++i) {
			if (i == function_found->second.parameter_ranges.size() - 1) {
				found_index = temp_str.find_first_of(")");
			}
			else {
				found_index = temp_str.find_first_of(",");
			}
			if (found_index == std::string::npos) {
				throw std::invalid_argument("failed to parse parameters.");
			}
			temp_func_name = temp_str.substr(0, found_index);
			temp_str.erase(0, found_index + 1);
			found_constant = function_found->second.parameter_ranges[i].allowed_constants.find(temp_func_name);
			if (found_constant == function_found->second.parameter_ranges[i].allowed_constants.end()) {
				throw std::invalid_argument("Constant " + temp_func_name + " not recognized.");
			}
			target_index += offset_multiplier * found_constant->second;
			offset_multiplier *= function_found->second.parameter_ranges[i].allowed_constants.size();
		}
	}
	if (ref.starts_with("== ")) {
		comparison = eq;
		ref.erase(0, 3);
	}
	else if (ref.starts_with("!= ")) {
		comparison = neq;
		ref.erase(0, 3);
	}
	else if (ref.starts_with("> ")) {
		comparison = lt;
		ref.erase(0, 2);
	}
	else if (ref.starts_with("< ")) {
		comparison = gt;
		ref.erase(0, 2);
	}
	else if (ref.starts_with(">= ")) {
		comparison = leq;
		ref.erase(0, 3);
	}
	else if (ref.starts_with("<= ")) {
		comparison = geq;
		ref.erase(0, 3);
	}
	else {
		throw std::invalid_argument("inequlity operator not recognized: " + ref);
	}
	found_parameter = parameter_mapping.find(ref);
	if (found_parameter == parameter_mapping.end()) {
		throw std::invalid_argument("Parameter " + ref + " not recognized.");
	}
	parameter_index = found_parameter->second.parameter_index;
}

void ParamValIneq::print_value() const
{
	std::cout << "Parameter Value Inequality: ";
	if (has_comparison_value) {
		std::cout << comparison_value;
	}
	else{
		if (is_double) {
			std::cout << "double function(";
		}
		else {
			std::cout << "integer function(";
		}
		std::cout << target_index << ")";
	}
	switch (comparison) {
	case eq:
		std::cout << " == ";
		break;
	case neq:
		std::cout << " != ";
		break;
	case leq:
		std::cout << " >= ";
		break;
	case geq:
		std::cout << " <= ";
		break;
	case lt:
		std::cout << " > ";
		break;
	case gt:
		std::cout << " < ";
		break;
	}
	std::cout << "Parameter(" << parameter_index << ")" << std::endl;
}

void ParamValIneq::update_constraint(const StaticState &static_ref, const std::list<DynamicState> &belief_state, const std::list<float> &belief_probs, OrthotopeList &out_parameter_sets) const
{
	double temp_val = 0.0f;
	if (!has_comparison_value) {
		if (is_double) {
			temp_val = static_ref.double_params[target_index];
		}
		else {
			temp_val = static_ref.integer_params[target_index];
		}
	}
	else {
		temp_val = comparison_value;
	}

	switch (comparison) {
	case eq:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 0u);
		break;
	case neq:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 1u);
		break;
	case leq:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 5u);
		break;
	case geq:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 4u);
		break;
	case lt:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 3u);
		break;
	case gt:
		out_parameter_sets.set_parameter_value(parameter_index, temp_val, 2u);
		break;
	}
}
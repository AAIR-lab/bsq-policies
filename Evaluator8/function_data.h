#ifndef EVAL7_FUNCTION_DATA
#define EVAL7_FUNCTION_DATA

#include "query_instance.h"
#include "action.h"
#include<vector>
#include<map>

enum state_variable_types { type_string, type_int, type_bool, type_double};
enum parameter_types {para_type_string, para_type_int, para_type_bool, para_type_double, para_type_constant};

struct parameter_info {
	parameter_types type = para_type_constant;
	bool output_value = true;
	std::map<std::string, unsigned int> allowed_constants;
};

class Problem;

struct function_data {
	std::string function_name;
	bool dynamic = true;
	bool full_observable = true;
	state_variable_types function_type = type_int;
	unsigned int index = 0u;
	std::vector<parameter_info> parameter_ranges;
	QueryInstance* build_instance(const std::string &ref, const Problem &problem_ref) const;
private:
	QueryInstance* build_boolean_instance(const std::string&, bool) const;
	QueryInstance* build_double_instance(const std::string&, bool) const;
	QueryInstance* build_integer_instance(const std::string&, bool) const;
	QueryInstance* build_string_instance(const std::string&, bool) const;
};

struct action_function_data {
	std::vector<parameter_info> action_parameters;
	std::string action_name;
	Action* build_action(const std::string&) const;
	void build_action(const std::string&, Action&) const;
};

#endif

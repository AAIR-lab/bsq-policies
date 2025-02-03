#ifndef EVAL8_PARAMETER_INFO
#define EVAL8_PARAMETER_INFO

#include<string>

struct ParameterInfo {
	std::string parameter_name;
	unsigned int parameter_index = 0u;
	float lower_bound = 0.0f;
	float upper_bound = 0.0f;
	bool include_lower = true;
	bool include_upper = true;
};

#endif
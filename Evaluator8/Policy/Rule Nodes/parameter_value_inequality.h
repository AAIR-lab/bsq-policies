#ifndef EVAL_8_PARAM_VAL_INEQ
#define EVAL_8_PARAM_VAL_INEQ

#include "../BSQ/belief_state_query.h"

class ParamValIneq {
public:
	ParamValIneq(std::string&, const Problem&, const std::map<std::string, ParameterInfo>&);
	void print_value() const;
	void update_constraint(const StaticState&, const std::list<DynamicState>&, const std::list<float>&, OrthotopeList&) const;
private:
	bool has_comparison_value;
	bool is_double;
	double comparison_value;
	unsigned int target_index;
	ComparisonOperator comparison;
	unsigned int parameter_index;
};

#endif

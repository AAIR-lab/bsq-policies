#ifndef EVAL8_BELIEF_STATE_QUERY
#define EVAL8_BELIEF_STATE_QUERY

#include "belief_state_query_node.h"
#include "../../orthotope_list.h"
#include "../../problem.h"
#include "../../parameter_info.h"

enum ComparisonOperator{leq,geq,lt,gt,eq,neq};

class BeliefStateQuery {
public:
	BeliefStateQuery(std::string&,const Problem&,const std::map<std::string,ParameterInfo>&);
	BeliefStateQuery(const BeliefStateQuery&);
	~BeliefStateQuery();
	BeliefStateQuery& operator=(const BeliefStateQuery&);
	void update_constraint(const StaticState&, const std::list<DynamicState>&, const std::list<float>&, OrthotopeList&) const;
	void print() const;
	void print(unsigned int) const;
private:
	BSQNode* root;
	ComparisonOperator comparison;
	bool no_parameter;
	double query_value;
	unsigned int parameter_index;
	void inline parse_ineq(std::string&, const Problem&, const std::map<std::string, ParameterInfo>&);
};

#endif
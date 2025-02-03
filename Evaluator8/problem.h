#ifndef EVAL7_PROBLEM_H
#define EVAL7_PROBLEM_H

#include<list>
#include<vector>
#include<map>
#include<random>
#include "dynamic_state.h"
#include "static_state.h"
#include "function_data.h"

class Problem {
public:
    virtual ~Problem();
    //std::vector<std::string> constants;
    //std::vector<std::pair<std::string,FunctionOutputType>> functions;
    float discount_factor = 1.0f;
    std::default_random_engine generator;
    virtual Problem* create_copy() const = 0;
    //void input(std::string);
    virtual void generate_initial_belief(StaticState& static_init_state, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const = 0;
    //void output(std::string) const;
    std::map<std::string, function_data> data_function_information;
    std::map<std::string, function_data> function_information;
    std::map<std::string, action_function_data> action_information;
    std::vector<std::string> constants;
//protected:
    StaticState actual_static_state;
};

#endif
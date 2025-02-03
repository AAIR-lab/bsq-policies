#ifndef EVAL7_STATIC_STATE_H
#define EVAL7_STATIC_STATE_H

#include<vector>
#include<iostream>
#include "dynamic_state.h"

struct StaticState {
    std::vector<std::string> string_params;
    std::vector<int> integer_params;
    std::vector<bool> bool_params;
    std::vector<double> double_params;
};

void add_to_sorted(std::list<DynamicState>&, std::list<float>&, const DynamicState&, const float&);

#endif
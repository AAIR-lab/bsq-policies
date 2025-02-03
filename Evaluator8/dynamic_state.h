#ifndef EVAL7_DYNAMIC_STATE_H
#define EVAL7_DYNAMIC_STATE_H

#include<vector>
#include<list>
#include<iostream>

struct DynamicState {
    //params --> variables
    std::vector<std::string> string_params;
    std::vector<int> integer_params;
    std::vector<bool> bool_params;
    std::vector<double> double_params;
    std::list<float> rewards;
    bool terminated = false;
    bool stuck = false;

    float calculate_expected_value(float) const;

    bool operator== (const DynamicState&) const;
};

void add_to_sorted(std::list<DynamicState>&, std::list<float>&, const DynamicState&, const float&);

#endif
#ifndef EVAL7_ACTION_H
#define EVAL7_ACTION_H

#include<string>
#include<vector>

struct Action {
    std::string action_name;
    std::vector<std::string> string_params;
    std::vector<int> integer_params;
    std::vector<bool> bool_params;
    std::vector<double> double_params;

    bool operator== (const Action&) const;
};

#endif
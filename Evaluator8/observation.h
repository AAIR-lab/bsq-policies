#ifndef EVAL7_OBSERVATION_H
#define EVAL7_OBSERVATION_H

#include<string>
#include<vector>
#include<list>

struct Observation {
    std::vector<std::string> string_params;
    std::vector<int> integer_params;
    std::vector<bool> bool_params;
    std::vector<double> double_params;

    bool operator== (const Observation&) const;
    bool operator< (const Observation&) const;
    int compare(const Observation&) const;
};

void add_to_sorted(std::list<Observation>&, std::list<float>&, const Observation&, const float&);

#endif
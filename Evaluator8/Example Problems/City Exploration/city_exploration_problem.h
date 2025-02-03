#ifndef CITY_EXPLORATION_PROBLEM_H
#define CITY_EXPLORATION_PROBLEM_H

#include "../../problem.h"

class CityExploration : public virtual Problem {
public:
    CityExploration();
    ~CityExploration();
    Problem* create_copy() const;
    void generate_initial_belief(StaticState&, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const;
};

#endif

#ifndef LANE_MERGER_PROBLEM_H
#define LANE_MERGER_PROBLEM_H

#include "../../problem.h"

class LaneMerger : public virtual Problem {
public:
    LaneMerger();
    ~LaneMerger();
    Problem* create_copy() const;
    void generate_initial_belief(StaticState&, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const;
private:
    void inline fill_observation_table();
};

#endif
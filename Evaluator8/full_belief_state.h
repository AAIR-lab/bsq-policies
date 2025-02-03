#ifndef EVAL7_FULL_BELIEF_STATE_H
#define EVAL7_FULL_BELIEF_STATE_H

#include "belief_state.h"

struct FullBeliefState : public virtual BeliefState {
    std::list<DynamicState> temp_states;
    std::list<float> temp_probs;

    void apply_action(const Action&);
    void apply_observation(const Action&, const Observation&);
    void build_initial_belief(const Problem&);
    BeliefState* create_copy() const;
    void generate_observation(const Action&, Observation&);

    FullBeliefState(Domain&);
    ~FullBeliefState();
};

#endif
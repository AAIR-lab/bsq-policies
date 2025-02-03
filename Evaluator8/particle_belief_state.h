#ifndef EVAL7_PARTICLE_FILTER_BELIEF_STATE_H
#define EVAL7_PARTICLE_FILTER_BELIEF_STATE_H

#include "belief_state.h"

struct PFBeliefState : public virtual BeliefState {
    unsigned int particle_number = 0;
    std::list<DynamicState> temp_states;
    std::list<float> temp_probs;

    void apply_action(const Action&);
    void apply_observation(const Action&, const Observation&);
    void build_initial_belief(const Problem&);
    BeliefState* create_copy() const;
    void generate_observation(const Action&, Observation&);

    PFBeliefState(unsigned int, Domain&);
    ~PFBeliefState();
};

#endif
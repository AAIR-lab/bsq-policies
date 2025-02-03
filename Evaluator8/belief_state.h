#ifndef EVAL7_BELIEF_STATE_H
#define EVAL7_BELIEF_STATE_H

#include "action.h"
#include "observation.h"
#include "problem.h"
#include "dynamic_state.h"
#include "static_state.h"
#include "domain.h"
#include<random>

struct BeliefState {
    StaticState static_state;
    std::list<DynamicState> state_list;
    std::list<float> probabilities;
    Domain* problem_domain;
    bool terminated = false;
    float goal_state_prob = 0.0f;
    std::default_random_engine generator;

    virtual ~BeliefState();
    void all_possible_observations(const Action&, std::list<Observation>&, std::list<float>&) const;
    virtual void apply_action(const Action&) = 0;
    virtual void apply_observation(const Action&, const Observation&) = 0;
    virtual void build_initial_belief(const Problem&) = 0;
    float calculate_expected_value(float) const;
    virtual BeliefState* create_copy() const = 0;
    virtual void generate_observation(const Action&, Observation&) = 0;
    //void update_terminated();
    void calculate_termination_stuck_percent();
};

#endif
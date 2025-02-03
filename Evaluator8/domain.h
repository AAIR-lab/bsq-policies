#ifndef EVAL7_DOMAIN_H
#define EVAL7_DOMAIN_H

#include "action.h"
#include "observation.h"
#include "dynamic_state.h"
#include "static_state.h"
#include <random>

struct Domain {
    virtual void all_possible_actions(const DynamicState&, const StaticState&, std::vector<Action>&) const = 0;
    virtual bool can_create_observation(const Action&) const = 0;
    virtual Domain* create_copy() const = 0;
    virtual bool next_state_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<DynamicState>&, std::vector<float>&) const = 0;
    virtual bool observation_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<Observation>&, std::vector<float>&) const = 0;
    virtual float observation_function(const DynamicState&, const StaticState&, const Action&, const Observation&) const;
    virtual float reward_function(const StaticState&, const DynamicState&, const Action&, const DynamicState&) const = 0;
    bool sample_observation(const DynamicState&, const StaticState&, const Action&, Observation&, std::default_random_engine&);
    virtual float transition_function(const DynamicState&, const StaticState&, const Action&, const DynamicState&) const;
    bool update_state(DynamicState&, const StaticState&, const Action&, std::default_random_engine&);

};

#endif
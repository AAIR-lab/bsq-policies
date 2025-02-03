#ifndef GRAPH_ROCK_SAMPLE_DOMAIN_H
#define GRAPH_ROCK_SAMPLE_DOMAIN_H

#include "../../domain.h"

struct GraphRockSampleDomain : public Domain {
    void all_possible_actions(const DynamicState&, const StaticState&, std::vector<Action>&) const;
    bool can_create_observation(const Action&) const;
    Domain* create_copy() const;
    bool next_state_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<DynamicState>&, std::vector<float>&) const;
    bool observation_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<Observation>&, std::vector<float>&) const;
    float reward_function(const StaticState&, const DynamicState&, const Action&, const DynamicState&) const;
};

#endif
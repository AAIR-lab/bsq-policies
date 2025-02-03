#ifndef LANE_MERGER_DOMAIN_H
#define LANE_MERGER_DOMAIN_H

#include "../../domain.h"

struct LaneMergerDomain : public Domain {
    LaneMergerDomain();
    void all_possible_actions(const DynamicState&, const StaticState&, std::vector<Action>&) const;
    bool can_create_observation(const Action&) const;
    Domain* create_copy() const;
    bool next_state_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<DynamicState>&, std::vector<float>&) const;
    bool observation_distribution(const DynamicState&, const StaticState&, const Action&, std::vector<Observation>&, std::vector<float>&) const;
    float reward_function(const StaticState&, const DynamicState&, const Action&, const DynamicState&) const;
private:
    inline void move_cars(const DynamicState&, const StaticState&, std::vector<DynamicState>&, std::vector<float>&, int, bool) const;
    std::vector<Observation> OBSERVATION_FILL;
};

#endif
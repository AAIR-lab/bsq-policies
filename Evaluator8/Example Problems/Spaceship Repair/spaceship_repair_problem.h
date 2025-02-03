#ifndef SPACESHIP_REPAIR_PROBLEM_H
#define SPACESHIP_REPAIR_PROBLEM_H

#include "../../problem.h"

class SpaceshipRepair : public virtual Problem {
public:
    SpaceshipRepair();
    SpaceshipRepair(unsigned int rob_station_distance, unsigned int ship_station_distance, double rob_acc, double ship_acc);
    ~SpaceshipRepair();
    Problem* create_copy() const;
    void generate_initial_belief(StaticState&, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const;
};

#endif
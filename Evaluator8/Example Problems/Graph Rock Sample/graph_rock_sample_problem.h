#ifndef GRAPH_ROCK_SAMPLE_PROBLEM_H
#define GRAPH_ROCK_SAMPLE_PROBLEM_H

#include "../../problem.h"

class GraphRockSample : public virtual Problem {
public:
    GraphRockSample();
    ~GraphRockSample();
    Problem* create_copy() const;
    void generate_initial_belief(StaticState&, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const;
};

#endif

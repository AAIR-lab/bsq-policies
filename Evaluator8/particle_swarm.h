#ifndef EVAL8_PART_SWARM
#define EVAL8_PART_SWARM

#include<vector>
#include "belief_state.h"
#include "Policy/policy.h"
#include "problem.h"
#include "parallel_evaluator.h"

struct Particle{
    float expected_value = 0.0f;
    std::vector<float> parameter_values;
    std::vector<float> velocity;
    
    std::vector<float> best_parameter_values_seen;
    float best_expected_value_seen = 0.0f;
};

class ParticleSwarm{
public:
    ParticleSwarm(BeliefState*, Policy*, Problem*, float, float, float, float, unsigned int, std::string, unsigned int, 
        float, float, unsigned int, unsigned int, unsigned int, std::pair<float,float>,std::string);
    ~ParticleSwarm();
    void optimize();
private:
    float coef_change_rate;
    float cognitive_coef_init;
    float cognitive_coef_min;
    float momentum;
    unsigned int patience;
    std::string results_file;
    unsigned int sample_limit;
    float social_coef_init;
    float social_coef_max;
    unsigned int swarm_size;
    unsigned int thread_number;
    unsigned int horizon;
    std::string iteration_path;
    std::pair<float,float> velocity_range;
    
    unsigned int steps_since_improvement;
    std::vector<Particle> particles; 
    BeliefState* belief_state_instance;
    Policy* policy_instance;
    Problem* problem_instance;
    std::default_random_engine* generator;
    ParallelEvaluator evaluator;
    Particle* current_best;
    Particle all_time_best;
    std::vector<std::pair<float,float>> parameter_ranges;

    inline void build_initial_particles();
    inline void print();
    inline void step();
};

#endif
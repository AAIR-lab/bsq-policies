#ifndef NELDER_MEAD_EVAL_7_H
#define NELDER_MEAD_EVAL_7_H

#include "belief_state.h"
#include "Policy/policy.h"
#include "problem.h"
#include "parallel_evaluator.h"

#include<list>
#include<vector>
#include<random>

struct parameter_point {
    std::vector<float> parameter_values;
    float expected_value = 0.0f;
    float distance(parameter_point&);
    void average(parameter_point&, std::vector<float>& out);
};

class NelderMead {
public:
    NelderMead(BeliefState*, Policy*, Problem*, std::string, std::string, std::vector<std::pair<float, float>>&, float, unsigned int, unsigned int,
        unsigned int, unsigned int, unsigned int,float, std::string);
    ~NelderMead();
    void optimize();
private:
    std::list<parameter_point> vertices;
    unsigned int vertex_count;
    unsigned int points_checked;
    unsigned int horizon;
    unsigned int particle_count;
    unsigned int sample_limit;
    unsigned int thread_number;
    std::vector<std::pair<float, float>>& parameter_ranges;
    BeliefState* belief_state_instance;
    Policy* policy_instance;
    Problem* problem_instance;
    std::string solved_path;
    std::string problem_path;
    std::string iteration_path;
    float discount_factor;
    float min_distance;
    std::default_random_engine* generator;
    bool verbose;
    ParallelEvaluator evaluator;

    bool add_vertice(parameter_point&);
    void build_point(parameter_point& out);
    void build_point(std::vector<float>&, parameter_point& out);
    void calculate_remaining_centroid(std::vector<float>& out);
    void contract_vertices();
    void generate_contract_points(std::vector<float>& first, std::vector<float>& second);
    void generate_point(std::vector<float>& out);
    void generate_extended_point(std::vector<float>& out);
    float sum_expected_value();
};

#endif
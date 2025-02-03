#ifndef EVAL7_INTERVAL_SOLVER_H
#define EVAL7_INTERVAL_SOLVER_H

#include "belief_state.h"
#include "Policy/policy.h"
#include <mutex>

struct interval_info {
    interval_info(const OrthotopeList&);
    interval_info(const std::vector<DimensionRange>&);
    interval_info(const OrthotopeList&, float);
    interval_info(const OrthotopeList&, const std::list<float>&);
    float expected_value = -1.0f * INFINITY;
    float standard_deviation = 0.0f;
    OrthotopeList intervals;
    std::list<float> leaf_values;
    void update_metrics();
};

enum interval_solver_type{global_thompson, local_thompson, epsilon_greedy, softmax, certainty_max};

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState*, Policy&, Problem&, Domain*, std::string solved_path, std::string interval_path, unsigned int minimum_samples, float min_time_allocation,
    float max_time_allocation);

void interval_solver(unsigned int thread_number, unsigned int horizon, 
    BeliefState*, Policy&, Problem&, Domain*, std::string solved_path, std::string interval_path, unsigned int minimum_samples, float min_time_allocation,
    float max_time_allocation,interval_solver_type, bool save_time_segments, std::string time_segment_path, int segment_count);

void interval_solver(unsigned int thread_number, unsigned int horizon, 
    BeliefState*, Policy&, Problem&, Domain*, std::string solved_path, std::string interval_path, unsigned int minimum_samples, float min_time_allocation,
    float max_time_allocation,interval_solver_type, bool save_time_segments, std::string time_segment_path, int segment_count, float max_explore_rate);

void interval_solver(unsigned int thread_number, unsigned int horizon, 
    BeliefState*, Policy&, Problem&, Domain*, std::string solved_path, std::string interval_path, unsigned int minimum_samples, float min_time_allocation,
    float max_time_allocation,interval_solver_type, bool save_time_segments, std::string time_segment_path, int segment_count, unsigned int sample_count);

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, unsigned int sample_count, float max_explore_rate);

void interval_solver_with_init(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, unsigned int sample_count, float max_explore_rate);

void interval_workers(std::string interval_path, BeliefState*, Policy*, Problem*, Domain*, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, unsigned int* best_leaf_count, int* best_id,
    int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, interval_solver_type solve_type);

void interval_workers_sample_count(std::string interval_path, BeliefState*, Policy*, Problem*, Domain*, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, unsigned int* best_leaf_count, int* best_id,
    int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, interval_solver_type solve_type, unsigned int sample_count);

/*inline void interval_workers_global_thompson(std::string interval_path, BeliefState*, Policy*, Problem*, Domain*, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::vector<bool>* out_list,
    std::mutex* best_lock, interval_info* best_interval, bool** current_best, int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_local_thompson(std::string interval_path, BeliefState*, Policy*, Problem*, Domain*, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::vector<bool>* out_list,
    std::mutex* best_lock, interval_info* best_interval, bool** current_best, int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState*, Policy*, Problem*, Domain*, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::vector<bool>* out_list,
    std::mutex* best_lock, interval_info* best_interval, bool** current_best, int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);*/

inline void interval_workers_global_thompson(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_local_thompson(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_softmax(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_certainty_max(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate);

inline void interval_workers_local_thompson(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count);

inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count);

inline void interval_workers_softmax(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count);
#endif

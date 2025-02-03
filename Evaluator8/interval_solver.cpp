#include "interval_solver.h"
#include "eval_policy_dfs.h"
#include <random>
#include <iostream>
#include <thread>
#include <fstream>
#include <math.h>

interval_info::interval_info(const OrthotopeList& ref) :intervals(ref)
{
}

interval_info::interval_info(const std::vector<DimensionRange>& ref) :intervals(ref)
{
}

interval_info::interval_info(const OrthotopeList& interval_ref, float value) :intervals(interval_ref)
{
    leaf_values.emplace_back(value);
    update_metrics();
}

interval_info::interval_info(const OrthotopeList &interval_ref, const std::list<float> &value_ref) :intervals(interval_ref), leaf_values(value_ref)
{
    update_metrics();
}

void interval_info::update_metrics()
{
    expected_value = 0.0f;
    for (std::list<float>::iterator it = leaf_values.begin(); it != leaf_values.end(); ++it) {
        expected_value += *it;
    }
    expected_value /= leaf_values.size();

    standard_deviation = 0.0f;
    for (std::list<float>::iterator it = leaf_values.begin(); it != leaf_values.end(); ++it) {
        standard_deviation += powf(*it - expected_value, 2.0f);
    }
    standard_deviation /= leaf_values.size();
    standard_deviation = powf(standard_deviation, 0.5f);
    standard_deviation += 0.00001f;
}

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation) {

    interval_solver(thread_number, horizon, belief_state_ptr, policy, problem, domain, solved_path, interval_path, minimum_samples, min_time_allocation,
        max_time_allocation, global_thompson,false,"",0);
}

/*
void interval_solver(unsigned int thread_number, unsigned int horizon, 
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments, 
    std::string time_segment_path, int segment_count) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;

    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false, * current_best = NULL;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::vector<bool> out_list(thread_number, false);
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    OrthotopeList temp_best_interval;
    policy.get_full_parameter_range(temp_best_interval);
    interval_info best_interval(temp_best_interval);

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock, &out_list,
            &best_lock, &best_interval, &current_best, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }

    total_intervals += 1;
    orphan_list.emplace_back(best_interval);


    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;
    while (time_expended < max_time_allocation) {
        all_waiting = true;
        out_lock.lock();
        for (out_it = out_list.begin(); all_waiting && out_it != out_list.end(); ++out_it) {
            all_waiting = *out_it;
        }
        out_lock.unlock();
        if (all_waiting) {
            break;
        }
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if(!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval.intervals << "," << best_interval.expected_value << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_interval.leaf_values.size() >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max(((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation,0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval.intervals << "," << best_interval.expected_value << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }

}*/

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;

    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    
    OrthotopeList best_interval;
    policy.get_full_parameter_range(best_interval);
    float best_ev = -1000000.0f;
    unsigned int best_leaf_count = 0u;
    int best_thread = -1;

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock,
            &best_lock, &best_interval, &best_ev, &best_leaf_count, &best_thread, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }

    total_intervals += 1;
    orphan_list.emplace_back(best_interval);


    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;
    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_leaf_count >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max(((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation, 0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }
}

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, float max_explore_rate) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;
    //std::cout << "PING" << std::endl;

    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    
    OrthotopeList best_interval;
    policy.get_full_parameter_range(best_interval);
    float best_ev = -1000000.0f;
    unsigned int best_leaf_count = 0u;
    int best_thread = -1;

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock,
            &best_lock, &best_interval, &best_ev, &best_leaf_count, &best_thread, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }

    total_intervals += 1;
    orphan_list.emplace_back(best_interval);


    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;
    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_leaf_count >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max(max_explore_rate * (((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation), 0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }
}

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, unsigned int sample_count, float max_explore_rate) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;

    //std::cout << "PING" << std::endl;
    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    
    OrthotopeList best_interval;
    //std::list<OrthotopeList> start_intervals;
    policy.get_full_parameter_range(best_interval);
    float best_ev = -1000000.0f;
    unsigned int best_leaf_count = 0u;
    int best_thread = -1;

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    //best_interval.split(thread_number,start_intervals);

    // total_intervals = start_intervals.size();
    // for(std::list<OrthotopeList>::iterator it = start_intervals.begin(); it != start_intervals.end(); ++it){
    //     orphan_list.emplace_back(*it);
    // }
    orphan_list.emplace_back(best_interval);
    total_intervals = 1;

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock,
            &best_lock, &best_interval, &best_ev, &best_leaf_count, &best_thread, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }


    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;
    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_leaf_count >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max((max_explore_rate * ((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation), 0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }
}

struct eval_data{
    std::vector<float> point;
    std::list<float> values;
    float ec = 0.0f;
    OrthotopeList interval;
};

void eval_worker(BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* in_lock, std::mutex *out_lock, unsigned int *current_index, std::vector<eval_data> *targets, 
    std::list<unsigned int> *out_list){

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    unsigned int chosen_index = 0u;
    bool got_index = false;

    while(!*close_worker){
        in_lock->lock();
        got_index = targets->size() > *current_index;
        if(got_index){
            chosen_index = *current_index;
            ++(*current_index);
        }
        in_lock->unlock();

        if(!got_index){
            continue;
        }
        evalPolicyDFS_multi(new_belief_state, *temp_pol, *temp_problem, (*targets)[chosen_index].point,horizon,1.0f,
            (*targets)[chosen_index].interval,(*targets)[chosen_index].values, (*targets)[chosen_index].ec, 80u);

        out_lock->lock();
        out_list->push_front(chosen_index);

        out_lock->unlock();
    }

    delete new_belief_state;
    new_belief_state = NULL;
    delete temp_pol;
    temp_pol = NULL;
    delete temp_problem;
    temp_problem = NULL;

}

void interval_solver_with_init(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, unsigned int sample_count, float max_explore_rate) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;

    //std::cout << "PING" << std::endl;
    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    
    OrthotopeList best_interval, full_interval;
    //std::list<OrthotopeList> start_intervals;
    policy.get_full_parameter_range(best_interval);
    full_interval = best_interval;
    float best_ev = -1000000.0f;
    unsigned int best_leaf_count = 0u;
    int best_thread = -1;

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    //orphan_list.emplace_back(best_interval);
    //total_intervals = 1;
    std::vector<eval_data> start_points(20);
    std::list<unsigned int> out_list;
    unsigned int current_index = 0u, temp_index = 0u;
    bool new_out = false;
    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    std::vector<std::thread> threads;
    int count = 0;

    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;

    for(int i = 0; i < 20; ++i){
        start_points[i].interval = best_interval;
        best_interval.uniform_sample_point(generator,start_points[i].point);
    }

    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(eval_worker, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &orphan_lock, &out_lock,
            &current_index, &start_points, &out_list);
    }    

    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (time_expended >= sample_take.front()) {
            best_lock.lock();
            output_file.open(time_segment_path, std::ios::app);
            output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
            output_file.close();
            best_lock.unlock();
            sample_take.pop_front();
        }
        out_lock.lock();
        new_out = !out_list.empty();
        if(new_out){
            temp_index = out_list.front();
            out_list.pop_front();
        }
        out_lock.unlock();

        if(new_out){
            if(start_points[temp_index].ec > best_ev){
                best_interval = start_points[temp_index].interval;
                best_ev = start_points[temp_index].ec;
            }
            ++count;
        }

        if(count == 20){
            break;
        }
        if (time_expended >= min_time_allocation) {
            break;
        }
    }

    worker_stop = true;
    best_ev = -1000000.0f;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }

    full_interval.not_update(start_points[0].interval);
    orphan_list.emplace_back(start_points[0].interval,start_points[0].values);

    for (int i = 1 ; i < 20; ++i){
        full_interval.not_update(start_points[i].interval);
        for(std::list<interval_info>::iterator oit = orphan_list.begin(); oit != orphan_list.end(); ++oit){
            start_points[i].interval.not_update(oit->intervals);
        }
        if(start_points[i].interval.is_satisfiable()){
            orphan_list.emplace_back(start_points[i].interval,start_points[i].values);
        }
    }
    if(full_interval.is_satisfiable()){
        orphan_list.emplace_back(full_interval);
    }

    worker_stop = false; 
    threads.clear();

    total_intervals = orphan_list.size();

    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock,
            &best_lock, &best_interval, &best_ev, &best_leaf_count, &best_thread, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }



    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_leaf_count >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max((max_explore_rate * ((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation), 0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }
}

void interval_solver(unsigned int thread_number, unsigned int horizon,
    BeliefState* belief_state_ptr, Policy& policy, Problem& problem, Domain* domain, std::string solved_path, std::string interval_path,
    unsigned int minimum_samples, float min_time_allocation, float max_time_allocation, interval_solver_type solve_type, bool save_time_segments,
    std::string time_segment_path, int segment_count, unsigned int sample_count) {

    min_time_allocation *= 1000.0f;
    max_time_allocation *= 1000.0f;

    std::list<float> sample_take;
    float time_dif = 0.0f;
    if (save_time_segments) {
        time_dif = min_time_allocation / float(segment_count);
        for (int i = 1; i <= segment_count; ++i) {
            sample_take.push_back(time_dif * i);
        }
    }

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now(), current_time;

    std::mutex orphan_lock, out_lock, best_lock;
    bool worker_stop = false;
    float exploration_rate = 1.0f;
    unsigned int total_intervals = 0u;
    std::list<interval_info> orphan_list;
    std::string best_interval_str;
    
    OrthotopeList best_interval;
    policy.get_full_parameter_range(best_interval);
    float best_ev = -1000000.0f;
    unsigned int best_leaf_count = 0u;
    int best_thread = -1;

    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    output_file << "I_braid" << "," << "E_braid";
    output_file << std::endl;
    output_file.close();

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_number; ++i) {
        threads.emplace_back(interval_workers, interval_path, belief_state_ptr, &policy, &problem, domain, horizon, &worker_stop, &out_lock,
            &best_lock, &best_interval, &best_ev, &best_leaf_count, &best_thread, i, &orphan_list, &orphan_lock,
            &total_intervals, thread_number, &exploration_rate, solve_type);
    }

    total_intervals += 1;
    orphan_list.emplace_back(best_interval);


    bool all_waiting = false;
    std::vector<bool>::iterator out_it;
    float time_expended = 0.0f;
    while (time_expended < max_time_allocation) {
        current_time = std::chrono::high_resolution_clock::now();
        time_expended = (float)std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (!sample_take.empty())
            if (time_expended >= sample_take.front()) {
                best_lock.lock();
                output_file.open(time_segment_path, std::ios::app);
                output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
                output_file.close();
                best_lock.unlock();
                sample_take.pop_front();
            }
        if (time_expended >= min_time_allocation) {
            best_lock.lock();
            all_waiting = best_leaf_count >= minimum_samples;
            best_lock.unlock();
        }
        if (all_waiting) {
            break;
        }
        exploration_rate = std::max((0.5f * ((float)min_time_allocation - (float)time_expended) / (float)min_time_allocation), 0.0f);
    }

    best_lock.lock();
    output_file.open(solved_path, std::ios::app);
    output_file << best_interval << "," << best_ev << "," << time_expended / 1000.0f << std::endl;
    output_file.close();
    best_lock.unlock();

    worker_stop = true;

    for (int i = 0; i < thread_number; ++i) {
        threads[i].join();
    }
}

void interval_workers(std::string interval_path, BeliefState *belief_state, Policy *policy, Problem *problem, Domain *domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, unsigned int* best_leaf_count, int* best_id,
    int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, interval_solver_type solve_type) {

    if (solve_type == global_thompson) {
        interval_workers_global_thompson(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);
    }
    else if (solve_type == local_thompson) {
        interval_workers_local_thompson(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);
    }
    else if (solve_type == softmax) {
        interval_workers_softmax(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);        
    }
    else if (solve_type == certainty_max) {
        interval_workers_certainty_max(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);           
    }
    else {
        interval_workers_epsilon_greedy(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);
    }
}

void interval_workers_sample_count(std::string interval_path, BeliefState *belief_state, Policy *policy, Problem *problem, Domain *domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, unsigned int* best_leaf_count, int* best_id,
    int id, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, interval_solver_type solve_type, unsigned int sample_count) {

    if (solve_type == global_thompson) {
        interval_workers_global_thompson(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate);
    }
    else if (solve_type == local_thompson) {
        interval_workers_local_thompson(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate, sample_count);
    }
    else if (solve_type == softmax) {
        interval_workers_softmax(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate, sample_count);        
    }
    else {
        interval_workers_epsilon_greedy(interval_path, belief_state, policy, problem, domain, horizon, close_worker, out_lock, best_lock,
            best_interval, best_ev, best_id, id, best_leaf_count, orphan_intervals, orphan_lock, total_intervals, thread_number, exploration_rate, sample_count);
    }
}

inline void interval_workers_global_thompson(std::string interval_path, BeliefState *belief_state, Policy *policy, Problem *problem, Domain *domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int *best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0;
    float local_best_ev = 0.0;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::const_iterator local_best_interval = intervals.cend();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
            if (!interval_it->intervals.is_satisfiable()) {
                continue;
            }
            if (interval_it->leaf_values.size() < 5 || std::normal_distribution<float>(interval_it->expected_value, (interval_it->standard_deviation * *exploration_rate) + 0.00001)(generator) >= *best_ev) {
                selected_interval = interval_it;
                if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                    continue;
                }
                temp_interval = selected_interval->intervals;
                //std::cout << selected_interval->intervals << std::endl;
                current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);

                if(selected_interval->intervals.is_satisfiable()){
                    temp_interval.not_update(selected_interval->intervals);
                    if (temp_interval.is_satisfiable()) {
                        orphan_lock->lock();
                        orphan_intervals->emplace_back(temp_interval,selected_interval->leaf_values);
                        ++*total_intervals;
                        orphan_lock->unlock();
                    }
                }
                else{
                    selected_interval->intervals = temp_interval;
                }

                selected_interval->leaf_values.emplace_back(current_expected_value);
                selected_interval->update_metrics();
                
                //new best check
                if (selected_interval == local_best_interval) {
                    // Means that this is the already known hypothesized best interval.
                    // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
                    if (selected_interval->expected_value < local_best_ev) {
                        for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                            if (sit->expected_value > selected_interval->expected_value) {
                                selected_interval = sit;
                            }
                        }
                        local_best_interval = selected_interval;
                    }
                    local_best_ev = local_best_interval->expected_value;
                }
                else if (selected_interval->expected_value > local_best_interval->expected_value) {
                    local_best_interval = selected_interval;
                    local_best_ev = local_best_interval->expected_value;
                }

                best_lock->lock();
                if (*best_id == id) {
                    // Means the current thread has the best.
                    *best_ev = local_best_ev;
                    *best_interval = local_best_interval->intervals;
                    *best_leaf_count = local_best_interval->leaf_values.size();
                }
                else if (local_best_ev > *best_ev) {
                    *best_ev = local_best_ev;
                    *best_interval = local_best_interval->intervals;
                    *best_id = id;
                    *best_leaf_count = local_best_interval->leaf_values.size();
                }
                best_lock->unlock();

            
                if (*close_worker) {
                    break;
                }
            }
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_local_thompson(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    //std::cout << "lts" << std::endl;

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, best_expected_value = 0.0f, possible_best_ev = 0.0f;
    float local_best_ev = 0.0f;
    unsigned int min_samples = 5u;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >=  5) {
            if (*exploration_rate <= 0.0f) {
                // No exploration.
                selected_interval = local_best_interval;
            }
            else {
                //std::normal_distribution<float> z_gen(-*exploration_rate, *exploration_rate);
                std::normal_distribution<float> z_gen(0.0f,1.0f);
                selected_interval = intervals.begin();
                best_expected_value = selected_interval->expected_value + (z_gen(generator) * selected_interval->standard_deviation);
                interval_it = selected_interval;
                ++interval_it;
                for (; interval_it != intervals.end(); ++interval_it) {
                    if (!interval_it->intervals.is_satisfiable()) {
                        continue;
                    }
                    possible_best_ev = interval_it->expected_value + (z_gen(generator) * interval_it->standard_deviation);
                    if (possible_best_ev > best_expected_value) {
                        selected_interval = interval_it;
                        best_expected_value = possible_best_ev;
                    }
                }
            }
        }

        do {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);
            

            if(!selected_interval->intervals.is_satisfiable()){
                // This occurs when the precision difference is too close to tell.
                // Look at implementation of is_close() in utitily functions.
                // In this case, the original interval is restored.
                selected_interval->intervals = temp_interval;
            }
            else{
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if(!selected_interval->intervals.is_satisfiable()){
            }

            //non-grs: 30, 3
            if(selected_interval->expected_value >= local_best_interval->expected_value){
                min_samples = 30u;
            }
            else{
                min_samples = 5u;
            }

        } while (selected_interval->leaf_values.size() < min_samples);

        if (selected_interval == local_best_interval) {
            // Means that this is the already known hypothesized best interval.
            // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
            if (selected_interval->expected_value < local_best_ev) {
                for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                    if (sit->leaf_values.size() < 30) {
                        continue;
                    }
                    if (sit->expected_value > selected_interval->expected_value) {
                        selected_interval = sit;
                    }
                }
                local_best_interval = selected_interval;
            }
            local_best_ev = local_best_interval->expected_value;
        }
        else if (selected_interval->expected_value > local_best_interval->expected_value) {
            local_best_interval = selected_interval;
            local_best_ev = local_best_interval->expected_value;
        }

        best_lock->lock();
        if (*best_id == id) {
            // Means the current thread has the best.
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        else if (local_best_ev > *best_ev) {
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_id = id;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        best_lock->unlock();

        if (*close_worker) {
            break;
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_local_thompson(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, best_expected_value = 0.0f, possible_best_ev = 0.0f;
    float local_best_ev = 0.0f;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >=  5) {
            if (*exploration_rate <= 0.0f) {
                // No exploration.
                selected_interval = local_best_interval;
            }
            else {
                //std::normal_distribution<float> z_gen(-*exploration_rate, *exploration_rate);
                std::normal_distribution<float> z_gen(0.0f, *exploration_rate);
                selected_interval = intervals.begin();
                best_expected_value = selected_interval->expected_value + (z_gen(generator) * selected_interval->standard_deviation);
                interval_it = selected_interval;
                ++interval_it;
                for (; interval_it != intervals.end(); ++interval_it) {
                    if (!interval_it->intervals.is_satisfiable()) {
                        continue;
                    }
                    possible_best_ev = interval_it->expected_value + (z_gen(generator) * interval_it->standard_deviation);
                    if (possible_best_ev > best_expected_value) {
                        selected_interval = interval_it;
                        best_expected_value = possible_best_ev;
                    }
                }
            }
        }

        for(int j = 0; j < sample_count; ++j) {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);
            

            if(!selected_interval->intervals.is_satisfiable()){
                // This occurs when the precision difference is too close to tell.
                // Look at implementation of is_close() in utitily functions.
                // In this case, the original interval is restored.
                selected_interval->intervals = temp_interval;
            }
            else{
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if(!selected_interval->intervals.is_satisfiable()){
            }
        }

        if (selected_interval == local_best_interval) {
            // Means that this is the already known hypothesized best interval.
            // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
            if (selected_interval->expected_value < local_best_ev) {
                for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                    if (sit->expected_value > selected_interval->expected_value) {
                        selected_interval = sit;
                    }
                }
                local_best_interval = selected_interval;
            }
            local_best_ev = local_best_interval->expected_value;
        }
        else if (selected_interval->expected_value > local_best_interval->expected_value) {
            local_best_interval = selected_interval;
            local_best_ev = local_best_interval->expected_value;
        }

        best_lock->lock();
        if (*best_id == id) {
            // Means the current thread has the best.
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        else if (local_best_ev > *best_ev) {
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_id = id;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        best_lock->unlock();

        if (*close_worker) {
            break;
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

/*
inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, best_expected_value = 0.0f, possible_best_ev = 0.0f;
    float local_best_ev = 0.0f;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    unsigned int sample_count = 0u;

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >= 5) {
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(generator) <= *exploration_rate) {
                //exploration
                selected_interval = intervals.begin();
                std::advance(selected_interval, std::uniform_int_distribution<>(0, intervals.size() - 1)(generator));
            }
            else {
                //exploitation
                selected_interval = local_best_interval;
            }
        }

        // if(intervals.size() == 1){
        //     sample_count = 50u;
        // }
        // else{
        //     sample_count = 5u;
        // }
        sample_count = 5u;

        do {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);

            if(selected_interval->intervals.is_satisfiable()){
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            else{
                selected_interval->intervals = temp_interval;
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if (selected_interval == local_best_interval) {
                // Means that this is the already known hypothesized best interval.
                // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
                if (selected_interval->expected_value < local_best_ev) {
                    for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                        if (sit->expected_value > selected_interval->expected_value) {
                            selected_interval = sit;
                        }
                    }
                    local_best_interval = selected_interval;
                }
                local_best_ev = local_best_interval->expected_value;
            }
            else if (selected_interval->expected_value > local_best_interval->expected_value) {
                local_best_interval = selected_interval;
                local_best_ev = local_best_interval->expected_value;
            }

            best_lock->lock();
            if (*best_id == id) {
                // Means the current thread has the best.
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            else if (local_best_ev > *best_ev) {
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_id = id;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            best_lock->unlock();

            if (*close_worker) {
                break;
            }
        } while (selected_interval->leaf_values.size() < sample_count);
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}
*/

inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    //std::cout << "eg" << std::endl;
    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, best_expected_value = 0.0f, possible_best_ev = 0.0f;
    float local_best_ev = 0.0f;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    unsigned int sample_count = 0u;

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >= 5) {
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(generator) <= *exploration_rate) {
                //exploration
                selected_interval = intervals.begin();
                std::advance(selected_interval, std::uniform_int_distribution<>(0, intervals.size() - 1)(generator));
            }
            else {
                //exploitation
                selected_interval = local_best_interval;
            }
        }

        // if(intervals.size() == 1){
        //     sample_count = 50u;
        // }
        // else{
        //     sample_count = 5u;
        // }
        sample_count = 5u;

        do {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);

            if(selected_interval->intervals.is_satisfiable()){
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            else{
                selected_interval->intervals = temp_interval;
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();

            if(selected_interval->expected_value >= local_best_interval->expected_value){
                sample_count = 30u;
            }
            else{
                sample_count = 5u;
            }

        } while (selected_interval->leaf_values.size() < sample_count);
        if (selected_interval == local_best_interval) {
            // Means that this is the already known hypothesized best interval.
            // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
            if (selected_interval->expected_value < local_best_ev) {
                for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                    if (sit->leaf_values.size() < 20) {
                        continue;
                    }
                    if (sit->expected_value > selected_interval->expected_value) {
                        selected_interval = sit;
                    }
                }
                local_best_interval = selected_interval;
            }
            local_best_ev = local_best_interval->expected_value;
        }
        else if (selected_interval->expected_value > local_best_interval->expected_value) {
            local_best_interval = selected_interval;
            local_best_ev = local_best_interval->expected_value;
        }

        best_lock->lock();
        if (*best_id == id) {
            // Means the current thread has the best.
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        else if (local_best_ev > *best_ev) {
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_id = id;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        best_lock->unlock();

        if (*close_worker) {
            break;
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_epsilon_greedy(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, best_expected_value = 0.0f, possible_best_ev = 0.0f;
    float local_best_ev = 0.0f;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >= 5) {
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(generator) <= *exploration_rate) {
                //exploration
                selected_interval = intervals.begin();
                std::advance(selected_interval, std::uniform_int_distribution<>(0, intervals.size() - 1)(generator));
            }
            else {
                //exploitation
                selected_interval = local_best_interval;
            }
        }

        for(int j = 0; j < sample_count; ++j) {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);

            if(selected_interval->intervals.is_satisfiable()){
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            else{
                selected_interval->intervals = temp_interval;
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if (selected_interval == local_best_interval) {
                // Means that this is the already known hypothesized best interval.
                // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
                if (selected_interval->expected_value < local_best_ev) {
                    for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                        if (sit->expected_value > selected_interval->expected_value) {
                            selected_interval = sit;
                        }
                    }
                    local_best_interval = selected_interval;
                }
                local_best_ev = local_best_interval->expected_value;
            }
            else if (selected_interval->expected_value > local_best_interval->expected_value) {
                local_best_interval = selected_interval;
                local_best_ev = local_best_interval->expected_value;
            }

            best_lock->lock();
            if (*best_id == id) {
                // Means the current thread has the best.
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            else if (local_best_ev > *best_ev) {
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_id = id;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            best_lock->unlock();

            if (*close_worker) {
                break;
            }
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_softmax(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    //std::cout << "bolt" << std::endl;

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float local_best_ev = 0.0f, temperature = 0.0f, sum_value = 0.0f, current_expected_value = 0.0f, chosen_value = 0.0f;

    std::vector<float> evaluation_tuple;
    std::vector<float> epsilon_values;
    unsigned int temp_index = 0u;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >=  5) {
            if (*exploration_rate <= 0.0f) {
                // No exploration.
                selected_interval = local_best_interval;
            }
            else {
                temperature = *exploration_rate * 0.5f * horizon;
                selected_interval = intervals.begin();
                epsilon_values.resize(intervals.size());
                epsilon_values[0] = exp(selected_interval->expected_value/temperature);
                sum_value = epsilon_values[0];
                interval_it = selected_interval;
                ++interval_it;
                temp_index = 1u;
                for (; interval_it != intervals.end(); ++interval_it, ++temp_index) {
                    if (!interval_it->intervals.is_satisfiable()) {
                        continue;
                    }
                    epsilon_values[temp_index] = exp(interval_it->expected_value/temperature);
                    sum_value += epsilon_values[temp_index];
                }
                chosen_value = sum_value * exploration_decider(generator);
                temp_index = 0u;
                chosen_value -= epsilon_values[0];
                while(chosen_value > 0.0f && selected_interval != intervals.end()){
                    ++temp_index;
                    ++selected_interval;
                    chosen_value -= epsilon_values[temp_index];
                }
            }
        }

        unsigned int min_samples = 5u;

        do {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);
            

            if(!selected_interval->intervals.is_satisfiable()){
                // This occurs when the precision difference is too close to tell.
                // Look at implementation of is_close() in utitily functions.
                // In this case, the original interval is restored.
                selected_interval->intervals = temp_interval;
            }
            else{
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if(!selected_interval->intervals.is_satisfiable()){
            }

            if(selected_interval->expected_value >= local_best_interval->expected_value){
                min_samples = 100u;
            }
            else{
                min_samples = 3u;
            }

        } while (selected_interval->leaf_values.size() < min_samples);

        if (selected_interval == local_best_interval) {
            // Means that this is the already known hypothesized best interval.
            // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
            if (selected_interval->expected_value < local_best_ev) {
                for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                    if(sit->leaf_values.size() < 100){
                        continue;
                    }
                    if (sit->expected_value > selected_interval->expected_value) {
                        selected_interval = sit;
                    }
                }
                local_best_interval = selected_interval;
            }
            local_best_ev = local_best_interval->expected_value;
        }
        else if (selected_interval->expected_value > local_best_interval->expected_value) {
            local_best_interval = selected_interval;
            local_best_ev = local_best_interval->expected_value;
        }

        best_lock->lock();
        if (*best_id == id) {
            // Means the current thread has the best.
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        else if (local_best_ev > *best_ev) {
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_id = id;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        best_lock->unlock();

        if (*close_worker) {
            break;
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_softmax(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate, unsigned int sample_count) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float local_best_ev = 0.0f, temperature = 0.0f, sum_value = 0.0f, current_expected_value = 0.0f, chosen_value = 0.0f;

    std::vector<float> evaluation_tuple;
    std::vector<float> epsilon_values;
    unsigned int temp_index = 0u;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >=  5) {
            if (*exploration_rate <= 0.0f) {
                // No exploration.
                selected_interval = local_best_interval;
            }
            else {
                temperature = *exploration_rate * 10.0f;
                selected_interval = intervals.begin();
                epsilon_values.resize(intervals.size());
                epsilon_values[0] = exp(selected_interval->expected_value/temperature);
                sum_value = epsilon_values[0];
                interval_it = selected_interval;
                ++interval_it;
                temp_index = 1u;
                for (; interval_it != intervals.end(); ++interval_it, ++temp_index) {
                    if (!interval_it->intervals.is_satisfiable()) {
                        continue;
                    }
                    epsilon_values[temp_index] = exp(interval_it->expected_value/temperature);
                    sum_value += epsilon_values[temp_index];
                }
                chosen_value = sum_value * exploration_decider(generator);
                temp_index = 0u;
                chosen_value -= epsilon_values[0];
                while(chosen_value > 0.0f && selected_interval != intervals.end()){
                    ++temp_index;
                    ++selected_interval;
                    chosen_value -= epsilon_values[temp_index];
                }
            }
        }

        for(int j = 0; j < sample_count; ++j) {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);
            

            if(!selected_interval->intervals.is_satisfiable()){
                // This occurs when the precision difference is too close to tell.
                // Look at implementation of is_close() in utitily functions.
                // In this case, the original interval is restored.
                selected_interval->intervals = temp_interval;
            }
            else{
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if(!selected_interval->intervals.is_satisfiable()){
            }
        }

        if (selected_interval == local_best_interval) {
            // Means that this is the already known hypothesized best interval.
            // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
            if (selected_interval->expected_value < local_best_ev) {
                for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                    if (sit->expected_value > selected_interval->expected_value) {
                        selected_interval = sit;
                    }
                }
                local_best_interval = selected_interval;
            }
            local_best_ev = local_best_interval->expected_value;
        }
        else if (selected_interval->expected_value > local_best_interval->expected_value) {
            local_best_interval = selected_interval;
            local_best_ev = local_best_interval->expected_value;
        }

        best_lock->lock();
        if (*best_id == id) {
            // Means the current thread has the best.
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        else if (local_best_ev > *best_ev) {
            *best_ev = local_best_ev;
            *best_interval = local_best_interval->intervals;
            *best_id = id;
            *best_leaf_count = local_best_interval->leaf_values.size();
        }
        best_lock->unlock();

        if (*close_worker) {
            break;
        }
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

inline void interval_workers_certainty_max(std::string interval_path, BeliefState* belief_state, Policy* policy, Problem* problem, Domain* domain, unsigned int horizon,
    bool* close_worker, std::mutex* out_lock, std::mutex* best_lock, OrthotopeList* best_interval, float* best_ev, int* best_id, int id, unsigned int* best_leaf_count, std::list<interval_info>* orphan_intervals, std::mutex* orphan_lock,
    unsigned int* total_intervals, unsigned int thread_number, float* exploration_rate) {

    BeliefState* new_belief_state = belief_state->create_copy();
    new_belief_state->problem_domain = domain->create_copy();
    Policy* temp_pol = new Policy(*policy);
    Problem* temp_problem = problem->create_copy();

    std::default_random_engine generator;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    new_belief_state->generator.seed(std::chrono::system_clock::now().time_since_epoch().count() + id);
    std::uniform_real_distribution<float> exploration_decider(0.0f, 1.0f);

    OrthotopeList temp_interval;
    temp_pol->get_full_parameter_range(temp_interval);

    float current_expected_value = 0.0f, highest_deviation_value = 0.0f;
    float local_best_ev = 0.0f;

    std::vector<float> evaluation_tuple;

    std::list<interval_info> intervals;
    std::list<interval_info>::iterator selected_interval, interval_it;
    std::list<interval_info>::iterator local_best_interval = intervals.end();

    while (!*close_worker) {
        if (orphan_lock->try_lock()) {
            if (!orphan_intervals->empty()) {
                if (*total_intervals / thread_number >= intervals.size()) {
                    intervals.push_back(orphan_intervals->front());
                    orphan_intervals->pop_front();
                    selected_interval = intervals.end();
                    --selected_interval;
                }
            }
            orphan_lock->unlock();
        }

        if (local_best_interval == intervals.end()) {
            if (!intervals.empty()) {
                local_best_interval = intervals.begin();
            }
            else {
                continue;
            }
        }

        if (selected_interval == intervals.end() || selected_interval->leaf_values.size() >= 5) {
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(generator) <= *exploration_rate) {
                //exploration
                selected_interval = intervals.begin();
                std::advance(selected_interval, std::uniform_int_distribution<>(0, intervals.size() - 1)(generator));
            }
            else {
                //exploitation
                selected_interval = intervals.begin();
                highest_deviation_value = selected_interval->standard_deviation;
                interval_it = selected_interval;
                ++interval_it;
                for(;interval_it != intervals.end(); ++interval_it){
                    if(interval_it->standard_deviation > highest_deviation_value){
                        highest_deviation_value = interval_it->standard_deviation;
                        selected_interval = interval_it;
                    }
                }
            }
        }

        do {
            if (!selected_interval->intervals.uniform_sample_point(generator, evaluation_tuple)) {
                break;
            }
            temp_interval = selected_interval->intervals;
            current_expected_value = evalPolicyDFS(new_belief_state, *temp_pol, *temp_problem, evaluation_tuple, horizon, temp_problem->discount_factor, selected_interval->intervals);

            if(selected_interval->intervals.is_satisfiable()){
                temp_interval.not_update(selected_interval->intervals);
                if (temp_interval.is_satisfiable()) {
                    orphan_lock->lock();
                    orphan_intervals->emplace_back(temp_interval, selected_interval->leaf_values);
                    ++* total_intervals;
                    orphan_lock->unlock();
                }
            }
            else{
                selected_interval->intervals = temp_interval;
            }
            selected_interval->leaf_values.emplace_back(current_expected_value);

            selected_interval->update_metrics();
            if (selected_interval == local_best_interval) {
                // Means that this is the already known hypothesized best interval.
                // If the update lowered its expected value it checks to see if there is a better interval before checking the global.
                if (selected_interval->expected_value < local_best_ev) {
                    for (std::list<interval_info>::iterator sit = intervals.begin(); sit != intervals.end(); ++sit) {
                        if (sit->expected_value > selected_interval->expected_value) {
                            selected_interval = sit;
                        }
                    }
                    local_best_interval = selected_interval;
                }
                local_best_ev = local_best_interval->expected_value;
            }
            else if (selected_interval->expected_value > local_best_interval->expected_value) {
                local_best_interval = selected_interval;
                local_best_ev = local_best_interval->expected_value;
            }

            best_lock->lock();
            if (*best_id == id) {
                // Means the current thread has the best.
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            else if (local_best_ev > *best_ev) {
                *best_ev = local_best_ev;
                *best_interval = local_best_interval->intervals;
                *best_id = id;
                *best_leaf_count = local_best_interval->leaf_values.size();
            }
            best_lock->unlock();

            if (*close_worker) {
                break;
            }
        } while (selected_interval->leaf_values.size() < 5);
    }

    out_lock->lock();
    std::ofstream output_file;
    output_file.open(interval_path, std::ios::app);
    for (interval_it = intervals.begin(); interval_it != intervals.end(); ++interval_it) {
        if (interval_it->leaf_values.size() == 0 || !interval_it->intervals.is_satisfiable()) {
            continue;
        }
        output_file << interval_it->intervals << "," << interval_it->expected_value << "," << interval_it->leaf_values.size() << std::endl;
    }
    output_file.close();
    out_lock->unlock();

    delete new_belief_state->problem_domain;
    new_belief_state->problem_domain = NULL;

    delete new_belief_state;
    new_belief_state = NULL;

    delete temp_pol;
    temp_pol = NULL;

    delete temp_problem;
    temp_problem = NULL;
}

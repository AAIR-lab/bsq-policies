#include "parallel_evaluator.h"
#include "eval_policy_dfs.h"
#include<iostream>

void parallel_evaluator_worker(Problem* current_problem, Domain* current_domain, BeliefState* current_belief, Policy* current_policy, 
    unsigned int horizon,std::mutex *input_lock, std::mutex *output_lock,std::list<std::pair<std::vector<float>,unsigned int>> *input_list, 
    std::list<float> *output_list, bool *continue_loop)
{
    Problem* my_problem = current_problem->create_copy();
    Domain* my_domain = current_domain->create_copy();
    BeliefState* my_belief = current_belief->create_copy();
    Policy my_policy(*current_policy);

    std::vector<float> current_input;
    float current_sum = 0.0f;
    unsigned int evaluation_count = 0u;
    bool found_input = false;

    while (*continue_loop) {
        input_lock->lock();
        found_input = !input_list->empty();
        if (found_input) {
            current_input = input_list->front().first;
            evaluation_count = input_list->front().second;
            input_list->pop_front();
        }
        input_lock->unlock();

        if (found_input) {
            current_sum = 0.0f;
            for (int i = 0; i < evaluation_count; ++i) {
                current_sum += evalPolicyDFS(my_belief, my_policy, *my_problem, current_input, horizon, 1.0f);
            }
            output_lock->lock();
            output_list->emplace_back(current_sum);
            output_lock->unlock();
        }
    }

    delete my_problem;
    delete my_domain;
    delete my_belief;
}

ThreadInfo::ThreadInfo(Problem *problem, Domain *domain, BeliefState *belief, Policy *policy, unsigned int horizon, 
    std::mutex *out_lock, std::list<float> *out_list):worker_thread(parallel_evaluator_worker,problem,domain,belief,
    policy,horizon,&input_lock,out_lock,&inbox,out_list,&keep_looping){

}

ParallelEvaluator::ParallelEvaluator(Problem* current_problem, Domain* current_domain, BeliefState* current_belief, 
    Policy* current_policy, unsigned int horizon, unsigned int thread_number):thread_number(thread_number){
    
    for(unsigned int i = 0u; i < thread_number; ++i){
        thread_list.emplace_back(current_problem, current_domain, current_belief, current_policy, horizon, &output_lock, &outbox);
    }
}

ParallelEvaluator::~ParallelEvaluator(){
    for(std::list<ThreadInfo>::iterator it = thread_list.begin(); it != thread_list.end(); ++it){
        it->keep_looping = false;
        it->worker_thread.join();
    }
    thread_list.clear();
}

float ParallelEvaluator::evaluate(const std::vector<float> &parameter_values, unsigned int sample_size){
    unsigned int non_last_samples = std::ceil(sample_size/thread_number);
    unsigned int last_worker_samples = sample_size - ((thread_number - 1) * non_last_samples);
    float output = 0.0f;
    unsigned int finished_workers = 0u;

    std::list<ThreadInfo>::iterator it = thread_list.begin(), last_element = std::prev(thread_list.end());

    for(;it != last_element; ++it){
        it->input_lock.lock();
        it->inbox.emplace_back(parameter_values,non_last_samples);
        it->input_lock.unlock();
    }

    last_element->input_lock.lock();
    last_element->inbox.emplace_back(parameter_values,last_worker_samples);
    last_element->input_lock.unlock();

    while(finished_workers < thread_number){
        output_lock.lock();
        if(!outbox.empty()){
            output += outbox.front();
            finished_workers += 1;
            outbox.pop_front();
        }
        output_lock.unlock();
    }

    return output / (float)sample_size;
}
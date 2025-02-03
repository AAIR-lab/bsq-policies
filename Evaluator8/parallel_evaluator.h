#ifndef EVAL8_PARALLEL_EVALUATOR
#define EVAL8_PARALLEL_EVALUATOR

#include<thread>
#include<mutex>
#include "problem.h"
#include "belief_state.h"
#include "domain.h"
#include "Policy/policy.h"

struct ThreadInfo{
    std::thread worker_thread;
    std::mutex input_lock;
    std::list<std::pair<std::vector<float>, unsigned int>> inbox;
    bool keep_looping = true;
    ThreadInfo(Problem*, Domain*, BeliefState*, Policy*, unsigned int, std::mutex*, std::list<float>*);
};

class ParallelEvaluator{
public:    
    ParallelEvaluator(Problem* current_problem, Domain* current_domain, BeliefState* current_belief, 
        Policy* current_policy, unsigned int horizon, unsigned int thread_number);
    ~ParallelEvaluator();
    float evaluate(const std::vector<float>&, unsigned int sample_size);
private:
    std::list<ThreadInfo> thread_list;
    std::mutex output_lock;
    std::list<float> outbox;
    unsigned int thread_number;
};

#endif
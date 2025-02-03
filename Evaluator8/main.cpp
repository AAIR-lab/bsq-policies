#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <iomanip>
#include "Analysis/problem_analysis.h"
#include "Analysis/domain_analysis.h"
#include "Example Problems/City Exploration/city_exploration_policy_generators.h"
#include "Example Problems/City Exploration/city_exploration_domain.h"
#include "Example Problems/City Exploration/city_exploration_problem.h"
#include "Example Problems/Graph Rock Sample/graph_rock_sample_domain.h"
#include "Example Problems/Graph Rock Sample/graph_rock_sample_problem.h"
#include "Policy/BSQ/belief_state_query.h"
#include "Policy/Rule Nodes/parameter_value_inequality.h"
#include "Policy/rule.h"
#include "Policy/policy.h"
#include "interval_solver.h"
#include "full_belief_state.h"
#include "Analysis/policy_analysis.h"
#include "eval_policy_dfs.h"
#include "Example Problems/Lane Merger/lane_merger_policy_generator.h"
#include "Example Problems/Lane Merger/lane_merger_domain.h"
#include "Example Problems/Lane Merger/lane_merger_problem.h"
#include "Example Problems/Spaceship Repair/spaceship_repair_domain.h"
#include "Example Problems/Spaceship Repair/spaceship_repair_problem.h"
#include "Analysis/compliance_analysis.h"
#include "action.h"
#include "observation.h"
#include "nelder_mead.h"
#include "particle_swarm.h"


//Source of code for detecting memory leaks: https://stackoverflow.com/questions/4790564/finding-memory-leaks-in-a-c-application-with-visual-studio
#include "windows.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>

enum defined_problems { LM, GRS, SR, SV };

void run_solver(defined_problems selected_problem, interval_solver_type selected_solver, std::string solutions_file_path, std::string interval_file_path, std::string time_samples_path, unsigned int run_count, unsigned int horizon, unsigned int min_time, unsigned int max_time, unsigned int thread_number, unsigned int time_sample_count)
{
    Problem* chosen_problem = NULL;
    Domain* chosen_domain = NULL;
    std::string policy_path = "";

    switch (selected_problem) {
    case LM:
        chosen_problem = new LaneMerger();
        chosen_domain = new LaneMergerDomain();
        policy_path = "Example Problems/Lane Merger/Policies/lane_merger_policy3.txt";
        break;
    case GRS:
        chosen_problem = new GraphRockSample();
        chosen_domain = new GraphRockSampleDomain();
        policy_path = "Example Problems/Graph Rock Sample/Policies/graph_rock_sample_policy1.txt";
        break;
    case SR:
        chosen_problem = new SpaceshipRepair();
        chosen_domain = new SpaceshipRepairDomain();
        policy_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
        break;
    case SV:
        chosen_problem = new CityExploration();
        chosen_domain = new CityExplorationDomain();
        policy_path = "Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt";
        break;
    default:

        return;
        break;
    }

    FullBeliefState current_belief(*chosen_domain);
    Policy test_policy(policy_path, *chosen_problem);
    for (int i = 0; i < run_count; ++i) {
        interval_solver_with_init(thread_number, horizon, &current_belief, test_policy, 
            *chosen_problem, chosen_domain, solutions_file_path, interval_file_path, 
            3u, min_time, max_time, selected_solver, true, time_samples_path, time_sample_count,1,1.0f);
    }

    if (chosen_problem) {
        delete chosen_problem;
        chosen_problem = NULL;
    }
    if (chosen_domain) {
        delete chosen_domain;
        chosen_domain = NULL;
    }
}

void run_nelder(defined_problems selected_problem, std::string solutions_file_path, std::string time_samples_path, unsigned int run_count, unsigned int horizon, unsigned int thread_number, unsigned int sample_number, float minimum_distance)
{
    Problem* chosen_problem = NULL;
    Domain* chosen_domain = NULL;
    std::string policy_path = "";

    switch (selected_problem) {
    case LM:
        chosen_problem = new LaneMerger();
        chosen_domain = new LaneMergerDomain();
        policy_path = "Example Problems/Lane Merger/Policies/lane_merger_policy3.txt";
        break;
    case GRS:
        chosen_problem = new GraphRockSample();
        chosen_domain = new GraphRockSampleDomain();
        policy_path = "Example Problems/Graph Rock Sample/Policies/graph_rock_sample_policy1.txt";
        break;
    case SR:
        chosen_problem = new SpaceshipRepair();
        chosen_domain = new SpaceshipRepairDomain();
        policy_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
        break;
    case SV:
        chosen_problem = new CityExploration();
        chosen_domain = new CityExplorationDomain();
        policy_path = "Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt";
        break;
    default:

        return;
        break;
    }

    FullBeliefState current_belief(*chosen_domain);
    Policy test_policy(policy_path, *chosen_problem);

    OrthotopeList temp_full_parameters;
    std::vector<std::pair<float, float>> parameter_ranges;
    test_policy.get_full_parameter_range(temp_full_parameters);
    temp_full_parameters.construct_ranges(parameter_ranges);
    
    // The number of edges in the simplex is one greater than the number of parameters.
    unsigned int vertex_count = parameter_ranges.size() + 1;

    NelderMead nm(&current_belief, &test_policy, chosen_problem,"",solutions_file_path,parameter_ranges,1.0f,vertex_count,horizon,0u,sample_number,thread_number,minimum_distance,time_samples_path);

    for (int i = 0; i < run_count; ++i) {
        nm.optimize();
    }

    if (chosen_problem) {
        delete chosen_problem;
        chosen_problem = NULL;
    }
    if (chosen_domain) {
        delete chosen_domain;
        chosen_domain = NULL;
    }
}

void run_particle(defined_problems selected_problem, std::string solutions_file_path, std::string time_samples_path, unsigned int run_count, unsigned int horizon, unsigned int thread_number)
{
    Problem* chosen_problem = NULL;
    Domain* chosen_domain = NULL;
    std::string policy_path = "";

    float coefficient_rate = 0.1f;                          //Rate the cognitive coefficient decreases and social coefficient increase for each iteration without seeing an improvemet.
    float cognitive_coefficient_init = 1.0f;
    float cognitive_coefficient_min = 0.1f;
    float social_coefficient_init = 0.0f;
    float social_coefficient_max = 1.0f;
    float momentum = 0.6f;
    unsigned int patience = 10u;                            //Number of iterations without seeing an improvement before deeming convergence.
    unsigned int sample_limit = 1000u;                      //Samples to take for each parameter value point evaluated.
    unsigned int swarm_size = 10u;
    std::pair<float, float> momentum_range(-0.5f, 0.5f);    //Range of values allowed for the momentum.

    switch (selected_problem) {
    case LM:
        chosen_problem = new LaneMerger();
        chosen_domain = new LaneMergerDomain();
        policy_path = "Example Problems/Lane Merger/Policies/lane_merger_policy3.txt";
        break;
    case GRS:
        chosen_problem = new GraphRockSample();
        chosen_domain = new GraphRockSampleDomain();
        policy_path = "Example Problems/Graph Rock Sample/Policies/graph_rock_sample_policy1.txt";
        break;
    case SR:
        chosen_problem = new SpaceshipRepair();
        chosen_domain = new SpaceshipRepairDomain();
        policy_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
        break;
    case SV:
        chosen_problem = new CityExploration();
        chosen_domain = new CityExplorationDomain();
        policy_path = "Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt";
        break;
    default:

        return;
        break;
    }

    FullBeliefState current_belief(*chosen_domain);
    Policy test_policy(policy_path, *chosen_problem);

    OrthotopeList temp_full_parameters;
    std::vector<std::pair<float, float>> parameter_ranges;
    test_policy.get_full_parameter_range(temp_full_parameters);
    temp_full_parameters.construct_ranges(parameter_ranges);

    ParticleSwarm ps(&current_belief,&test_policy,chosen_problem,coefficient_rate,cognitive_coefficient_init,cognitive_coefficient_min,
        momentum,patience,solutions_file_path,sample_limit,social_coefficient_init,social_coefficient_max,swarm_size,horizon,thread_number,
        momentum_range,time_samples_path);

    for (int i = 0; i < run_count; ++i) {
        ps.optimize();
    }

    if (chosen_problem) {
        delete chosen_problem;
        chosen_problem = NULL;
    }
    if (chosen_domain) {
        delete chosen_domain;
        chosen_domain = NULL;
    }
}

void policy_evaluator(defined_problems selected_problem, std::string solutions_file_path, std::string results_file_path, unsigned int horizon, unsigned int evaluation_count, unsigned int random_seed, unsigned int thread_number)
{
    Problem* chosen_problem = NULL;
    Domain* chosen_domain = NULL;
    std::string policy_path = "";

    switch (selected_problem) {
    case LM:
        chosen_problem = new LaneMerger();
        chosen_domain = new LaneMergerDomain();
        policy_path = "Example Problems/Lane Merger/Policies/lane_merger_policy3.txt";
        break;
    case GRS:
        chosen_problem = new GraphRockSample();
        chosen_domain = new GraphRockSampleDomain();
        policy_path = "Example Problems/Graph Rock Sample/Policies/graph_rock_sample_policy1.txt";
        break;
    case SR:
        chosen_problem = new SpaceshipRepair();
        chosen_domain = new SpaceshipRepairDomain();
        policy_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
        break;
    case SV:
        chosen_problem = new CityExploration();
        chosen_domain = new CityExplorationDomain();
        policy_path = "Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt";
        break;
    default:

        return;
        break;
    }

    FullBeliefState current_belief(*chosen_domain);
    Policy test_policy(policy_path, *chosen_problem);
    policy_analysis_parrallel(&current_belief, test_policy, *chosen_problem, solutions_file_path, horizon, 1.0f, evaluation_count, results_file_path, random_seed, thread_number);

    if (chosen_problem) {
        delete chosen_problem;
        chosen_problem = NULL;
    }
    if (chosen_domain) {
        delete chosen_domain;
        chosen_domain = NULL;
    }
}

void rcompliant_policy_evaluator(defined_problems selected_problem, std::string results_file_path, unsigned int horizon, unsigned int evaluation_count, unsigned int random_seed, unsigned int thread_number, unsigned int policy_count)
{
    Problem* chosen_problem = NULL;
    Domain* chosen_domain = NULL;
    std::string policy_path = "";

    switch (selected_problem) {
    case LM:
        chosen_problem = new LaneMerger();
        chosen_domain = new LaneMergerDomain();
        policy_path = "Example Problems/Lane Merger/Policies/lane_merger_policy3.txt";
        break;
    case GRS:
        chosen_problem = new GraphRockSample();
        chosen_domain = new GraphRockSampleDomain();
        policy_path = "Example Problems/Graph Rock Sample/Policies/graph_rock_sample_policy1.txt";
        break;
    case SR:
        chosen_problem = new SpaceshipRepair();
        chosen_domain = new SpaceshipRepairDomain();
        policy_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
        break;
    case SV:
        chosen_problem = new CityExploration();
        chosen_domain = new CityExplorationDomain();
        policy_path = "Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt";
        break;
    default:

        return;
        break;
    }

    FullBeliefState current_belief(*chosen_domain);
    Policy test_policy(policy_path, *chosen_problem);
    random_policy_analysis_parrallel(&current_belief, test_policy, *chosen_problem, horizon, 1.0f, evaluation_count, results_file_path, random_seed, thread_number, policy_count);
    if (chosen_problem) {
        delete chosen_problem;
        chosen_problem = NULL;
    }
    if (chosen_domain) {
        delete chosen_domain;
        chosen_domain = NULL;
    }
}

void test_spaceship_worker(Problem* current_problem, Domain* current_domain, BeliefState* current_belief, Policy* current_policy, unsigned int horizon,
    std::mutex* input_lock, std::mutex* output_lock, std::list<std::vector<float>>* input_list, std::list<std::pair<std::vector<float>, float>>* output_list, bool* continue_loop)
{
    Problem* my_problem = current_problem->create_copy();
    Domain* my_domain = current_domain->create_copy();
    BeliefState* my_belief = current_belief->create_copy();
    Policy my_policy(*current_policy);

    std::vector<float> current_input;
    float current_average = 0.0f;
    bool found_input = false;

    while (*continue_loop) {
        input_lock->lock();
        found_input = !input_list->empty();
        if (found_input) {
            current_input = input_list->front();
            input_list->pop_front();
        }
        input_lock->unlock();

        if (found_input) {
            current_average = 0.0f;
            for (int i = 0; i < 300; ++i) {
                current_average += evalPolicyDFS(my_belief, my_policy, *my_problem, current_input, horizon, 1.0f);
            }
            current_average /= 300.0f;
            output_lock->lock();
            output_list->emplace_back(current_input, current_average);
            output_lock->unlock();
        }
    }

    delete my_problem;
    delete my_domain;
    delete my_belief;
}

void test_spaceship_heatmap() {
    SpaceshipRepair test_problem(5u,5u,0.6,0.75);
    SpaceshipRepairDomain test_domain;
    FullBeliefState test_belief(test_domain);
    std::string policy_file_path = "Example Problems/Spaceship Repair/Policies/spaceship_repair_policy.txt";
    std::string solve_path = "Results/heatmap_spaceship_repair.txt";
    unsigned int thread_number = 10u;
    unsigned int horizon = 12u;
    std::vector<float> parameter_values(2);
    float current_average;
    Policy test_policy(policy_file_path, test_problem);
    std::ofstream output_file;

    std::pair<float, float> x_range = { 0.0f, 1.0f }, y_range = { 0.0f, 1.0f };
    //float point_number = 100.0f;

    std::list<std::vector<float>> input_list;
    std::list<std::pair<std::vector<float>, float>> output_list;
    std::mutex input_lock, output_lock;
    bool keep_looping = true;
    std::list<std::thread> thread_list;

    for (int i = 0; i < thread_number; ++i) {
        thread_list.emplace_back(test_spaceship_worker, &test_problem, &test_domain, &test_belief, &test_policy, horizon, &input_lock, &output_lock, &input_list, &output_list, &keep_looping);
    }

    for (float x = 0.0f; x <= 1.0f; x += 0.002) {
        for (float y = 0.0f; y <= 1.0f; y += 0.002) {
            input_lock.lock();
            parameter_values[0] = x;
            parameter_values[1] = y;
            input_list.emplace_back(parameter_values);
            input_lock.unlock();
        }
    }

    int count = 251001;
    bool found_output = false;
    output_file.open(solve_path);
    while (count > 0) {
        output_lock.lock();
        found_output = !output_list.empty();
        if (found_output) {
            parameter_values = output_list.front().first;
            current_average = output_list.front().second;
            output_list.pop_front();
        }
        output_lock.unlock();
        if (found_output) {
            std::cout << parameter_values[0] << "," << parameter_values[1] << "," << current_average << "," << count << std::endl;
            output_file << std::setprecision(4) << parameter_values[0] << "," << parameter_values[1] << ",";
            output_file << std::setprecision(10) << current_average << std::endl;
            --count;
        }
    }
    output_file.close();
    keep_looping = false;
    for (std::list<std::thread>::iterator it = thread_list.begin(); it != thread_list.end(); ++it) {
        it->join();
    }
    thread_list.clear();
}

void run_all_experiments(const std::map<std::string,defined_problems> &problems, const std::map<std::string,interval_solver_type> &solvers) {

    unsigned int evaluation_random_seed = 76421903u;   //Random seed to use for setting the initial states while evaluating.
    unsigned int evaluation_thread_number = 16u;       //Number of worker processes running in parallel while evaluating.
    unsigned int rconfident_policy_count = 10u;        //Number of times to evaluate RConfident for each problem.
    unsigned int solution_evaluation_count = 25000u;   //Number of runs to evaluate solutions on.
    unsigned int solver_run_count = 10u;               //Number of times to run the solver on each problem.
    unsigned int solve_thread_number = 8u;             //Number of worker processes running in parallel while running the solver.
    unsigned int time_slice_evaluation_count = 10000u; //Number of run to evaluate each recorded hypothesized optimal partitions on.

    //Nelder-Mead hyperparameters:
    unsigned int nm_sample_count = 1000u;              //Number of times to evaluate a single parameter point for Nelder-Mead.
    float minimum_separation = 0.4f;                   //Minimum distance between automatically included points in the initial simplex. These points can be used in the case there are not enough points.

    //Solve time given to each problem. The solve is allowed to run for 60 seconds longer in the case the hypothesized optimal partition has less than the minimum number of samples.
    std::map<defined_problems, unsigned int> solve_time = { {LM,120u},{GRS,120u},{SR,30u},{SV,300u} };
    //Number of evenly-spaced samples to take of the hypothesized optimal partition over the run time.
    std::map<defined_problems, unsigned int> samples_to_take = { {LM,240u},{GRS,120u},{SR,240u},{SV,120u} };
    //Horizon for each problem.
    std::map<defined_problems, unsigned int> problem_horizon = { {LM,100u},{GRS,100u},{SR,12u},{SV,100u} };

    unsigned int max_solve_time = 0u, min_solve_time = 0u,time_slice_count = 0u, horizon = 0u;              
    std::string solution_file_path, rconfident_eval_path, interval_file_path, time_slice_path, solutions_eval_path, time_slice_eval_path;
    std::map<defined_problems, unsigned int>::iterator it;
    for (std::map<std::string, defined_problems>::const_iterator problem_it = problems.cbegin(); problem_it != problems.cend(); ++problem_it) {
        std::cout << "Current Problem: " << problem_it->first << std::endl;
        rconfident_eval_path = "Results/RConfident_" + problem_it->first + ".txt";
        it = solve_time.find(problem_it->second);
        if (it == solve_time.end()) {
            std::cout << "Problem does not have a defined solved time!" << std::endl;
            min_solve_time = 1500u;
            max_solve_time = 1560u;
        }
        else {
            min_solve_time = it->second;
            max_solve_time = min_solve_time + 60u;
        }
        it = samples_to_take.find(problem_it->second);
        if (it == samples_to_take.end()) {
            std::cout << "Problem does not have a defined sample count!" << std::endl;
            time_slice_count = 120u;
        }
        else {
            time_slice_count = it->second;
        }
        it = problem_horizon.find(problem_it->second);
        if (it == problem_horizon.end()) {
            std::cout << "Problem does not have a defined horizon!" << std::endl;
            horizon = 100u;
        }
        else {
            horizon = it->second;
        }

        //Run PRS
        for (std::map<std::string, interval_solver_type>::const_iterator solver_it = solvers.cbegin(); solver_it != solvers.cend(); ++solver_it) {
            solution_file_path = "Results/" + solver_it->first + "_" + problem_it->first + "_solutions.txt";
            interval_file_path = "Results/" + solver_it->first + "_" + problem_it->first + "_intervals.txt";
            time_slice_path = "Results/" + solver_it->first + "_" + problem_it->first + "_time_slices.txt";
            solutions_eval_path = "Results/" + solver_it->first + "_" + problem_it->first + "_solutions_evaluated.txt";
            time_slice_eval_path = "Results/" + solver_it->first + "_" + problem_it->first + "_time_slices_evaluated.txt";

            run_solver(problem_it->second, solver_it->second, solution_file_path, interval_file_path, time_slice_path, solver_run_count, horizon, min_solve_time, max_solve_time, solve_thread_number, time_slice_count);
            policy_evaluator(problem_it->second, solution_file_path, solutions_eval_path, horizon, solution_evaluation_count, evaluation_random_seed, evaluation_thread_number);
            policy_evaluator(problem_it->second, time_slice_path, time_slice_eval_path, horizon, time_slice_evaluation_count, evaluation_random_seed, evaluation_thread_number);
        }

        //Run Nelder-Mead
        solution_file_path = "Results/nm_" + problem_it->first + "_solutions.txt";
        time_slice_path = "Results/nm_" + problem_it->first + "_time_slices.txt";
        solutions_eval_path = "Results/nm_" + problem_it->first + "_solutions_evaluated.txt";
        time_slice_eval_path = "Results/nm_" + problem_it->first + "_time_slices_evaluated.txt";
        run_nelder(problem_it->second, solution_file_path, time_slice_path, solver_run_count, horizon, solve_thread_number, nm_sample_count, minimum_separation);
        policy_evaluator(problem_it->second, solution_file_path, solutions_eval_path, horizon, solution_evaluation_count, evaluation_random_seed, evaluation_thread_number);
        policy_evaluator(problem_it->second, time_slice_path, time_slice_eval_path, horizon, time_slice_evaluation_count, evaluation_random_seed, evaluation_thread_number);
        

        //Run Particle Swarm
        solution_file_path = "Results/ps_" + problem_it->first + "_solutions.txt";
        time_slice_path = "Results/ps_" + problem_it->first + "_time_slices.txt";
        solutions_eval_path = "Results/ps_" + problem_it->first + "_solutions_evaluated.txt";
        time_slice_eval_path = "Results/ps_" + problem_it->first + "_time_slices_evaluated.txt";
        run_particle(problem_it->second, solution_file_path, time_slice_path, solver_run_count, horizon, solve_thread_number);
        policy_evaluator(problem_it->second, solution_file_path, solutions_eval_path, horizon, solution_evaluation_count, evaluation_random_seed, evaluation_thread_number);
        policy_evaluator(problem_it->second, time_slice_path, time_slice_eval_path, horizon, time_slice_evaluation_count, evaluation_random_seed, evaluation_thread_number);


        rcompliant_policy_evaluator(problem_it->second, rconfident_eval_path, horizon, solution_evaluation_count, evaluation_random_seed, evaluation_thread_number, rconfident_policy_count);
    }

    test_spaceship_heatmap();
}

int main()
{
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
    _CrtMemCheckpoint(&sOld);

    // Set of defined problems
    std::map<std::string, defined_problems> problems = { {"lm",LM},{"grs",GRS},{"sv",SV},{"sr",SR} };

    // Set of defined Partition Refinement Search partition selection approaches.
    std::map<std::string, interval_solver_type> solvers = { {"gts",global_thompson},{"lts",local_thompson},{"epsilon",epsilon_greedy},{"bolt",softmax},{"max",certainty_max} };

    run_all_experiments(problems,solvers);

    _CrtMemCheckpoint(&sNew);
    if (_CrtMemDifference(&sDiff, &sOld, &sNew))
    {
        OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
        _CrtMemDumpStatistics(&sDiff);
        OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
        _CrtMemDumpAllObjectsSince(&sOld);
        OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
        _CrtDumpMemoryLeaks();
    }

    return 0;
}

#include "policy_analysis.h"
#include<random>
#include<chrono>
#include<fstream>
#include<iostream>
#include<algorithm>
#include<exception>
#include <thread>

void policy_analysis(BeliefState* belief_state, const Policy& policy, const Problem& problem, const std::string& solution_file_path,
    unsigned int horizon, float discount_factor, unsigned int samples, std::string file_path, unsigned int random_seed)
{

    std::vector<std::vector<std::vector<float>>> solutions;
    parse_results_file(solution_file_path, solutions);

    if (solutions.empty()) {
        return;
    }

    belief_state->build_initial_belief(problem);
    std::list<DynamicState> different_states;
    std::default_random_engine generator;
    std::uniform_real_distribution<> state_selecter(0, 1);
    generator.seed(random_seed);
    float selected_prob = 0.0f;
    std::list<DynamicState>::iterator state_it;
    std::list<float>::iterator prob_it;

    for (int i = 0; i < samples; ++i) {
        state_it = belief_state->state_list.begin();
        prob_it = belief_state->probabilities.begin();
        selected_prob = state_selecter(generator) - *prob_it;
        while (selected_prob > 0.0f && state_it != belief_state->state_list.end()) {
            ++state_it;
            ++prob_it;
            selected_prob -= *prob_it;
        }
        different_states.emplace_back(*state_it);
    }

    std::list<float> reached_goal, expected_cost, upheld_policy;
    float completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;
    Observation current_observation;
    bool is_otherwise_action = true;
    unsigned int selected_orthotope = 0u;
    Action otherwise_action;
    DynamicState current_eval_state;
    std::vector<DynamicState> next_state_dist;
    std::vector<float> next_state_probs;
    std::ofstream out_file;

    out_file.open(file_path);
    out_file << "Samples: " << samples << std::endl;
    out_file << "Random Seed: " << random_seed << std::endl;
    out_file << "Average Expected Cost,Expected Cost Deviation,Percent Completion Rate,Completion Rate Deviation, Percent Preference Satisfaction, Preference Satisfication Deviation\n";
    out_file.close();

    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::vector<std::vector<std::vector<float>>>::const_iterator tuple_it = solutions.cbegin(); tuple_it != solutions.cend(); ++tuple_it) {
        reached_goal.clear();
        expected_cost.clear();
        upheld_policy.clear();
        std::cout << "ping" << std::endl;
        for (state_it = different_states.begin(); state_it != different_states.end(); ++state_it) {
            belief_state->build_initial_belief(problem);
            current_eval_state = *state_it;
            selected_orthotope = std::uniform_int_distribution<>(0, tuple_it->size() - 1)(generator);
            for (unsigned int current = 0; current < horizon; ++current) {
                std::cout << current_eval_state.integer_params[0] << std::endl;
                std::cout << "{";
                for (std::list<DynamicState>::iterator it = belief_state->state_list.begin(); it != belief_state->state_list.end(); ++it) {
                    std::cout << it->integer_params[0] << " ";
                }
                std::cout << "}" << std::endl;
                policy.build_policy_distribution(belief_state->static_state, belief_state->state_list, belief_state->probabilities, current_constraint_list, current_action_list, otherwise_action);
                is_otherwise_action = true;
                std::cout << "=========" << std::endl;
                for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
                    ++constraint_it, ++action_it) {
                    std::cout << *constraint_it << " --> " << action_it->action_name << std::endl;
                }
                std::cout << "----" << std::endl;
                for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
                    ++constraint_it, ++action_it) {
                    if (constraint_it->is_satisfied((*tuple_it)[selected_orthotope])) {

                        belief_state->apply_action(*action_it);
                        std::cout << *constraint_it << " --> " << action_it->action_name << std::endl;

                        belief_state->problem_domain->update_state(current_eval_state, belief_state->static_state, *action_it, generator);

                        if (belief_state->problem_domain->can_create_observation(*action_it)) {
                            belief_state->problem_domain->sample_observation(current_eval_state, belief_state->static_state, *action_it, current_observation, generator);
                            belief_state->apply_observation(*action_it, current_observation);
                        }
                        is_otherwise_action = false;
                        break;
                    }
                }
                if (is_otherwise_action) {
                    std::cout << otherwise_action.action_name << std::endl;
                    belief_state->apply_action(otherwise_action);
                    belief_state->problem_domain->update_state(current_eval_state, belief_state->static_state, otherwise_action, generator);
                    if (belief_state->problem_domain->can_create_observation(otherwise_action)) {
                        belief_state->problem_domain->sample_observation(current_eval_state, belief_state->static_state, otherwise_action, current_observation, generator);
                        belief_state->apply_observation(otherwise_action, current_observation);
                    }
                }
                if (current_eval_state.terminated) {
                    expected_cost.push_back(current + 1.0f);
                    reached_goal.push_back(1.0f);
                    upheld_policy.push_back(1.0f);
                    break;
                }
                else if (current_eval_state.stuck) {
                    expected_cost.push_back(horizon);
                    reached_goal.push_back(0.0f);
                    upheld_policy.push_back(0.0f);
                    break;
                }
            }
            if (!current_eval_state.terminated && !current_eval_state.stuck) {
                expected_cost.push_back(horizon);
                reached_goal.push_back(0.0f);
                upheld_policy.push_back(1.0f);
            }
        }
        completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate += *rate_it;
            average_expected_cost += *ec_it;
            average_upheld_policy += *upheld_it;
        }
        completion_rate /= samples;
        average_expected_cost /= samples;
        average_upheld_policy /= samples;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate_dev += powf(*rate_it - completion_rate, 2.0f);
            expected_cost_dev += powf(*ec_it - average_expected_cost, 2.0f);
            upheld_policy_dev += powf(*upheld_it - average_upheld_policy, 2.0f);
        }

        completion_rate_dev /= samples;
        expected_cost_dev /= samples;
        upheld_policy_dev /= samples;

        completion_rate_dev = powf(completion_rate_dev, 0.5f);
        expected_cost_dev = powf(expected_cost_dev, 0.5f);
        upheld_policy_dev = powf(upheld_policy_dev, 0.5f);

        out_file.open(file_path, std::ios::app);
        out_file << average_expected_cost << "," << expected_cost_dev << "," << completion_rate << "," << completion_rate_dev << "," << average_upheld_policy << "," << upheld_policy_dev << "\n";
        out_file.close();
    }
}

void parse_results_file(const std::string &file_path, std::vector<std::vector<std::vector<float>>> &out)
{
    std::ifstream input_file;
    std::string current_line, temp_str, stemp_str;
    std::size_t found_index, comma_index;
    int dimensions = -1;
    input_file.open(file_path);
    out.clear();

    while (std::getline(input_file, current_line)) {
        std::replace(current_line.begin(), current_line.end(), ')', ']');
        found_index = current_line.find_last_of(',');
        std::string time_value = current_line.substr(found_index + 1,current_line.size());
        //std::cout << time_value << std::endl;
        // if(std::stoi(time_value) != 600){
        //     continue;
        // }
        found_index = current_line.find_first_of('}');
        out.emplace_back();
        while (found_index != std::string::npos) {
            out.back().emplace_back();
            temp_str = current_line.substr(1, found_index - 1);
            current_line.erase(0, found_index + 1);
            found_index = temp_str.find_first_of(']');
            while (found_index != std::string::npos) {
                stemp_str = temp_str.substr(1, found_index - 1);
                temp_str.erase(0, found_index + 1);
                comma_index = stemp_str.find_first_of(',');
                if (comma_index == std::string::npos) {
                    throw std::invalid_argument("Missing comma.");
                }

                out.back().back().emplace_back(std::stof(stemp_str.substr(0,comma_index)));
                stemp_str.erase(0, comma_index + 1);
                out.back().back().back() += std::stof(stemp_str);
                out.back().back().back() /= 2.0f;
                found_index = temp_str.find_first_of(']');
            }
            if (out.back().back().size() == 0) {
                throw std::invalid_argument("Failed to parse solution file.");
            }
            if (dimensions == -1) {
                dimensions = out.back().back().size();
            }
            else if(dimensions != out.back().back().size()){
                throw std::invalid_argument("Dimension size isn't constant.");
            }

            found_index = current_line.find_first_of('}');
        }
    }
    input_file.close();
}

void policy_analysis_worker(BeliefState *belief_state, const Policy *policy, const Problem *problem, unsigned int horizon, 
    float discount_factor, int random_seed, std::mutex *input_lock, std::mutex *output_lock, std::list<DynamicState> *input, 
    std::list<float> *reached_goal, std::list<float> *expected_cost, std::list<float> *upheld_policy, bool *worker_loop, 
    std::vector<std::vector<float>>*solutions)
{
    BeliefState* worker_belief = belief_state->create_copy();
    Policy worker_pol(*policy);
    Problem* worker_problem = problem->create_copy();
    std::default_random_engine generator;
    generator.seed(random_seed);
    Observation current_observation;
    bool is_otherwise_action = true, found_input = false;
    unsigned int selected_orthotope;
    Action otherwise_action;
    DynamicState current_eval_state;
    std::vector <std::vector<float>> current_solution = *solutions;
    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;

    while (*worker_loop) {
        input_lock->lock();
        found_input = !input->empty();
        if (found_input) {
            current_eval_state = input->front();
            input->pop_front();
        }
        input_lock->unlock();
        if (!found_input) {
            continue;
        }
        selected_orthotope = selected_orthotope = std::uniform_int_distribution<>(0, current_solution.size() - 1)(generator);
        worker_belief->build_initial_belief(*worker_problem);
        for (unsigned int current = 0; current < horizon; ++current) {
            //std::cout << current_eval_state.integer_params[0] << std::endl;
            worker_pol.build_policy_distribution(worker_belief->static_state, worker_belief->state_list, worker_belief->probabilities, current_constraint_list, current_action_list, otherwise_action);
            is_otherwise_action = true;
            /*std::cout << "=========" << std::endl;
            for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
                ++constraint_it, ++action_it) {
                std::cout << *constraint_it << " --> " << action_it->action_name << std::endl;
            }
            std::cout << "----" << std::endl;*/
            for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
                ++constraint_it, ++action_it) {
                if (constraint_it->is_satisfied(current_solution[selected_orthotope])) {

                    worker_belief->apply_action(*action_it);
                    //std::cout << *constraint_it << " --> " << action_it->action_name << std::endl;

                    worker_belief->problem_domain->update_state(current_eval_state, worker_belief->static_state, *action_it, generator);

                    if (worker_belief->problem_domain->can_create_observation(*action_it)) {
                        worker_belief->problem_domain->sample_observation(current_eval_state, worker_belief->static_state, *action_it, current_observation, generator);
                        worker_belief->apply_observation(*action_it, current_observation);
                    }
                    is_otherwise_action = false;
                    break;
                }
            }
            if (is_otherwise_action) {
                worker_belief->apply_action(otherwise_action);
                worker_belief->problem_domain->update_state(current_eval_state, worker_belief->static_state, otherwise_action, generator);
                if (worker_belief->problem_domain->can_create_observation(otherwise_action)) {
                    worker_belief->problem_domain->sample_observation(current_eval_state, worker_belief->static_state, otherwise_action, current_observation, generator);
                    worker_belief->apply_observation(otherwise_action, current_observation);
                }
            }
            if (current_eval_state.terminated) {
                output_lock->lock();
                expected_cost->push_back(current + 1.0f);
                reached_goal->push_back(1.0f);
                upheld_policy->push_back(1.0f);
                output_lock->unlock();
                break;
            }
            else if (current_eval_state.stuck) {
                output_lock->lock();
                expected_cost->push_back(horizon);
                reached_goal->push_back(0.0f);
                upheld_policy->push_back(0.0f);
                output_lock->unlock();
                break;
            }
        }
        if (!current_eval_state.terminated && !current_eval_state.stuck) {
            output_lock->lock();
            expected_cost->push_back(horizon);
            reached_goal->push_back(0.0f);
            upheld_policy->push_back(1.0f);
            output_lock->unlock();
        }
    }

    delete worker_belief;
    worker_belief = nullptr;
    delete worker_problem;
    worker_problem = nullptr;
}

void policy_analysis_parrallel(BeliefState* belief_state, const Policy& policy, const Problem& problem, const std::string& solution_file_path,
    unsigned int horizon, float discount_factor, unsigned int samples, std::string file_path, unsigned int random_seed, unsigned int thread_number)
{
    std::vector<std::vector<std::vector<float>>> solutions;
    parse_results_file(solution_file_path, solutions);


    if (solutions.empty()) {
        return;
    }

    belief_state->build_initial_belief(problem);
    std::list<DynamicState> different_states;
    std::default_random_engine generator;
    std::uniform_real_distribution<> state_selecter(0, 1);
    generator.seed(random_seed);
    float selected_prob = 0.0f;
    std::list<DynamicState>::iterator state_it;
    std::list<float>::iterator prob_it;

    for (int i = 0; i < samples; ++i) {
        state_it = belief_state->state_list.begin();
        prob_it = belief_state->probabilities.begin();
        selected_prob = state_selecter(generator) - *prob_it;
        while (selected_prob > 0.0f && state_it != belief_state->state_list.end()) {
            ++state_it;
            ++prob_it;
            selected_prob -= *prob_it;
        }
        different_states.emplace_back(*state_it);
    }

    std::list<float> reached_goal, expected_cost, upheld_policy;
    float completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;
    Observation current_observation;
    bool is_otherwise_action = true;
    unsigned int selected_orthotope = 0u;
    Action otherwise_action;
    DynamicState current_eval_state;
    std::vector<DynamicState> next_state_dist;
    std::vector<float> next_state_probs;
    std::ofstream out_file;
    std::vector<std::thread> threads;
    std::mutex input_lock, output_lock;
    std::list<DynamicState> inputs;
    bool continue_looping = true;

    out_file.open(file_path);
    out_file << "Samples: " << samples << std::endl;
    out_file << "Random Seed: " << random_seed << std::endl;
    out_file << "Average Expected Cost,Expected Cost Deviation,Percent Completion Rate,Completion Rate Deviation, Percent Preference Satisfaction, Preference Satisfication Deviation\n";
    out_file.close();
    std::vector<std::vector<float>> temp_vec;

    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::vector<std::vector<std::vector<float>>>::const_iterator tuple_it = solutions.cbegin(); tuple_it != solutions.cend(); ++tuple_it) {
        reached_goal.clear();
        expected_cost.clear();
        upheld_policy.clear();
        continue_looping = true;
        temp_vec = *tuple_it;
        for (int i = 0; i < thread_number; ++i) {
            threads.emplace_back(policy_analysis_worker, belief_state, &policy, &problem, horizon, discount_factor,
                (int)std::uniform_int_distribution<>(100000,999999)(generator),&input_lock,&output_lock,&inputs,
                &reached_goal,&expected_cost,&upheld_policy,&continue_looping,&temp_vec);
        }
        for (state_it = different_states.begin(); state_it != different_states.end(); ++state_it) {
            input_lock.lock();
            inputs.push_back(*state_it);
            input_lock.unlock();
        }
        while (reached_goal.size() < samples) {
        //    std::cout << reached_goal.size() << std::endl;
        }
        continue_looping = false;
        for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); ++it) {
            it->join();
        }
        threads.clear();
        completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate += *rate_it;
            average_expected_cost += *ec_it;
            average_upheld_policy += *upheld_it;
        }
        completion_rate /= samples;
        average_expected_cost /= samples;
        average_upheld_policy /= samples;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate_dev += powf(*rate_it - completion_rate, 2.0f);
            expected_cost_dev += powf(*ec_it - average_expected_cost, 2.0f);
            upheld_policy_dev += powf(*upheld_it - average_upheld_policy, 2.0f);
        }

        completion_rate_dev /= samples;
        expected_cost_dev /= samples;
        upheld_policy_dev /= samples;

        completion_rate_dev = powf(completion_rate_dev, 0.5f);
        expected_cost_dev = powf(expected_cost_dev, 0.5f);
        upheld_policy_dev = powf(upheld_policy_dev, 0.5f);

        out_file.open(file_path, std::ios::app);
        out_file << average_expected_cost << "," << expected_cost_dev << "," << completion_rate << "," << completion_rate_dev << "," << average_upheld_policy << "," << upheld_policy_dev << "\n";
        out_file.close();
    }
}

void random_policy_analysis_parrallel(BeliefState* belief_state, const Policy& policy, const Problem& problem, 
    unsigned int horizon, float discount_factor, unsigned int samples, std::string file_path, unsigned int random_seed, unsigned int thread_number, unsigned int count)
{
    std::uniform_real_distribution<> state_selecter(0, 1);
    std::vector<std::vector<std::vector<float>>> solutions;
    std::default_random_engine generator;
    generator.seed(random_seed);

    belief_state->build_initial_belief(problem);
    std::list<DynamicState> different_states;
    float selected_prob = 0.0f;
    std::list<DynamicState>::iterator state_it;
    std::list<float>::iterator prob_it;

    for (int i = 0; i < samples; ++i) {
        state_it = belief_state->state_list.begin();
        prob_it = belief_state->probabilities.begin();
        selected_prob = state_selecter(generator) - *prob_it;
        while (selected_prob > 0.0f && state_it != belief_state->state_list.end()) {
            ++state_it;
            ++prob_it;
            selected_prob -= *prob_it;
        }
        different_states.emplace_back(*state_it);
    }

    OrthotopeList parameter_ranges;
    policy.get_full_parameter_range(parameter_ranges);

    solutions.resize(count);
    for(int i = 0; i < count; ++i){
        solutions[i].resize(1);
        parameter_ranges.uniform_sample_point(generator,solutions[i][0]);
        for(std::vector<float>::iterator it = solutions[i][0].begin(); it != solutions[i][0].end(); ++it){
            std::cout << *it << std::endl;
        }
        std::cout << "---" << std::endl;
    }


    std::list<float> reached_goal, expected_cost, upheld_policy;
    float completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;
    Observation current_observation;
    bool is_otherwise_action = true;
    unsigned int selected_orthotope = 0u;
    Action otherwise_action;
    DynamicState current_eval_state;
    std::vector<DynamicState> next_state_dist;
    std::vector<float> next_state_probs;
    std::ofstream out_file;
    std::vector<std::thread> threads;
    std::mutex input_lock, output_lock;
    std::list<DynamicState> inputs;
    bool continue_looping = true;

    out_file.open(file_path);
    out_file << "Samples: " << samples << std::endl;
    out_file << "Random Seed: " << random_seed << std::endl;
    out_file << "Average Expected Cost,Expected Cost Deviation,Percent Completion Rate,Completion Rate Deviation, Percent Preference Satisfaction, Preference Satisfication Deviation\n";
    out_file.close();
    std::vector<std::vector<float>> temp_vec;

    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    for (std::vector<std::vector<std::vector<float>>>::const_iterator tuple_it = solutions.cbegin(); tuple_it != solutions.cend(); ++tuple_it) {
        reached_goal.clear();
        expected_cost.clear();
        upheld_policy.clear();
        continue_looping = true;
        temp_vec = *tuple_it;
        for (int i = 0; i < thread_number; ++i) {
            threads.emplace_back(policy_analysis_worker, belief_state, &policy, &problem, horizon, discount_factor,
                (int)std::uniform_int_distribution<>(100000,999999)(generator),&input_lock,&output_lock,&inputs,
                &reached_goal,&expected_cost,&upheld_policy,&continue_looping,&temp_vec);
        }
        for (state_it = different_states.begin(); state_it != different_states.end(); ++state_it) {
            input_lock.lock();
            inputs.push_back(*state_it);
            input_lock.unlock();
        }
        while (reached_goal.size() < samples) {
            std::cout << reached_goal.size() << std::endl;
        }
        continue_looping = false;
        for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); ++it) {
            it->join();
        }
        threads.clear();
        completion_rate = 0.0f, completion_rate_dev = 0.0f, average_expected_cost = 0.0f, expected_cost_dev = 0.0f, average_upheld_policy = 0.0f, upheld_policy_dev = 0.0f;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate += *rate_it;
            average_expected_cost += *ec_it;
            average_upheld_policy += *upheld_it;
        }
        completion_rate /= samples;
        average_expected_cost /= samples;
        average_upheld_policy /= samples;
        for (std::list<float>::iterator ec_it = expected_cost.begin(), rate_it = reached_goal.begin(), upheld_it = upheld_policy.begin(); ec_it != expected_cost.end(); ++ec_it, ++rate_it, ++upheld_it) {
            completion_rate_dev += powf(*rate_it - completion_rate, 2.0f);
            expected_cost_dev += powf(*ec_it - average_expected_cost, 2.0f);
            upheld_policy_dev += powf(*upheld_it - average_upheld_policy, 2.0f);
        }

        completion_rate_dev /= samples;
        expected_cost_dev /= samples;
        upheld_policy_dev /= samples;

        completion_rate_dev = powf(completion_rate_dev, 0.5f);
        expected_cost_dev = powf(expected_cost_dev, 0.5f);
        upheld_policy_dev = powf(upheld_policy_dev, 0.5f);

        out_file.open(file_path, std::ios::app);
        out_file << average_expected_cost << "," << expected_cost_dev << "," << completion_rate << "," << completion_rate_dev << "," << average_upheld_policy << "," << upheld_policy_dev << "\n";
        out_file.close();
    }
}
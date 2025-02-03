#include "full_belief_state.h"
#include<chrono>
#include<random>
#include<iostream>

FullBeliefState::FullBeliefState(Domain& prob_domain) {
    problem_domain = &prob_domain;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

FullBeliefState::~FullBeliefState() {}

void FullBeliefState::apply_action(const Action& ref) {
    std::list<DynamicState> next_states;
    std::list<float> next_probs;
    std::vector<DynamicState> temp_states;
    std::vector<float> temp_probs;
    std::list<DynamicState>::iterator next_state_it;
    std::vector<DynamicState>::iterator temp_state_it;
    std::list<float>::iterator next_prob_it;
    std::vector<float>::iterator temp_prob_it;
    bool not_placed = true;
    std::list<float>::iterator prob_it = probabilities.begin();
    //goal_state_prob = 0.0f;
    //terminated = true;
    for (std::list<DynamicState>::iterator it = state_list.begin(); it != state_list.end(); ++it, ++prob_it) {
        if (problem_domain->next_state_distribution(*it, static_state, ref, temp_states, temp_probs)) {
            for (temp_state_it = temp_states.begin(), temp_prob_it = temp_probs.begin(); temp_state_it != temp_states.end(); ++temp_state_it, ++temp_prob_it) {
                // next_state_it = next_states.begin();
                // next_prob_it = next_probs.begin();
                // not_placed = true;
                //temp_state_it->rewards.push_back(problem_domain->reward_function(static_state,*it,ref,*temp_state_it));
                // for(;next_state_it != next_states.end() && not_placed; ++next_state_it, ++next_prob_it){

                //     if(*temp_state_it == *next_state_it){
                //         *next_prob_it += (*temp_prob_it) * (*prob_it);
                //         not_placed = false;
                //     }
                // }
                // if(not_placed){
                //     next_states.push_back(*temp_state_it);
                //     next_probs.push_back((*temp_prob_it) * (*prob_it));
                // }
                /*if (temp_state_it->terminated) {
                    goal_state_prob += *temp_prob_it;
                }
                else if (!temp_state_it->stuck) {
                    terminated = false;
                }*/
                add_to_sorted(next_states, next_probs, *temp_state_it, (*temp_prob_it) * (*prob_it));
            }
        }
    }
    if (next_states.size() == 0) {
        // std::cout << state_list.begin()->terminated << std::endl;
        // std::cout << state_list.begin()->rewards.size() << std::endl;
        std::cout << ref.action_name << std::endl;
    }
    state_list.swap(next_states);
    probabilities.swap(next_probs);
}

void FullBeliefState::apply_observation(const Action& action_ref, const Observation& ref) {
    float total = 0.0f;
    std::list<float>::iterator prob_it = probabilities.begin();
    std::list<DynamicState>::iterator it = state_list.begin();
    //terminated = true;
    //goal_state_prob = 0.0f;
    while (it != state_list.end()) {
        *prob_it *= problem_domain->observation_function(*it, static_state, action_ref, ref);
        if (*prob_it <= 0.0f) {
            it = state_list.erase(it);
            prob_it = probabilities.erase(prob_it);
        }
        else {
            total += *prob_it;
            /*if (it->terminated) {
                goal_state_prob += *prob_it;
            }
            else if(!it->stuck){
                terminated = false;
            }*/

            ++it;
            ++prob_it;
        }
    }
    if (total != 0.0f && state_list.size() > 0) {
        for (prob_it = probabilities.begin(); prob_it != probabilities.end(); ++prob_it) {
            *prob_it /= total;
        }
    }
}

void FullBeliefState::build_initial_belief(const Problem& ref) {
    terminated = false;
    ref.generate_initial_belief(static_state, state_list, probabilities);
}

BeliefState* FullBeliefState::create_copy() const {
    return new FullBeliefState(*this);
}

void FullBeliefState::generate_observation(const Action& action_ref, Observation& observation_out) {
    std::vector<Observation> observation_dist;
    std::vector<float> probability_dist;
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::list<DynamicState>::const_iterator it = state_list.cbegin();
    std::list<float>::const_iterator state_prob_it = probabilities.cbegin();
    float generated_value = dist(generator);
    generated_value -= *state_prob_it;
    int j = 0;
    while (it != state_list.cend() && generated_value > 0.0f) {
        ++it;
        ++state_prob_it;
        generated_value -= *state_prob_it;
    }

    generated_value = dist(generator);
    problem_domain->observation_distribution(*it, static_state, action_ref, observation_dist, probability_dist);
    std::vector<Observation>::const_iterator observation_it = observation_dist.cbegin();
    std::vector<float>::const_iterator prob_it = probability_dist.cbegin();
    generated_value -= *prob_it;
    while (generated_value > 0.0f && observation_it != observation_dist.cend()) {
        ++observation_it;
        ++prob_it;
        generated_value -= *prob_it;
    }
    if(observation_it == observation_dist.cend()){
    }
    observation_out = *observation_it;
}
#include "belief_state.h"
#include <chrono>
#include <iostream>

BeliefState::~BeliefState() {};

void BeliefState::all_possible_observations(const Action& ref, std::list<Observation>& possible_observations, std::list<float>& probs) const {
    possible_observations.clear();
    probs.clear();
    std::list<DynamicState>::const_iterator state_it = state_list.cbegin();
    std::list<float>::const_iterator  prob_it = probabilities.cbegin();

    std::vector<Observation> temp_observations;
    std::vector<float> temp_probs;
    std::vector<Observation>::iterator temp_it;
    std::vector<float>::iterator temp_prob_it;

    std::list<Observation>::iterator obs_it;
    std::list<float>::iterator  out_prob_it;
    bool no_match = true;

    for (; state_it != state_list.cend(); ++state_it, ++prob_it) {
        if (problem_domain->observation_distribution(*state_it, static_state, ref, temp_observations, temp_probs)) {
            for (temp_it = temp_observations.begin(), temp_prob_it = temp_probs.begin(); temp_it != temp_observations.end(); ++temp_it, ++temp_prob_it) {
                // no_match = true;
                // for(obs_it = possible_observations.begin(), out_prob_it = probs.begin(); obs_it != possible_observations.end() && no_match; ++obs_it, ++out_prob_it){
                //     if(*obs_it == *temp_it){
                //         no_match = false;
                //         *out_prob_it += (*temp_prob_it) * (*prob_it);
                //     }
                // }
                // if(no_match){
                //     possible_observations.emplace_back(*temp_it);
                //     probs.emplace_back((*temp_prob_it) * (*prob_it));
                // }
                add_to_sorted(possible_observations, probs, *temp_it, (*temp_prob_it) * (*prob_it));
            }
        }
    }
}

float BeliefState::calculate_expected_value(float discount_factor) const {
    std::list<DynamicState>::const_iterator state_it = state_list.cbegin();
    std::list<float>::const_iterator  prob_it = probabilities.cbegin();
    float total = 0.0f;
    for (; state_it != state_list.cend(); ++state_it, ++prob_it) {
        total += state_it->calculate_expected_value(discount_factor) * (*prob_it);
    }
    return total;
}

/*void BeliefState::update_terminated() {
    std::list<DynamicState>::iterator state_it = state_list.begin();
    std::list<float>::iterator prob_it = probabilities.begin();
    float terminate_prob = 0.0f;
    goal_state_prob = 0.0f;
    for (; state_it != state_list.cend(); ++state_it, ++prob_it) {
        if (state_it->terminated) {
            terminate_prob += *prob_it;
            goal_state_prob += *prob_it;
        }
        else if (state_it->stuck) {
            terminate_prob += *prob_it;
        }
    }
    if (terminate_prob <= 0.0f) {
        terminated = false;
    }
    else if (terminate_prob >= 1.0f) {
        terminated = true;
    }
    else {
        terminated = terminate_prob >= std::uniform_real_distribution<>(0.0, 1.0)(generator);
        state_it = state_list.begin();
        prob_it = probabilities.begin();
        if (!terminated) {
            terminate_prob = 1.0 - terminate_prob;
            goal_state_prob = 0.0f;
        }
        while (state_it != state_list.end()) {
            if ((state_it->terminated || state_it->stuck) != terminated) {
                state_it = state_list.erase(state_it);
                prob_it = probabilities.erase(prob_it);
            }
            else {
                *prob_it /= terminate_prob;
                ++state_it;
                ++prob_it;
            }
        }
    }
}*/

void BeliefState::calculate_termination_stuck_percent()
{
    std::list<DynamicState>::iterator state_it = state_list.begin();
    std::list<float>::iterator prob_it = probabilities.begin();
    goal_state_prob = 0.0f;
    terminated = true;

    for (; state_it != state_list.cend(); ++state_it, ++prob_it) {
        if (state_it->terminated) {
            goal_state_prob += *prob_it;
        }
        else if (!state_it->stuck) {
            terminated = false;
        }
    }
}
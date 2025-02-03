#include "particle_belief_state.h"
#include <iostream>
#include<chrono>

PFBeliefState::PFBeliefState(unsigned int particles, Domain& prob_domain) {
    particle_number = particles;
    problem_domain = &prob_domain;
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

PFBeliefState::~PFBeliefState() {}

void PFBeliefState::apply_action(const Action& ref) {
    std::vector<DynamicState> next_states;
    std::vector<DynamicState>::iterator state_it;
    std::vector<float> next_probs;
    std::vector<float>::iterator prob_it;
    float temp = 0.0f, temp_rewards = 0.0f;
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    for (std::list<DynamicState>::iterator it = state_list.begin(); it != state_list.end(); ++it) {
        if (problem_domain->next_state_distribution(*it, static_state, ref, next_states, next_probs)) {
            temp = dist(generator);

            state_it = next_states.begin();
            prob_it = next_probs.begin();
            for (; state_it != next_states.end(); ++state_it, ++prob_it) {
                temp -= *prob_it;
                if (temp <= 0.0f) {
                    temp_rewards = problem_domain->reward_function(static_state, *it, ref, *state_it);
                    *it = *state_it;
                    it->rewards.push_back(temp_rewards);
                    break;
                }
            }
        }
    }
}
void PFBeliefState::apply_observation(const Action& action_ref, const Observation& ref) {
    float sum = 0.0f, total = 0.0f;
    std::list<DynamicState>::iterator state_it = state_list.begin(), temp_state_it = temp_states.begin();
    std::list<float>::iterator prob_it = probabilities.begin(), temp_prob_it = temp_probs.begin();
    std::uniform_real_distribution<float> dist(0.0, 1.0);

    for (; state_it != state_list.end(); ++state_it, ++prob_it, ++temp_prob_it) {
        *prob_it = problem_domain->observation_function(*state_it, static_state, action_ref, ref);
        sum += *prob_it;
        *temp_prob_it = dist(generator);
    }
    temp_probs.sort();

    temp_prob_it = temp_probs.begin();
    state_it = state_list.begin();
    prob_it = probabilities.begin();

    total += *prob_it;

    terminated = true;
    for (; temp_state_it != temp_states.end(); ++temp_state_it, ++temp_prob_it) {
        *temp_prob_it *= sum;
        while (*temp_prob_it > total && state_it != state_list.end()) {
            ++state_it;
            ++prob_it;
            total += *prob_it;
        }
        *temp_prob_it = 1.0f / particle_number;
        *temp_state_it = *state_it;
        if (terminated) {
            terminated = temp_state_it->terminated;
        }
    }
    state_list.swap(temp_states);
    probabilities.swap(temp_probs);
}
void PFBeliefState::build_initial_belief(const Problem& ref) {
    std::list<DynamicState> all_initial_states;
    std::list<float> state_probabilities;
    ref.generate_initial_belief(static_state, all_initial_states, state_probabilities);
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    terminated = false;


    if (state_list.size() != particle_number) {
        state_list.resize(particle_number);
        probabilities.resize(particle_number);
        temp_states.resize(particle_number);
        temp_probs.resize(particle_number);
    }

    for (std::list<float>::iterator it = probabilities.begin(); it != probabilities.end(); ++it) {
        *it = dist(generator);
    }
    probabilities.sort();

    std::list<DynamicState>::iterator state_it = state_list.begin();
    std::list<DynamicState>::iterator init_state_it = all_initial_states.begin();
    std::list<float>::iterator init_state_probs = state_probabilities.begin();
    float current_prob = *init_state_probs;
    for (std::list<float>::iterator prob_it = probabilities.begin(); state_it != state_list.end(); ++state_it, ++prob_it) {
        while (*prob_it > current_prob && init_state_it != all_initial_states.end()) {
            ++init_state_it;
            ++init_state_probs;
            current_prob += *init_state_probs;
        }
        *prob_it = 1.0f / particle_number;
        *state_it = *init_state_it;
    }
}

BeliefState* PFBeliefState::create_copy() const {
    return new PFBeliefState(*this);
}

void PFBeliefState::generate_observation(const Action& action_ref, Observation& observation_out) {
    std::vector<Observation> observation_dist;
    std::vector<float> probability_dist;

    std::list<DynamicState>::const_iterator it = state_list.cbegin();
    std::advance(it, std::uniform_int_distribution<>(0, state_list.size() - 1)(generator));
    float generated_value = std::uniform_real_distribution<>(0.0, 1.0)(generator);

    problem_domain->observation_distribution(*it, static_state, action_ref, observation_dist, probability_dist);
    std::vector<Observation>::const_iterator observation_it = observation_dist.cbegin();
    std::vector<float>::const_iterator prob_it = probability_dist.cbegin();
    generated_value -= *prob_it;
    while (generated_value > 0.0f && observation_it != observation_dist.cend()) {
        ++observation_it;
        ++prob_it;
        generated_value -= *prob_it;
    }
    observation_out = *observation_it;
}
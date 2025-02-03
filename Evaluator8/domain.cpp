#include "domain.h"
#include <iostream>

//TODO compute_observation_prob
float Domain::observation_function(const DynamicState& dynamic_state_ref, const StaticState& const_state_ref, const Action& action_ref,
    const Observation& observation_ref) const {
    std::vector<Observation> obs_dist;
    std::vector<float> obs_probs;

    if (!observation_distribution(dynamic_state_ref, const_state_ref, action_ref, obs_dist, obs_probs)) {
        return 0.0f;
    }

    std::vector<Observation>::iterator obs_it = obs_dist.begin();
    std::vector<float>::iterator prob_it = obs_probs.begin();

    for (; obs_it != obs_dist.end(); ++obs_it, ++prob_it) {

        if (*obs_it == observation_ref) {
            return *prob_it;
        }
    }
    return 0.0f;
}

bool Domain::sample_observation(const DynamicState& current_state, const StaticState& static_state, const Action& action, Observation& out, std::default_random_engine& generator) {
    std::vector<Observation> obs_dist;
    std::vector<float> obs_probs;

    if (!observation_distribution(current_state, static_state, action, obs_dist, obs_probs)) {
        return false;
    }

    std::vector<Observation>::iterator obs_it = obs_dist.begin();
    std::vector<float>::iterator prob_it = obs_probs.begin();
    float chosen_prob = std::uniform_real_distribution<>(0, 1)(generator) - *prob_it;
    while (chosen_prob > 0.0f && obs_it != obs_dist.end()) {
        ++obs_it;
        ++prob_it;
        chosen_prob -= *prob_it;
    }
    out = *obs_it;
    return true;
}

float Domain::transition_function(const DynamicState& dynamic_state_ref, const StaticState& static_state_ref, const Action& action_ref,
    const DynamicState& next_state_ref) const {
    std::vector<DynamicState> next_state_dist;
    std::vector<float> next_state_probs;

    if (!next_state_distribution(dynamic_state_ref, static_state_ref, action_ref, next_state_dist, next_state_probs)) {
        return 0.0f;
    }

    std::vector<DynamicState>::const_iterator state_it = next_state_dist.cbegin();
    std::vector<float>::iterator prob_it = next_state_probs.begin();

    for (; state_it != next_state_dist.cend(); ++state_it, ++prob_it) {
        if (*state_it == next_state_ref) {
            return *prob_it;
        }
    }
    return 0.0f;
}

bool Domain::update_state(DynamicState& current_state, const StaticState& static_state, const Action& action_ref, std::default_random_engine& generator) {
    std::vector<DynamicState> next_state_dist;
    std::vector<float> next_state_probs;

    if (!next_state_distribution(current_state, static_state, action_ref, next_state_dist, next_state_probs)) {
        return false;
    }

    std::vector<DynamicState>::const_iterator state_it = next_state_dist.cbegin();
    std::vector<float>::iterator prob_it = next_state_probs.begin();
    float chosen_prob = std::uniform_real_distribution<>(0, 1)(generator) - *prob_it;
    while (chosen_prob > 0.0f && state_it != next_state_dist.cend()) {
        ++state_it;
        ++prob_it;
        chosen_prob -= *prob_it;
    }
    current_state = *state_it;
    return true;
}
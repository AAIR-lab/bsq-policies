#include "city_exploration_domain.h"
#include "../../utility.h"
#include<iostream>

void CityExplorationDomain::all_possible_actions(const DynamicState& dynamic_ref, const StaticState& static_ref, std::vector<Action>& out) const
{
    out.clear();

    out.resize(6);
    out[0].action_name = "Left";
    out[1].action_name = "Right";
    out[2].action_name = "Up";
    out[3].action_name = "Down";
    out[4].action_name = "Scan";
    out[5].action_name = "Visit";
}

bool CityExplorationDomain::can_create_observation(const Action& ref) const
{
    return ref.action_name == "Scan" || ref.action_name == "Visit";
}

Domain* CityExplorationDomain::create_copy() const
{
    return new CityExplorationDomain(*this);
}

bool CityExplorationDomain::next_state_distribution(const DynamicState& dynamic_ref, const StaticState& static_ref, const Action& action_ref,
    std::vector<DynamicState>& next_state_dist, std::vector<float>& next_state_prob) const
{
    int temp_x = 0, temp_y = 0, temp_idx = 0;
    std::vector<int> next_locs[2];
    std::vector<int>::iterator it, sit;
    
    if (dynamic_ref.terminated) {
        return false;
    }
    if (dynamic_ref.stuck) {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        return true;
    }
    if (action_ref.action_name == "Left") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        int x = dynamic_ref.integer_params[0];
        next_state_dist[0].integer_params[0] = static_ref.integer_params[static_ref.integer_params[2] + static_ref.integer_params[3] + 5 + next_state_dist[0].integer_params[0]];
        /*unsigned int temp_x = next_state_dist[0].integer_params[0] % static_ref.integer_params[0];
        unsigned int temp_y = next_state_dist[0].integer_params[0] / static_ref.integer_params[0];
        if (temp_x == 0) {
            next_state_dist[0].stuck = true;
            return true;
        }
        else {
            --next_state_dist[0].integer_params[0];
        }*/
        if (next_state_dist[0].integer_params[0] == -1) {
            next_state_dist[0].stuck = true;
            return true;
        }
    }
    else if (action_ref.action_name == "Right") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        next_state_dist[0].integer_params[0] = static_ref.integer_params[(static_ref.integer_params[0] * static_ref.integer_params[1]) + static_ref.integer_params[2] + static_ref.integer_params[3] + 5 + next_state_dist[0].integer_params[0]];
        if (next_state_dist[0].integer_params[0] == -1) {
            next_state_dist[0].stuck = true;
            return true;
        }
        /*unsigned int temp_x = next_state_dist[0].integer_params[0] % static_ref.integer_params[0];
        unsigned int temp_y = next_state_dist[0].integer_params[0] / static_ref.integer_params[0];
        if (temp_x == next_state_dist[0].integer_params[0] - 1) {
            next_state_dist[0].stuck = true;
            return true;
        }
        else {
            ++next_state_dist[0].integer_params[0];
        }*/
    }
    else if (action_ref.action_name == "Up") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        next_state_dist[0].integer_params[0] = static_ref.integer_params[(2 * static_ref.integer_params[0] * static_ref.integer_params[1]) + static_ref.integer_params[2] + static_ref.integer_params[3] + 5 + next_state_dist[0].integer_params[0]];
        if (next_state_dist[0].integer_params[0] == -1) {
            next_state_dist[0].stuck = true;
            return true;
        }
    }
    else if (action_ref.action_name == "Down") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        next_state_dist[0].integer_params[0] = static_ref.integer_params[(3 * static_ref.integer_params[0] * static_ref.integer_params[1]) + static_ref.integer_params[2] + static_ref.integer_params[3] + 5 + next_state_dist[0].integer_params[0]];
        if (next_state_dist[0].integer_params[0] == -1) {
            next_state_dist[0].stuck = true;
            return true;
        }
    }
    else if (action_ref.action_name == "Scan") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        return true;
    }
    else if (action_ref.action_name == "Visit") {
        next_state_dist.resize(1);
        next_state_prob.resize(1);
        next_state_prob[0] = 1.0f;
        next_state_dist[0] = dynamic_ref;
        for (int i = 5; i < 5 + static_ref.integer_params[2]; ++i) {
            if (static_ref.integer_params[i] == next_state_dist[0].integer_params[0]) {
                if (next_state_dist[0].bool_params[0]) {
                    //Visited a bank for a second time.
                    next_state_dist[0].stuck = true;
                }
                else {
                    //std::cout << "visited bank" << std::endl;
                    next_state_dist[0].bool_params[0] = true;
                }
                return true;
            }
        }
        for (int i = 5 + static_ref.integer_params[2]; i < 5 + static_ref.integer_params[2] + static_ref.integer_params[3]; ++i) {
            if (static_ref.integer_params[i] == next_state_dist[0].integer_params[0]) {
                if (next_state_dist[0].bool_params[0]) {
                    //Visited a bank before this store visit
                    //std::cout << "visited store" << std::endl;
                    next_state_dist[0].terminated = true;
                }
                else {
                    next_state_dist[0].stuck = true;
                }
                return true;
            }
        }
        //Visited not a bank or store.
        next_state_dist[0].stuck = true;
        return true;
    }
    else {
        next_state_dist.clear();
        next_state_prob.clear();
        return false;
    }

    if (!static_ref.bool_params[next_state_dist[0].integer_params[0]]) {
        next_state_dist[0].stuck = true;
    }

    for(int i = 0; i < 1; ++i){
        temp_x = dynamic_ref.integer_params[i] % static_ref.integer_params[0];
        temp_y = dynamic_ref.integer_params[i] / static_ref.integer_params[0];

        next_locs[i].push_back(dynamic_ref.integer_params[i]);
        if(temp_x > 0){
            temp_idx = (temp_x - 1) + (temp_y * static_ref.integer_params[0]);
            if(static_ref.bool_params[temp_idx]){
                next_locs[i].push_back(temp_idx);
            }
        }
        if(temp_x < (static_ref.integer_params[0] - 1)){
            temp_idx = (temp_x + 1) + (temp_y * static_ref.integer_params[0]);
            if(static_ref.bool_params[temp_idx]){
                next_locs[i].push_back(temp_idx);
            }
        }
        if(temp_y > 0){
            temp_idx = temp_x + ((temp_y - 1) * static_ref.integer_params[0]);
            if(static_ref.bool_params[temp_idx]){
                next_locs[i].push_back(temp_idx);
            }
        }    
        if(temp_y < 0){
            temp_idx = temp_x + ((temp_y - 1) * static_ref.integer_params[0]);
            if(static_ref.bool_params[temp_idx]){
                next_locs[i].push_back(temp_idx);
            }
        }
    }    

    for(it = next_locs[0].begin(); it != next_locs[0].end(); ++it){
        next_state_dist.push_back(next_state_dist[0]);
        next_state_dist.back().integer_params[1] = *it;
    }

    next_state_prob.assign(next_state_dist.size(),1.0f / (float)next_state_dist.size());


    return true;
}

bool CityExplorationDomain::observation_distribution(const DynamicState& dynamic_ref, const StaticState& static_ref, const Action& action_ref,
    std::vector<Observation>& observation_dist, std::vector<float>& observation_prob) const
{
    
    if (dynamic_ref.terminated || dynamic_ref.stuck) {
        observation_dist.resize(1);
        observation_prob.resize(1);
        observation_prob[0] = 1.0f;
        observation_dist[0].bool_params.clear();
        observation_dist[0].integer_params.clear();
    }
    else if (action_ref.action_name == "Scan") {
        observation_dist.resize(static_ref.integer_params[0] * static_ref.integer_params[1]);
        observation_prob.resize(static_ref.integer_params[0] * static_ref.integer_params[1]);
        int current_count = 0, grid_squares = static_ref.integer_params[0] * static_ref.integer_params[1];
        for (int i = 0; i < grid_squares; ++i) {
            if (static_ref.double_params[(dynamic_ref.integer_params[0] * grid_squares) + i] > 0.0) {
                observation_dist[current_count].bool_params.clear();
                observation_dist[current_count].integer_params.resize(1);
                observation_dist[current_count].integer_params[0] = i;
                observation_prob[current_count] = static_ref.double_params[(dynamic_ref.integer_params[0] * grid_squares) + i];
                ++current_count;
            }
        }
        observation_dist.resize(current_count);
        observation_dist.resize(current_count);
    }
    else if (action_ref.action_name == "Visit") {
        observation_dist.resize(1);
        observation_prob.resize(1);
        observation_prob[0] = 1.0f;
        observation_dist[0].bool_params.resize(1);
        observation_dist[0].bool_params[0] = false;
        for (int i = 5; i < 5 + static_ref.integer_params[2] + static_ref.integer_params[3]; ++i) {
            if (static_ref.integer_params[i] == dynamic_ref.integer_params[0]) {
                observation_dist[0].bool_params[0] = true;
                break;
            }
        }
    }
    else {
        observation_dist.resize(1);
        observation_prob.resize(1);
        observation_prob[0] = 1.0f;
        observation_dist[0].bool_params.clear();
    }

    return true;
}

float CityExplorationDomain::reward_function(const StaticState& static_ref, const DynamicState& dynamic_ref, const Action& action_ref,
    const DynamicState& next_state_ref) const {

    if (dynamic_ref.terminated) {
        return 0.0f;
    }

    return -1.0f;
}
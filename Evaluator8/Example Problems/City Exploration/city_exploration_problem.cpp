#include "city_exploration_problem.h"
#include<chrono>
#include<iostream>
#include<string>
#include <algorithm>

CityExploration::CityExploration()
{
    //constants = { "0","1","2","3","4","5","bank_1","bank_2","store_1" };
    std::pair<unsigned int, unsigned int> grid_dimensions = { 6, 5 };
    std::vector<unsigned int> bank_locations = { 5,18 }, store_locations = { 8 }, unsafe_locations = { 7,10,16,22 };
    std::map<std::string, unsigned int> x_dimension_constants, y_dimension_constants, bank_constants, store_constants;
    std::string temp_string;
    double observation_in_neighbor_prob = 0.15;

    for (unsigned int i = 0; i < grid_dimensions.first; ++i) {
        temp_string = std::to_string(i);
        constants.push_back(temp_string);
        x_dimension_constants[temp_string] = i;
    }
    for (unsigned int i = 0; i < grid_dimensions.second; ++i) {
        temp_string = std::to_string(i);
        if (i >= grid_dimensions.first) {
            constants.push_back(temp_string);
        }
        y_dimension_constants[temp_string] = i;
    }
    for (unsigned int i = 0; i < bank_locations.size(); ++i) {
        temp_string = "bank_" + std::to_string(i + 1u);
        constants.push_back(temp_string);
        bank_constants[temp_string] = i;
    }
    for (unsigned int i = 0; i < store_locations.size(); ++i) {
        temp_string = "store_" + std::to_string(i + 1u);
        constants.push_back(temp_string);
        store_constants[temp_string] = i;
    }

    actual_static_state.integer_params.resize(5u + bank_locations.size() + store_locations.size() + (grid_dimensions.first * grid_dimensions.second * 4u));
    //x dimension_size
    actual_static_state.integer_params[0] = grid_dimensions.first;
    function_information["x_dimension"].function_name = "x_dimension";
    function_information["x_dimension"].dynamic = false;
    function_information["x_dimension"].full_observable = true;
    function_information["x_dimension"].function_type = type_int;
    function_information["x_dimension"].index = 0u;
    function_information["x_dimension"].parameter_ranges.resize(1);
    function_information["x_dimension"].parameter_ranges[0].type = para_type_int;
    function_information["x_dimension"].parameter_ranges[0].output_value = true;

    //y dimension size
    actual_static_state.integer_params[1] = grid_dimensions.second;
    function_information["y_dimension"].function_name = "y_dimension";
    function_information["y_dimension"].dynamic = false;
    function_information["y_dimension"].full_observable = true;
    function_information["y_dimension"].function_type = type_int;
    function_information["y_dimension"].index = 1u;
    function_information["y_dimension"].parameter_ranges.resize(1);
    function_information["y_dimension"].parameter_ranges[0].type = para_type_int;
    function_information["y_dimension"].parameter_ranges[0].output_value = true;
    // bank number
    actual_static_state.integer_params[2] = bank_locations.size();
    function_information["bank_number"].function_name = "bank_number";
    function_information["bank_number"].dynamic = false;
    function_information["bank_number"].full_observable = true;
    function_information["bank_number"].function_type = type_int;
    function_information["bank_number"].index = 2u;
    function_information["bank_number"].parameter_ranges.resize(1);
    function_information["bank_number"].parameter_ranges[0].type = para_type_int;
    function_information["bank_number"].parameter_ranges[0].output_value = true;
    // store number
    actual_static_state.integer_params[3] = store_locations.size();
    function_information["store_number"].function_name = "store_number";
    function_information["store_number"].dynamic = false;
    function_information["store_number"].full_observable = true;
    function_information["store_number"].function_type = type_int;
    function_information["store_number"].index = 3u;
    function_information["store_number"].parameter_ranges.resize(1);
    function_information["store_number"].parameter_ranges[0].type = para_type_int;
    function_information["store_number"].parameter_ranges[0].output_value = true;
    // unsafe location number
    actual_static_state.integer_params[4] = unsafe_locations.size();
    function_information["unsafe_location_number"].function_name = "unsafe_location_number";
    function_information["unsafe_location_number"].dynamic = false;
    function_information["unsafe_location_number"].full_observable = true;
    function_information["unsafe_location_number"].function_type = type_int;
    function_information["unsafe_location_number"].index = 4u;
    function_information["unsafe_location_number"].parameter_ranges.resize(1);
    function_information["unsafe_location_number"].parameter_ranges[0].type = para_type_int;
    function_information["unsafe_location_number"].parameter_ranges[0].output_value = true;

    // bank locations
    int index_offset = 5;
    for (std::vector<unsigned int>::const_iterator it = bank_locations.cbegin(); it != bank_locations.cend(); ++it) {
        actual_static_state.integer_params[index_offset] = *it;
        ++index_offset;
    }
    function_information["bank_location"].function_name = "bank_location";
    function_information["bank_location"].dynamic = false;
    function_information["bank_location"].full_observable = true;
    function_information["bank_location"].function_type = type_int;
    function_information["bank_location"].index = 5u;
    function_information["bank_location"].parameter_ranges.resize(3);
    function_information["bank_location"].parameter_ranges[0].type = para_type_constant;
    function_information["bank_location"].parameter_ranges[0].output_value = false;
    function_information["bank_location"].parameter_ranges[0].allowed_constants = bank_constants;
    function_information["bank_location"].parameter_ranges[1].type = para_type_constant;
    function_information["bank_location"].parameter_ranges[1].output_value = true;
    function_information["bank_location"].parameter_ranges[1].allowed_constants = x_dimension_constants;
    function_information["bank_location"].parameter_ranges[2].type = para_type_constant;
    function_information["bank_location"].parameter_ranges[2].output_value = true;
    function_information["bank_location"].parameter_ranges[2].allowed_constants = y_dimension_constants;
    
    // store locations
    for (std::vector<unsigned int>::const_iterator it = store_locations.cbegin(); it != store_locations.cend(); ++it) {
        actual_static_state.integer_params[index_offset] = *it;
        ++index_offset;
    }
    function_information["store_location"].function_name = "store_location";
    function_information["store_location"].dynamic = false;
    function_information["store_location"].full_observable = true;
    function_information["store_location"].function_type = type_int;
    function_information["store_location"].index = 5u + bank_locations.size();
    function_information["store_location"].parameter_ranges.resize(3);
    function_information["store_location"].parameter_ranges[0].type = para_type_constant;
    function_information["store_location"].parameter_ranges[0].output_value = false;
    function_information["store_location"].parameter_ranges[0].allowed_constants = store_constants;
    function_information["store_location"].parameter_ranges[1].type = para_type_constant;
    function_information["store_location"].parameter_ranges[1].output_value = true;
    function_information["store_location"].parameter_ranges[1].allowed_constants = x_dimension_constants;
    function_information["store_location"].parameter_ranges[2].type = para_type_constant;
    function_information["store_location"].parameter_ranges[2].output_value = true;
    function_information["store_location"].parameter_ranges[2].allowed_constants = y_dimension_constants;
    // left
    int temp_x = 0, temp_y = 0;
    for (int i = 0; i < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++i) {
        temp_x = i % actual_static_state.integer_params[0];
        temp_y = i / actual_static_state.integer_params[0];
        temp_x -= 1;
        if (temp_x >= 0) {
            actual_static_state.integer_params[index_offset + i] = temp_x + (temp_y * actual_static_state.integer_params[0]);
        }
        else {
            actual_static_state.integer_params[index_offset + i] = -1;
        }
    }
    function_information["location_left"].function_name = "location_left";
    function_information["location_left"].dynamic = false;
    function_information["location_left"].full_observable = true;
    function_information["location_left"].function_type = type_int;
    function_information["location_left"].index = index_offset;
    function_information["location_left"].parameter_ranges.resize(4);
    function_information["location_left"].parameter_ranges[0].type = para_type_constant;
    function_information["location_left"].parameter_ranges[0].output_value = false;
    function_information["location_left"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["location_left"].parameter_ranges[1].type = para_type_constant;
    function_information["location_left"].parameter_ranges[1].output_value = false;
    function_information["location_left"].parameter_ranges[1].allowed_constants = y_dimension_constants;
    function_information["location_left"].parameter_ranges[2].type = para_type_constant;
    function_information["location_left"].parameter_ranges[2].output_value = true;
    function_information["location_left"].parameter_ranges[2].allowed_constants = x_dimension_constants;
    function_information["location_left"].parameter_ranges[3].type = para_type_constant;
    function_information["location_left"].parameter_ranges[3].output_value = true;
    function_information["location_left"].parameter_ranges[3].allowed_constants = y_dimension_constants;
    // right
    index_offset += actual_static_state.integer_params[0] * actual_static_state.integer_params[1];
    for (int i = 0; i < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++i) {
        temp_x = i % actual_static_state.integer_params[0];
        temp_y = i / actual_static_state.integer_params[0];
        temp_x += 1;
        if (temp_x < actual_static_state.integer_params[0]) {
            actual_static_state.integer_params[index_offset + i] = temp_x + (temp_y * actual_static_state.integer_params[0]);
        }
        else {
            actual_static_state.integer_params[index_offset + i] = -1;
        }
    }
    function_information["location_right"].function_name = "location_right";
    function_information["location_right"].dynamic = false;
    function_information["location_right"].full_observable = true;
    function_information["location_right"].function_type = type_int;
    function_information["location_right"].index = index_offset;
    function_information["location_right"].parameter_ranges.resize(4);
    function_information["location_right"].parameter_ranges[0].type = para_type_constant;
    function_information["location_right"].parameter_ranges[0].output_value = false;
    function_information["location_right"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["location_right"].parameter_ranges[1].type = para_type_constant;
    function_information["location_right"].parameter_ranges[1].output_value = false;
    function_information["location_right"].parameter_ranges[1].allowed_constants = y_dimension_constants;
    function_information["location_right"].parameter_ranges[2].type = para_type_constant;
    function_information["location_right"].parameter_ranges[2].output_value = true;
    function_information["location_right"].parameter_ranges[2].allowed_constants = x_dimension_constants;
    function_information["location_right"].parameter_ranges[3].type = para_type_constant;
    function_information["location_right"].parameter_ranges[3].output_value = true;
    function_information["location_right"].parameter_ranges[3].allowed_constants = y_dimension_constants;
    // up
    index_offset += actual_static_state.integer_params[0] * actual_static_state.integer_params[1];
    for (int i = 0; i < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++i) {
        temp_x = i % actual_static_state.integer_params[0];
        temp_y = i / actual_static_state.integer_params[0];
        temp_y += 1;
        if (temp_y < actual_static_state.integer_params[1]) {
            actual_static_state.integer_params[index_offset + i] = temp_x + (temp_y * actual_static_state.integer_params[0]);
        }
        else {
            actual_static_state.integer_params[index_offset + i] = -1;
        }
    }
    function_information["location_up"].function_name = "location_up";
    function_information["location_up"].dynamic = false;
    function_information["location_up"].full_observable = true;
    function_information["location_up"].function_type = type_int;
    function_information["location_up"].index = index_offset;
    function_information["location_up"].parameter_ranges.resize(4);
    function_information["location_up"].parameter_ranges[0].type = para_type_constant;
    function_information["location_up"].parameter_ranges[0].output_value = false;
    function_information["location_up"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["location_up"].parameter_ranges[1].type = para_type_constant;
    function_information["location_up"].parameter_ranges[1].output_value = false;
    function_information["location_up"].parameter_ranges[1].allowed_constants = y_dimension_constants;
    function_information["location_up"].parameter_ranges[2].type = para_type_constant;
    function_information["location_up"].parameter_ranges[2].output_value = true;
    function_information["location_up"].parameter_ranges[2].allowed_constants = x_dimension_constants;
    function_information["location_up"].parameter_ranges[3].type = para_type_constant;
    function_information["location_up"].parameter_ranges[3].output_value = true;
    function_information["location_up"].parameter_ranges[3].allowed_constants = y_dimension_constants;

    // down
    index_offset += actual_static_state.integer_params[0] * actual_static_state.integer_params[1];
    for (int i = 0; i < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++i) {
        temp_x = i % actual_static_state.integer_params[0];
        temp_y = i / actual_static_state.integer_params[0];
        temp_y -= 1;
        if (temp_y >= 0) {
            actual_static_state.integer_params[index_offset + i] = temp_x + (temp_y * actual_static_state.integer_params[0]);
        }
        else {
            actual_static_state.integer_params[index_offset + i] = -1;
        }
    }
    function_information["location_down"].function_name = "location_down";
    function_information["location_down"].dynamic = false;
    function_information["location_down"].full_observable = true;
    function_information["location_down"].function_type = type_int;
    function_information["location_down"].index = index_offset;
    function_information["location_down"].parameter_ranges.resize(4);
    function_information["location_down"].parameter_ranges[0].type = para_type_constant;
    function_information["location_down"].parameter_ranges[0].output_value = false;
    function_information["location_down"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["location_down"].parameter_ranges[1].type = para_type_constant;
    function_information["location_down"].parameter_ranges[1].output_value = false;
    function_information["location_down"].parameter_ranges[1].allowed_constants = y_dimension_constants;
    function_information["location_down"].parameter_ranges[2].type = para_type_constant;
    function_information["location_down"].parameter_ranges[2].output_value = true;
    function_information["location_down"].parameter_ranges[2].allowed_constants = x_dimension_constants;
    function_information["location_down"].parameter_ranges[3].type = para_type_constant;
    function_information["location_down"].parameter_ranges[3].output_value = true;
    function_information["location_down"].parameter_ranges[3].allowed_constants = y_dimension_constants;

    // Safe grid locations
    actual_static_state.bool_params.resize(actual_static_state.integer_params[0] * actual_static_state.integer_params[1], true);
    for (std::vector<unsigned int>::const_iterator it = unsafe_locations.cbegin(); it != unsafe_locations.cend(); ++it) {
        actual_static_state.bool_params[*it] = false;
    }
    function_information["is_safe"].function_name = "is_safe";
    function_information["is_safe"].dynamic = false;
    function_information["is_safe"].full_observable = true;
    function_information["is_safe"].function_type = type_bool;
    function_information["is_safe"].index = 0u;
    function_information["is_safe"].parameter_ranges.resize(2);
    function_information["is_safe"].parameter_ranges[0].type = para_type_constant;
    function_information["is_safe"].parameter_ranges[0].output_value = false;
    function_information["is_safe"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["is_safe"].parameter_ranges[1].type = para_type_constant;
    function_information["is_safe"].parameter_ranges[1].output_value = false;
    function_information["is_safe"].parameter_ranges[1].allowed_constants = y_dimension_constants;

    // Agent's location function
    function_information["current_location"].function_name = "current_location";
    function_information["current_location"].dynamic = true;
    function_information["current_location"].full_observable = false;
    function_information["current_location"].function_type = type_int;
    function_information["current_location"].index = 0u;
    function_information["current_location"].parameter_ranges.resize(2);
    function_information["current_location"].parameter_ranges[0].type = para_type_constant;
    function_information["current_location"].parameter_ranges[0].output_value = true;
    function_information["current_location"].parameter_ranges[0].allowed_constants = x_dimension_constants;
    function_information["current_location"].parameter_ranges[1].type = para_type_constant;
    function_information["current_location"].parameter_ranges[1].output_value = true;
    function_information["current_location"].parameter_ranges[1].allowed_constants = y_dimension_constants;

    // Whether the agent has visited a bank or not function
    function_information["visited_bank"].function_name = "visited_bank";
    function_information["visited_bank"].dynamic = true;
    function_information["visited_bank"].full_observable = true;
    function_information["visited_bank"].function_type = type_bool;
    function_information["visited_bank"].index = 0u;

    // Build probability table of observations.
    actual_static_state.double_params.resize(actual_static_state.integer_params[0] * actual_static_state.integer_params[1] * actual_static_state.integer_params[0] * actual_static_state.integer_params[1], 0.0);
    //std::list<int> safe_neighbors;
    int stemp_x = 0;
    int stemp_y = 0;

    double individual_probs = 0.0;
    double temp_weighted_sum = 0.0;
    int temp_distance = 0;
    int temp_calc = 0;
    std::vector<unsigned int>::iterator unsafe_it;

    for (int i = 0; i < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++i) {
        temp_x = i % actual_static_state.integer_params[0];
        temp_y = i / actual_static_state.integer_params[0];
        temp_calc = i * (actual_static_state.integer_params[0] * actual_static_state.integer_params[1]);
        actual_static_state.double_params[temp_calc + i] = 0.5;
        temp_weighted_sum = 0.0;
        for (int j = 0; j < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++j) {
            if(i == j){
                continue;
            }
            unsafe_it = std::find(unsafe_locations.begin(),unsafe_locations.end(),j);
            if(unsafe_it != unsafe_locations.end()){
                continue;
            }
            stemp_x = j % actual_static_state.integer_params[0];
            stemp_y = j / actual_static_state.integer_params[0];

            temp_distance = abs(temp_x - stemp_x) + abs(temp_y - stemp_y);
            if(temp_distance > 2){
                continue;
            }
            actual_static_state.double_params[temp_calc + j] = pow(0.5,(double)temp_distance);
            temp_weighted_sum += actual_static_state.double_params[temp_calc + j];
        }
        individual_probs = 0.5 / temp_weighted_sum;
        for (int j = 0; j < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++j) {
            if(i == j){
                continue;
            }
            actual_static_state.double_params[temp_calc + j] *= individual_probs;
            //std::cout << actual_static_state.double_params[temp_calc + j] << std::endl;
        }
        // if (temp_x > 0) {
        //     if (actual_static_state.bool_params[i - 1]) {
        //         safe_neighbors.push_back(i - 1);
        //     }
        // }
        // if (temp_x < actual_static_state.integer_params[0] - 1) {
        //     if (actual_static_state.bool_params[i + 1]) {
        //         safe_neighbors.push_back(i + 1);
        //     }
        // }
        // if (temp_y > 0) {
        //     if (actual_static_state.bool_params[i - actual_static_state.integer_params[0]]) {
        //         safe_neighbors.push_back(i - actual_static_state.integer_params[0]);
        //     }
        // }
        // if (temp_y < actual_static_state.integer_params[1] - 1) {
        //     if (actual_static_state.bool_params[i + actual_static_state.integer_params[0]]) {
        //         safe_neighbors.push_back(i + actual_static_state.integer_params[0]);
        //     }
        // }

        // temp_calc = i * (actual_static_state.integer_params[0] * actual_static_state.integer_params[1]);
        // actual_static_state.double_params[temp_calc + i] = 1.0 - (observation_in_neighbor_prob * safe_neighbors.size());

        // for (std::list<int>::iterator it = safe_neighbors.begin(); it != safe_neighbors.end(); ++it) {
        //     actual_static_state.double_params[temp_calc + *it] = observation_in_neighbor_prob;
        // }
        // safe_neighbors.clear();
    }

    // Initialize possible actions.
    action_information["Left"].action_name = "Left";
    action_information["Right"].action_name = "Right";
    action_information["Up"].action_name = "Up";
    action_information["Down"].action_name = "Down";
    action_information["Scan"].action_name = "Scan";
    action_information["Visit"].action_name = "Visit";
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());

    data_function_information["bank_1_x"].dynamic = false;
    data_function_information["bank_1_x"].full_observable = true;
    data_function_information["bank_1_x"].function_name = "bank_x";
    data_function_information["bank_1_x"].function_type = type_int;
    data_function_information["bank_1_x"].index = 5u;
}
CityExploration::~CityExploration()
{

}
Problem* CityExploration::create_copy() const
{
    return new CityExploration(*this);
}
void CityExploration::generate_initial_belief(StaticState& static_state_out, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const
{
    int state_count = (actual_static_state.integer_params[0] * actual_static_state.integer_params[1]) - actual_static_state.integer_params[4];
    static_state_out = actual_static_state;
    float state_number = state_count * state_count;
    init_states.resize(state_number);
    init_state_probs.resize(state_number);
    std::list<DynamicState>::iterator it = init_states.begin();
    std::list<float>::iterator float_it = init_state_probs.begin();
    int temp = 0;
    for (int current = 0; current < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++current) {
        if (actual_static_state.bool_params[current]) {
            for (int current_p1 = 0; current_p1 < actual_static_state.integer_params[0] * actual_static_state.integer_params[1]; ++current_p1) {
                if (actual_static_state.bool_params[current_p1]) {
                    it->rewards.clear();
                    it->terminated = false;
                    it->stuck = false;
                    it->integer_params.resize(2);
                    it->bool_params.resize(1);
                    //set starting location and that is hasn't visited a bank
                    it->integer_params[0] = current;
                    it->integer_params[1] = current_p1;
                    it->bool_params[0] = false;
                    *float_it = 1.0f / state_number;
                    ++it;
                    ++float_it;
                }
            }
        }
    }

}
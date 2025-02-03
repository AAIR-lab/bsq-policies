#include "graph_rock_sample_problem.h"
#include <iostream>

GraphRockSample::GraphRockSample()
{
	std::vector<std::string> rock_names = { "rock_1", "rock_2", "rock_3", "rock_4" };
	std::vector<std::string> other_locations = { "marker_1", "marker_2" };
	std::map<std::string, unsigned int> rock_types = { {"type_1",0u},{"type_2",1u}};
	std::vector<std::pair<std::string, std::string>> connected_locations;
	connected_locations.emplace_back("starting_location", "marker_1");
	connected_locations.emplace_back("starting_location", "marker_2");
	connected_locations.emplace_back("starting_location", "rock_3");
	connected_locations.emplace_back("rock_1", "rock_2");
	connected_locations.emplace_back("rock_1", "marker_1");
	connected_locations.emplace_back("rock_2", "rock_1");
	connected_locations.emplace_back("rock_2", "marker_1");
	connected_locations.emplace_back("rock_2", "dropoff_location");
	connected_locations.emplace_back("rock_3", "starting_location");
	connected_locations.emplace_back("rock_3", "marker_1");
	connected_locations.emplace_back("rock_3", "marker_2");
	connected_locations.emplace_back("rock_4", "marker_2");
	connected_locations.emplace_back("marker_1", "starting_location");
	connected_locations.emplace_back("marker_1", "rock_1");
	connected_locations.emplace_back("marker_1", "rock_2");
	connected_locations.emplace_back("marker_1", "rock_3");
	connected_locations.emplace_back("marker_1", "marker_2");
	connected_locations.emplace_back("marker_2", "starting_location");
	connected_locations.emplace_back("marker_2", "rock_3");
	connected_locations.emplace_back("marker_2", "rock_4");
	connected_locations.emplace_back("marker_2", "marker_1");
	connected_locations.emplace_back("marker_2", "dropoff_location");

	std::map<std::string, std::map<std::string, double>> distances;
	distances["starting_location"]["rock_1"] = 3.0;
	distances["starting_location"]["rock_2"] = 5.0;
	distances["starting_location"]["rock_3"] = 3.0;
	distances["starting_location"]["rock_4"] = 5.0;

	distances["rock_1"]["rock_1"] = 0.0;
	distances["rock_1"]["rock_2"] = 1.0;
	distances["rock_1"]["rock_3"] = 5.0;
	distances["rock_1"]["rock_4"] = 6.0;

	distances["rock_2"]["rock_1"] = 1.0;
	distances["rock_2"]["rock_2"] = 0.0;
	distances["rock_2"]["rock_3"] = 5.0;
	distances["rock_2"]["rock_4"] = 4.0;

	distances["rock_3"]["rock_1"] = 5.0;
	distances["rock_3"]["rock_2"] = 5.0;
	distances["rock_3"]["rock_3"] = 0.0;
	distances["rock_3"]["rock_4"] = 2.0;

	distances["rock_4"]["rock_1"] = 6.0;
	distances["rock_4"]["rock_2"] = 4.0;
	distances["rock_4"]["rock_3"] = 2.0;
	distances["rock_4"]["rock_4"] = 0.0;

	distances["marker_1"]["rock_1"] = 1.0;
	distances["marker_1"]["rock_2"] = 1.0;
	distances["marker_1"]["rock_3"] = 4.0;
	distances["marker_1"]["rock_4"] = 5.0;

	distances["marker_2"]["rock_1"] = 4.0;
	distances["marker_2"]["rock_2"] = 2.0;
	distances["marker_2"]["rock_3"] = 3.0;
	distances["marker_2"]["rock_4"] = 1.0;

	constants = { "rock_1", "rock_2", "rock_3", "rock_4", "starting_location", "dropoff_location", "marker_1", "marker_2", "type_1", "type_2"};
	std::map<std::string, std::string> rock_data;
	rock_data["rock_1"] = "type_1";
	rock_data["rock_2"] = "type_2";
	rock_data["rock_3"] = "type_1";
	rock_data["rock_4"] = "type_2";
	std::map<std::string, unsigned int> temp_mapping;
	temp_mapping["starting_location"] = 0u;
	for (int i = 0; i < rock_names.size(); ++i) {
		temp_mapping[rock_names[i]] = i + 1u;
	}
	for (int i = 0; i < other_locations.size(); ++i) {
		temp_mapping[other_locations[i]] = i + 1u + rock_names.size();
	}
	temp_mapping["dropoff_location"] = rock_names.size() + other_locations.size() + 1;
	actual_static_state.integer_params.resize(rock_names.size() + 3);
	actual_static_state.integer_params[0] = rock_names.size();
	actual_static_state.integer_params[1] = actual_static_state.integer_params[0] + other_locations.size() + 2;
	actual_static_state.integer_params[2] = rock_types.size();
	for (int i = 0; i < rock_data.size(); ++i) {
		actual_static_state.integer_params[i + 3] = rock_types[rock_data[rock_names[i]]];
	}
	actual_static_state.bool_params.resize(actual_static_state.integer_params[1] * actual_static_state.integer_params[1],false);
	for (std::vector<std::pair<std::string, std::string>>::iterator it = connected_locations.begin(); it != connected_locations.end(); ++it) {
		actual_static_state.bool_params[temp_mapping[it->first] + (actual_static_state.integer_params[1] * temp_mapping[it->second])] = true;
	}
	actual_static_state.double_params.resize((actual_static_state.integer_params[1] - 1) * rock_names.size() * 2,0.0);
	unsigned int temp_val = 0u;
	for (std::map<std::string, std::map<std::string, double>>::iterator it = distances.begin(); it != distances.end(); ++it) {
		for (std::map<std::string, double>::iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
			temp_val = temp_mapping[it->first] + ((actual_static_state.integer_params[1] - 1) * (temp_mapping[sit->first] - 1));
			actual_static_state.double_params[temp_val] = sit->second;
			temp_val += (actual_static_state.integer_params[1] - 1) * rock_names.size();
			actual_static_state.double_params[temp_val] = pow(2.0, (sit->second) / -5) + 1.0;
			actual_static_state.double_params[temp_val] *= 0.5;
			actual_static_state.double_params[temp_val] *= 0.9; //maximum sensor accuracy (when it's in the same location as the rock)
		}
	}

	action_information["goto"].action_name = "goto";
	action_information["goto"].action_parameters.resize(1);
	action_information["goto"].action_parameters[0].allowed_constants = temp_mapping;
	action_information["goto"].action_parameters[0].output_value = false;
	action_information["goto"].action_parameters[0].type = para_type_constant;

	function_information["location"].dynamic = true;
	function_information["location"].full_observable = true;
	function_information["location"].function_name = "location";
	function_information["location"].function_type = type_int;
	function_information["location"].index = 0;
	function_information["location"].parameter_ranges.resize(1);
	function_information["location"].parameter_ranges[0].allowed_constants = temp_mapping;
	function_information["location"].parameter_ranges[0].output_value = true;
	function_information["location"].parameter_ranges[0].type = para_type_constant;

	data_function_information["distance"].dynamic = false;
	data_function_information["distance"].full_observable = true;
	data_function_information["distance"].function_name = "distance";
	data_function_information["distance"].function_type = type_double;
	data_function_information["distance"].index = 0;
	data_function_information["distance"].parameter_ranges.resize(2);
	data_function_information["distance"].parameter_ranges[0].type = para_type_constant;
	data_function_information["distance"].parameter_ranges[0].output_value = false;
	temp_mapping.erase("dropoff_location");
	data_function_information["distance"].parameter_ranges[0].allowed_constants = temp_mapping;
	temp_mapping.erase("starting_location");
	for (std::string& current : other_locations) {
		temp_mapping.erase(current);
	}

	function_information["worth_sampling"].dynamic = true;
	function_information["worth_sampling"].full_observable = false;
	function_information["worth_sampling"].function_name = "worth_sampling";
	function_information["worth_sampling"].function_type = type_bool;
	function_information["worth_sampling"].index = actual_static_state.integer_params[2];
	function_information["worth_sampling"].parameter_ranges.resize(1);

	action_information["scan"].action_name = "scan";
	action_information["scan"].action_parameters.resize(1);
	action_information["scan"].action_parameters[0].allowed_constants = temp_mapping;
	action_information["scan"].action_parameters[0].output_value = false;
	action_information["scan"].action_parameters[0].type = para_type_constant;

	for (std::map<std::string, unsigned int>::iterator map_it = temp_mapping.begin(); map_it != temp_mapping.end(); ++map_it) {
		--map_it->second;
	}

	data_function_information["distance"].parameter_ranges[1].type = para_type_constant;
	data_function_information["distance"].parameter_ranges[1].output_value = false;
	data_function_information["distance"].parameter_ranges[1].allowed_constants = temp_mapping;

	function_information["worth_sampling"].parameter_ranges[0].allowed_constants = temp_mapping;
	function_information["worth_sampling"].parameter_ranges[0].output_value = false;
	function_information["worth_sampling"].parameter_ranges[0].type = para_type_constant;

	function_information["have_sample"].dynamic = true;
	function_information["have_sample"].full_observable = true;
	function_information["have_sample"].function_name = "have_sample";
	function_information["have_sample"].function_type = type_bool;
	function_information["have_sample"].index = 0;
	function_information["have_sample"].parameter_ranges.resize(1);
	function_information["have_sample"].parameter_ranges[0].allowed_constants = rock_types;
	function_information["have_sample"].parameter_ranges[0].output_value = false;
	function_information["have_sample"].parameter_ranges[0].type = para_type_constant;

	action_information["sample"].action_name = "sample";
}

GraphRockSample::~GraphRockSample()
{

}

Problem* GraphRockSample::create_copy() const
{
	return new GraphRockSample(*this);
}

void GraphRockSample::generate_initial_belief(StaticState &static_state_out, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const
{
	int state_count = (int)pow(2.0, actual_static_state.integer_params[0]);
	static_state_out = actual_static_state;
	init_states.resize(state_count);
	init_state_probs.resize(state_count);

	std::list<DynamicState>::iterator it = init_states.begin();
	std::list<float>::iterator float_it = init_state_probs.begin();
	int temp = 0;
	for (int current = 0; current < state_count; ++current) {
		it->rewards.clear();
		it->terminated = false;
		it->stuck = false;
		it->integer_params.resize(1);
		it->bool_params.resize(actual_static_state.integer_params[2] + actual_static_state.integer_params[0]);
		it->integer_params[0] = 0;
		std::fill(it->bool_params.begin(), it->bool_params.end(), false);
		temp = current;
		for (int i = actual_static_state.integer_params[2]; i < actual_static_state.integer_params[0] + actual_static_state.integer_params[2]; ++i) {
			it->bool_params[i] = temp % 2 == 1;
			temp = temp / 2;
		}
		*float_it = 1.0f / state_count;
		++it;
		++float_it;
	}
}
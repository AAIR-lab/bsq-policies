#include "spaceship_repair_problem.h"

SpaceshipRepair::SpaceshipRepair()
{
	//5,5
	int distance_from_robot_repair = 7, distance_from_ship_repair = 5;
	double robot_issue_sensor_acc = 0.75, ship_issue_sensor_acc = 0.55;

	actual_static_state.integer_params.resize(2);
	actual_static_state.integer_params[0] = -distance_from_robot_repair;
	actual_static_state.integer_params[1] = distance_from_ship_repair;
	actual_static_state.double_params.resize(4);

	actual_static_state.double_params[0] = (1.0 - robot_issue_sensor_acc) * (1.0 - ship_issue_sensor_acc);
	actual_static_state.double_params[1] = (1.0 - robot_issue_sensor_acc) * ship_issue_sensor_acc;
	actual_static_state.double_params[2] = robot_issue_sensor_acc * (1.0 - ship_issue_sensor_acc);
	actual_static_state.double_params[3] = robot_issue_sensor_acc * ship_issue_sensor_acc;

	function_information["repair_needed"].function_name = "repair_needed";
	function_information["repair_needed"].dynamic = true;
	function_information["repair_needed"].full_observable = false;
	function_information["repair_needed"].function_type = type_bool;
	function_information["repair_needed"].index = 0u;
	function_information["repair_needed"].parameter_ranges.resize(1);
	function_information["repair_needed"].parameter_ranges[0].output_value = false;
	function_information["repair_needed"].parameter_ranges[0].type = para_type_constant;
	function_information["repair_needed"].parameter_ranges[0].allowed_constants = { {"robot",0u},{"spaceship",1u} };

	action_information["wait"].action_name = "wait";
	action_information["fix"].action_name = "fix";
	action_information["fix"].action_parameters.resize(1);
	action_information["fix"].action_parameters[0].type = para_type_constant;
	action_information["fix"].action_parameters[0].allowed_constants = { {"robot",0},{"spaceship",1} };
}

SpaceshipRepair::SpaceshipRepair(unsigned int rob_station_distance, unsigned int ship_station_distance, double rob_acc, double ship_acc)
{
	int distance_from_robot_repair = rob_station_distance, distance_from_ship_repair = ship_station_distance;
	double robot_issue_sensor_acc = rob_acc, ship_issue_sensor_acc = ship_acc;
	
	actual_static_state.integer_params.resize(2);
	actual_static_state.integer_params[0] = -distance_from_robot_repair;
	actual_static_state.integer_params[1] = distance_from_ship_repair;
	actual_static_state.double_params.resize(4);

	actual_static_state.double_params[0] = (1.0 - robot_issue_sensor_acc) * (1.0 - ship_issue_sensor_acc);
	actual_static_state.double_params[1] = (1.0 - robot_issue_sensor_acc) * ship_issue_sensor_acc;
	actual_static_state.double_params[2] = robot_issue_sensor_acc * (1.0 - ship_issue_sensor_acc);
	actual_static_state.double_params[3] = robot_issue_sensor_acc * ship_issue_sensor_acc;

	function_information["repair_needed"].function_name = "repair_needed";
	function_information["repair_needed"].dynamic = true;
	function_information["repair_needed"].full_observable = false;
	function_information["repair_needed"].function_type = type_bool;
	function_information["repair_needed"].index = 0u;
	function_information["repair_needed"].parameter_ranges.resize(1);
	function_information["repair_needed"].parameter_ranges[0].output_value = false;
	function_information["repair_needed"].parameter_ranges[0].type = para_type_constant;
	function_information["repair_needed"].parameter_ranges[0].allowed_constants = { {"robot",0u},{"spaceship",1u} };

	action_information["wait"].action_name = "wait";
	action_information["fix"].action_name = "fix";
	action_information["fix"].action_parameters.resize(1);
	action_information["fix"].action_parameters[0].type = para_type_constant;
	action_information["fix"].action_parameters[0].allowed_constants = { {"robot",0},{"spaceship",1} };
}

SpaceshipRepair::~SpaceshipRepair()
{

}

Problem* SpaceshipRepair::create_copy() const
{
	return new SpaceshipRepair(*this);
}
void SpaceshipRepair::generate_initial_belief(StaticState& static_state_out, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const
{
	static_state_out = actual_static_state;
	init_states.resize(4);
	init_state_probs.resize(4);

	std::list<DynamicState>::iterator state_it = init_states.begin();
	std::list<float>::iterator prob_it = init_state_probs.begin();

	state_it->bool_params = { false, false };
	state_it->integer_params = { 0 };
	state_it->rewards.clear();
	state_it->stuck = false;
	state_it->terminated = false;
	*prob_it = 0.25f;

	++state_it;
	++prob_it;

	state_it->bool_params = { false, true };
	state_it->integer_params = { 0 };
	state_it->rewards.clear();
	state_it->stuck = false;
	state_it->terminated = false;
	*prob_it = 0.25f;

	++state_it;
	++prob_it;

	state_it->bool_params = { true, false };
	state_it->integer_params = { 0 };
	state_it->rewards.clear();
	state_it->stuck = false;
	state_it->terminated = false;
	*prob_it = 0.25f;

	++state_it;
	++prob_it;

	state_it->bool_params = { true, true };
	state_it->integer_params = { 0 };
	state_it->rewards.clear();
	state_it->stuck = false;
	state_it->terminated = false;
	*prob_it = 0.25f;
}
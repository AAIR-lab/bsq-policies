#include "lane_merger_problem.h"

LaneMerger::LaneMerger()
{
	unsigned int merger_length = 40;
	unsigned int agent_start_location = 2;
	std::pair<unsigned int, unsigned int> other_car_start_location = {0, 4};
	unsigned int agent_start_speed = 3;
	unsigned int speed_limit = 3;
	std::pair<unsigned int, unsigned int> other_car_speed_range = { 0,4 };
	float side_by_side_true_positive = 0.7f;
	float side_by_side_true_negative = 0.65f;
	float diagonal_accuracy_true_positive = 0.6f;
	float diagonal_accuracy_true_negative = 0.6f;
	float front_true_positive = 0.95f;
	float front_distance_tp_retention = 0.8f;
	float front_true_negative = 0.65f;
	float back_true_positive = 0.9f;
	float back_distance_tp_retention = 0.8f;
	float back_true_negative = 0.65f;

	actual_static_state.integer_params.resize(8);
	actual_static_state.integer_params[0] = merger_length;
	actual_static_state.integer_params[1] = agent_start_location;
	actual_static_state.integer_params[2] = other_car_start_location.first;
	actual_static_state.integer_params[3] = agent_start_speed;
	actual_static_state.integer_params[4] = speed_limit;
	actual_static_state.integer_params[5] = other_car_speed_range.first;
	actual_static_state.integer_params[6] = other_car_speed_range.second;
	actual_static_state.integer_params[7] = other_car_start_location.second;

	actual_static_state.double_params.resize(10 + (32 * (actual_static_state.integer_params[0] + 1) * (actual_static_state.integer_params[0] + 1)));
	actual_static_state.double_params[0] = side_by_side_true_positive;
	actual_static_state.double_params[1] = side_by_side_true_negative;
	actual_static_state.double_params[2] = diagonal_accuracy_true_positive;
	actual_static_state.double_params[3] = diagonal_accuracy_true_negative;
	actual_static_state.double_params[4] = front_true_positive;
	actual_static_state.double_params[5] = front_distance_tp_retention;
	actual_static_state.double_params[6] = front_true_negative;
	actual_static_state.double_params[7] = back_true_positive;
	actual_static_state.double_params[8] = back_distance_tp_retention;
	actual_static_state.double_params[9] = back_true_negative;
	fill_observation_table();

	function_information["current_location"].dynamic = true;
	function_information["current_location"].full_observable = true;
	function_information["current_location"].function_name = "current_location";
	function_information["current_location"].function_type = type_int;
	function_information["current_location"].index = 0;
	function_information["current_location"].parameter_ranges.resize(1);
	function_information["current_location"].parameter_ranges[0].output_value = true;
	function_information["current_location"].parameter_ranges[0].type = para_type_int;

	function_information["current_speed"].dynamic = true;
	function_information["current_speed"].full_observable = true;
	function_information["current_speed"].function_name = "current_speed";
	function_information["current_speed"].function_type = type_int;
	function_information["current_speed"].index = 1;
	function_information["current_speed"].parameter_ranges.resize(1);
	function_information["current_speed"].parameter_ranges[0].output_value = true;
	function_information["current_speed"].parameter_ranges[0].type = para_type_int;

	function_information["other_car_location"].dynamic = true;
	function_information["other_car_location"].full_observable = false;
	function_information["other_car_location"].function_name = "other_car_location";
	function_information["other_car_location"].function_type = type_int;
	function_information["other_car_location"].index = 2;
	function_information["other_car_location"].parameter_ranges.resize(1);
	function_information["other_car_location"].parameter_ranges[0].output_value = true;
	function_information["other_car_location"].parameter_ranges[0].type = para_type_int;

	function_information["other_car_speed"].dynamic = true;
	function_information["other_car_speed"].full_observable = false;
	function_information["other_car_speed"].function_name = "other_car_speed";
	function_information["other_car_speed"].function_type = type_int;
	function_information["other_car_speed"].index = 3;
	function_information["other_car_speed"].parameter_ranges.resize(1);
	function_information["other_car_speed"].parameter_ranges[0].output_value = true;
	function_information["other_car_speed"].parameter_ranges[0].type = para_type_int;

	function_information["safe_to_merge"].dynamic = true;
	function_information["safe_to_merge"].full_observable = false;
	function_information["safe_to_merge"].function_name = "safe_to_merge";
	function_information["safe_to_merge"].index = 0;
	function_information["safe_to_merge"].function_type = type_bool;

	function_information["next_to"].dynamic = true;
	function_information["next_to"].full_observable = false;
	function_information["next_to"].function_name = "next_to";
	function_information["next_to"].index = 1;
	function_information["next_to"].function_type = type_bool;	

	action_information["merge"].action_name = "merge";
	action_information["increase_speed"].action_name = "increase_speed";
	action_information["decrease_speed"].action_name = "decrease_speed";
	action_information["keep_speed"].action_name = "keep_speed";


}

LaneMerger::~LaneMerger()
{

}

Problem* LaneMerger::create_copy() const
{
	return new LaneMerger(*this);
}

void LaneMerger::generate_initial_belief(StaticState & static_state_out, std::list<DynamicState>& init_states, std::list<float>& init_state_probs) const
{
	int state_count = (1 + actual_static_state.integer_params[6] - actual_static_state.integer_params[5]) * (1 + actual_static_state.integer_params[7] - actual_static_state.integer_params[2]) ;
	static_state_out = actual_static_state;
	init_states.resize(state_count);
	init_state_probs.resize(state_count);

	std::list<DynamicState>::iterator it = init_states.begin();
	std::list<float>::iterator float_it = init_state_probs.begin();
	int temp = 0;
	for (int other_car = actual_static_state.integer_params[2]; other_car <= actual_static_state.integer_params[7]; ++other_car){
		for (int current = actual_static_state.integer_params[5]; current <= actual_static_state.integer_params[6]; ++current) {
			it->rewards.clear();
			it->terminated = false;
			it->stuck = false;
			it->integer_params.resize(4);
			it->integer_params[0] = actual_static_state.integer_params[1];
			it->integer_params[1] = actual_static_state.integer_params[3];
			it->integer_params[2] = other_car;
			it->integer_params[3] = current;

			it->bool_params.resize(2);
			it->bool_params[0] = (it->integer_params[2] - it->integer_params[0] - it->integer_params[1] > 2) || (it->integer_params[0] - it->integer_params[2] - it->integer_params[3] > 2);
			it->bool_params[1] = abs(it->integer_params[0] - it->integer_params[2]) <= 1;
			*float_it = 1.0f / state_count;
			++it;
			++float_it;
		}
	}
}

void inline LaneMerger::fill_observation_table() {
	float front_positive_prob = 0.0f, front_diag_positive_prob = 0.0f, side_positive_prob = 0.0f, back_diag_positive_prob = 0.0f, back_positive_prob = 0.0f;
	int distance_dif = 0, temp = 0, current_index = 0;
	for (int current_agent_loc = 0; current_agent_loc <= actual_static_state.integer_params[0]; ++current_agent_loc) {
		for (int current_other_loc = 0; current_other_loc <= actual_static_state.integer_params[0]; ++current_other_loc) {
			current_index = 32 * ((current_agent_loc * (actual_static_state.integer_params[0] + 1)) + current_other_loc);
			current_index += 10;
			distance_dif = current_other_loc - current_agent_loc;
			if (distance_dif > 1) {
				front_positive_prob = std::max(actual_static_state.double_params[4] * pow(actual_static_state.double_params[5], distance_dif - 2), 0.5);
			}
			else {
				front_positive_prob = 1.0 - actual_static_state.double_params[6];
			}

			if (distance_dif == 1) {
				front_diag_positive_prob = actual_static_state.double_params[2];
			}
			else {
				front_diag_positive_prob = 1.0 - actual_static_state.double_params[3];
			}

			if (distance_dif == 0) {
				side_positive_prob = actual_static_state.double_params[0];
			}
			else {
				side_positive_prob = 1.0 - actual_static_state.double_params[1];
			}

			if (distance_dif == -1) {
				back_diag_positive_prob = actual_static_state.double_params[2];
			}
			else {
				back_diag_positive_prob = 1.0 - actual_static_state.double_params[3];
			}

			if (distance_dif < -1) {
				back_positive_prob = std::max(actual_static_state.double_params[7] * pow(actual_static_state.double_params[8], std::abs(distance_dif) - 2), 0.5);
			}
			else {
				back_positive_prob = 1.0 - actual_static_state.double_params[9];
			}
			for (int i = 0; i < 32; ++i) {
				temp = i;
				actual_static_state.double_params[current_index + i] = 1.0;
				if (temp % 2 == 1) {
					actual_static_state.double_params[current_index + i] *= front_positive_prob;
				}
				else {
					actual_static_state.double_params[current_index + i] *= 1.0 - front_positive_prob;
				}
				temp /= 2;
				if (temp % 2 == 1) {
					actual_static_state.double_params[current_index + i] *= front_diag_positive_prob;
				}
				else {
					actual_static_state.double_params[current_index + i] *= 1.0 - front_diag_positive_prob;
				}
				temp /= 2;
				if (temp % 2 == 1) {
					actual_static_state.double_params[current_index + i] *= side_positive_prob;
				}
				else {
					actual_static_state.double_params[current_index + i] *= 1.0 - side_positive_prob;
				}
				temp /= 2;
				if (temp % 2 == 1) {
					actual_static_state.double_params[current_index + i] *= back_diag_positive_prob;
				}
				else {
					actual_static_state.double_params[current_index + i] *= 1.0 - back_diag_positive_prob;
				}
				temp /= 2;
				if (temp % 2 == 1) {
					actual_static_state.double_params[current_index + i] *= back_positive_prob;
				}
				else {
					actual_static_state.double_params[current_index + i] *= 1.0 - back_positive_prob;
				}
			}
		}
	}
}
#include "lane_merger_domain.h"
#include<iostream>

LaneMergerDomain::LaneMergerDomain()
{
	OBSERVATION_FILL.resize(32);
	int temp = 0;
	for (int i = 0; i < 32; ++i) {
		temp = i;
		OBSERVATION_FILL[i].bool_params.resize(5);
		for (int j = 0; j < 5; ++j) {
			OBSERVATION_FILL[i].bool_params[j] = temp % 2 == 1;
			temp /= 2;
		}
	}
}

void LaneMergerDomain::all_possible_actions(const DynamicState &dynamic_ref, const StaticState &static_ref, std::vector<Action> &out_actions) const
{
	out_actions.resize(4);
	out_actions[0].action_name = "increase_speed";
	out_actions[1].action_name = "decrease_speed";
	out_actions[2].action_name = "keep_speed";
	out_actions[3].action_name = "merge";
}

bool LaneMergerDomain::can_create_observation(const Action &action_ref) const
{
	return true;
}

Domain* LaneMergerDomain::create_copy() const
{
	return new LaneMergerDomain(*this);
}

bool LaneMergerDomain::next_state_distribution(const DynamicState &dynamic_ref, const StaticState &static_ref, const Action &action_ref, std::vector<DynamicState> &out_states, std::vector<float> &out_probs) const
{
	bool include_increase = dynamic_ref.integer_params[3] != static_ref.integer_params[5], include_decrease = dynamic_ref.integer_params[3] != static_ref.integer_params[6];
	unsigned int next_state_count = 1u + (include_increase ? 1u : 0u) + (include_decrease ? 1u : 0u);
	if (dynamic_ref.terminated) {
		out_states.clear();
		out_probs.clear();
		return false;
	}
	if (dynamic_ref.stuck) {
		out_states.resize(1);
		out_probs.resize(1);
		out_states[0] = dynamic_ref;
		out_probs[0] = 1.0;
		return true;
	}
	if (action_ref.action_name == "merge") {
		move_cars(dynamic_ref, static_ref, out_states, out_probs, dynamic_ref.integer_params[1], true);
	}
	else if (action_ref.action_name == "keep_speed") {
		move_cars(dynamic_ref, static_ref, out_states, out_probs, dynamic_ref.integer_params[1], false);
	}
	else if (action_ref.action_name == "increase_speed") {
		if (static_ref.integer_params[4] == dynamic_ref.integer_params[1]) {
			out_states.resize(1);
			out_probs.resize(1);
			out_states[0] = dynamic_ref;
			out_states[0].stuck = true;
			out_probs[0] = 1.0f;
			return true;
		}
		move_cars(dynamic_ref, static_ref, out_states, out_probs, dynamic_ref.integer_params[1] + 1, false);
	}
	else if (action_ref.action_name == "decrease_speed") {
		if (dynamic_ref.integer_params[1] == 0) {
			out_states.resize(1);
			out_probs.resize(1);
			out_states[0] = dynamic_ref;
			out_states[0].stuck = true;
			out_probs[0] = 1.0f;
			return true;
		}
		move_cars(dynamic_ref, static_ref, out_states, out_probs, dynamic_ref.integer_params[1] - 1, false);
	}
	else {
		out_states.resize(1);
		out_probs.resize(1);
		out_states[0] = dynamic_ref;
		out_states[0].stuck = true;
		out_probs[0] = 1.0;
		return true;
	}
	return true;
}

bool LaneMergerDomain::observation_distribution(const DynamicState &dynamic_ref, const StaticState &static_ref, const Action &action_ref, std::vector<Observation> &obs_dist, std::vector<float> &obs_probs) const
{
	if (dynamic_ref.terminated || dynamic_ref.stuck) {
		obs_dist.resize(1);
		obs_probs.resize(1);
		obs_probs[0] = 1.0f;
		obs_dist[0].bool_params.clear();
		return true;
	}

	int current_index = 32 * ((dynamic_ref.integer_params[0] * (static_ref.integer_params[0] + 1)) + dynamic_ref.integer_params[2]);
	current_index += 10;
	obs_dist = OBSERVATION_FILL;
	obs_probs.resize(32);
	for (int i = 0; i < 32; ++i) {
		obs_probs[i] = static_ref.double_params[current_index + i];
	}

	return true;
}

float LaneMergerDomain::reward_function(const StaticState &static_ref, const DynamicState &dynamic_ref, const Action &action_ref, const DynamicState &next_state_ref) const
{
	if (next_state_ref.terminated) {
		return 0.0f;
	}
	return 1.0f;
}

inline void LaneMergerDomain::move_cars(const DynamicState &dynamic_ref, const StaticState &static_ref, std::vector<DynamicState> &next_states, std::vector<float> &next_state_probs, int new_speed, bool perform_merge) const
{
	bool include_increase = false, include_decrease = false;
	if (dynamic_ref.integer_params[2] <= static_ref.integer_params[0]) {
		include_increase = dynamic_ref.integer_params[3] != static_ref.integer_params[6];
		include_decrease = dynamic_ref.integer_params[3] != static_ref.integer_params[5];
	}
	unsigned int next_state_count = 1u + (include_increase ? 1u : 0u) + (include_decrease ? 1u : 0u);
	next_states.resize(next_state_count);
	next_state_probs.resize(next_state_count);
	next_states[0] = dynamic_ref;
	next_states[0].integer_params[1] = new_speed;
	next_states[0].integer_params[0] += next_states[0].integer_params[1];
	next_states[0].bool_params[1] = abs(next_states[0].integer_params[0] - next_states[0].integer_params[2]) <= 1;
	next_states[0].bool_params[0] = (next_states[0].integer_params[2] - next_states[0].integer_params[0] - next_states[0].integer_params[1] > 2) || (next_states[0].integer_params[0] - next_states[0].integer_params[2] - next_states[0].integer_params[3] > 2);
	
	next_states[0].stuck = static_ref.integer_params[0] <= next_states[0].integer_params[0];
	next_state_probs[0] = 1.0f;
	if (include_increase) {
		next_states[1] = next_states[0];

		++next_states[1].integer_params[3];
		next_states[1].integer_params[2] += next_states[1].integer_params[3];
		next_states[1].bool_params[1] = abs(next_states[1].integer_params[0] - next_states[1].integer_params[2]) <= 1;
		next_states[1].bool_params[0] = (next_states[1].integer_params[2] - next_states[1].integer_params[0] - next_states[1].integer_params[1] > 2) || (next_states[1].integer_params[0] - next_states[1].integer_params[2] - next_states[1].integer_params[3] > 2);
		if (perform_merge) {
			if ((next_states[1].integer_params[2] - next_states[1].integer_params[0] - next_states[1].integer_params[1] > 1) || (next_states[1].integer_params[0] - next_states[1].integer_params[2] - next_states[1].integer_params[3] > 1)) {
				//If the other car was already ahead of the agent such that the entire merge length is at least 1 length behind it's safe.
				next_states[1].terminated = true;
			}
			else {
				next_states[1].stuck = true;
			}
		}

		next_state_probs[1] = 0.5 * powf(2.0f, -1.0f * std::abs(static_ref.integer_params[5] - next_states[1].integer_params[3]));
		next_state_probs[0] -= next_state_probs[1];
	}
	if (include_decrease) {
		next_states.back() = next_states[0];

		--next_states.back().integer_params[3];
		next_states.back().integer_params[2] += next_states.back().integer_params[3];
		next_states.back().bool_params[1] = abs(next_states.back().integer_params[0] - next_states.back().integer_params[2]) <= 1;
		next_states.back().bool_params[0] = (next_states.back().integer_params[2] - next_states.back().integer_params[0] - next_states.back().integer_params[1] > 2) || (next_states.back().integer_params[0] - next_states.back().integer_params[2] - next_states.back().integer_params[3] > 2);
		if (perform_merge) {
			if ((next_states.back().integer_params[2] - next_states.back().integer_params[0] - next_states.back().integer_params[1] > 1) || (next_states.back().integer_params[0] - next_states.back().integer_params[2] - next_states.back().integer_params[3] > 1)) {
				//If the other car was already ahead of the agent such that the entire merge length is at least 1 length behind it's safe.
				next_states.back().terminated = true;
			}
			else {

				next_states.back().stuck = true;
			}
		}

		next_state_probs.back() = 0.4 * powf(2.0f, -1.0f * std::abs(static_ref.integer_params[5] - next_states.back().integer_params[3]));
		next_state_probs[0] -= next_state_probs.back();
	}

	if (perform_merge) {
		if ((next_states[0].integer_params[2] - next_states[0].integer_params[0] - next_states[0].integer_params[1] > 1) || (next_states[0].integer_params[0] - next_states[0].integer_params[2] - next_states[0].integer_params[3] > 1)) {
			//If the other car was already ahead of the agent such that the entire merge length is at least 1 length behind it's safe.
			next_states[0].terminated = true;
		}
		else {
			next_states[0].stuck = true;
		}
	}
}
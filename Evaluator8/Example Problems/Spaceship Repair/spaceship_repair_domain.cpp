#include "spaceship_repair_domain.h"
#include<iostream>

void SpaceshipRepairDomain::all_possible_actions(const DynamicState &dynamic_ref, const StaticState &static_ref, std::vector<Action> &all_possible_actions) const
{
	if (dynamic_ref.stuck || dynamic_ref.terminated) {
		all_possible_actions.clear();
	}
	all_possible_actions.resize(3);
	all_possible_actions[0].action_name = "fix";
	all_possible_actions[0].integer_params.resize(1);
	all_possible_actions[0].integer_params[0] = 0;
	all_possible_actions[1].action_name = "fix";
	all_possible_actions[1].integer_params.resize(1);
	all_possible_actions[1].integer_params[0] = 1;
	all_possible_actions[2].action_name = "wait";
}

bool SpaceshipRepairDomain::can_create_observation(const Action &action_ref) const
{
	return true;
}

Domain* SpaceshipRepairDomain::create_copy() const
{
	return new SpaceshipRepairDomain(*this);
}

bool SpaceshipRepairDomain::next_state_distribution(const DynamicState &dynamic_ref, const StaticState &static_ref, const Action &action_ref, std::vector<DynamicState> &next_state_dist, std::vector<float> &next_state_probs) const
{
	if (dynamic_ref.terminated) {
		return false;
	}
	next_state_dist.resize(1);
	next_state_probs.resize(1);
	next_state_probs[0] = 1.0f;
	next_state_dist[0] = dynamic_ref;
	if (dynamic_ref.stuck) {
		return true;
	}
	if (action_ref.action_name == "fix") {
		if (action_ref.integer_params[0] == 0) {
			--next_state_dist[0].integer_params[0];
			if (next_state_dist[0].integer_params[0] <= static_ref.integer_params[0]) {
				if (next_state_dist[0].bool_params[0]) {
					next_state_dist[0].terminated = true;
				}
				else {
					next_state_dist[0].stuck = true;
				}
			}
			return true;
		}
		else if (action_ref.integer_params[0] == 1) {
			++next_state_dist[0].integer_params[0];
			if (next_state_dist[0].integer_params[0] >= static_ref.integer_params[1]) {
				if (next_state_dist[0].bool_params[1]) {
					next_state_dist[0].terminated = true;
				}
				else {
					next_state_dist[0].stuck = true;
				}
			}
			return true;
		}
	}
	else if (action_ref.action_name == "wait") {
		return true;
	}

	next_state_dist[0].stuck = true;
	return true;
}

bool SpaceshipRepairDomain::observation_distribution(const DynamicState &dynamic_ref, const StaticState &static_ref, const Action &action_ref, std::vector<Observation> &observation_dist, std::vector<float> &observation_prob) const
{
	if (dynamic_ref.terminated || dynamic_ref.stuck) {
		observation_dist.resize(1);
		observation_prob.resize(1);
		observation_prob[0] = 1.0f;
		observation_dist[0].bool_params.clear();
		return true;
	}

	observation_dist.resize(4);
	observation_prob = { (float)static_ref.double_params[0], (float)static_ref.double_params[1], (float)static_ref.double_params[2], (float)static_ref.double_params[3] };

	observation_dist[0].bool_params.resize(2);
	observation_dist[0].bool_params[0] = !dynamic_ref.bool_params[0];
	observation_dist[0].bool_params[1] = !dynamic_ref.bool_params[1];

	observation_dist[1].bool_params.resize(2);
	observation_dist[1].bool_params[0] = !dynamic_ref.bool_params[0];
	observation_dist[1].bool_params[1] = dynamic_ref.bool_params[1];

	observation_dist[2].bool_params.resize(2);
	observation_dist[2].bool_params[0] = dynamic_ref.bool_params[0];
	observation_dist[2].bool_params[1] = !dynamic_ref.bool_params[1];

	observation_dist[3].bool_params.resize(2);
	observation_dist[3].bool_params[0] = dynamic_ref.bool_params[0];
	observation_dist[3].bool_params[1] = dynamic_ref.bool_params[1];
	return true;
}

float SpaceshipRepairDomain::reward_function(const StaticState &static_ref, const DynamicState &dynamic_ref, const Action &action_ref, const DynamicState &next_state_ref) const
{
	if (next_state_ref.terminated) {
		return 0.0f;
	}
	return 1.0f;
}
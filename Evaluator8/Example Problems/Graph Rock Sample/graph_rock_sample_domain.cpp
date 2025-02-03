#include "graph_rock_sample_domain.h"

void GraphRockSampleDomain::all_possible_actions(const DynamicState& dynamic_ref, const StaticState& static_ref, std::vector<Action>& out_actions) const
{
    if (dynamic_ref.terminated || dynamic_ref.stuck) {
        out_actions.clear();
        return;
    }

}

bool GraphRockSampleDomain::can_create_observation(const Action& action_ref) const
{
    return action_ref.action_name == "scan" || action_ref.action_name == "sample";
}

Domain* GraphRockSampleDomain::create_copy() const
{
    return new GraphRockSampleDomain(*this);
}

bool GraphRockSampleDomain::next_state_distribution(const DynamicState& dynamic_ref, const StaticState& static_ref, const Action& action_ref, std::vector<DynamicState>& out_states, std::vector<float>& out_probs) const
{
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
    if (action_ref.action_name == "scan") {
        out_states.resize(1);
        out_probs.resize(1);
        out_states[0] = dynamic_ref;
        out_probs[0] = 1.0;
        if (action_ref.integer_params[0] < 1 || action_ref.integer_params[0] > static_ref.integer_params[0]) {
            //Trying to scan something that isn't a rock.
            out_states[0].stuck = true;
        }
        return true;
    }
    if (action_ref.action_name == "sample") {
        out_states.resize(1);
        out_probs.resize(1);
        out_states[0] = dynamic_ref;
        out_probs[0] = 1.0;
        if (dynamic_ref.integer_params[0] < 1 || dynamic_ref.integer_params[0] > static_ref.integer_params[0]) {
            //Location doesn't have a rock.
            out_states[0].stuck = true;
        }
        else if (dynamic_ref.bool_params[static_ref.integer_params[dynamic_ref.integer_params[0] + 2]]) {
            //Already sampled that type.
            out_states[0].stuck = true;
        }
        else if (dynamic_ref.bool_params[dynamic_ref.integer_params[0] - 1 + static_ref.integer_params[2]]) {
            //Rock was worth sampling.
            out_states[0].bool_params[static_ref.integer_params[dynamic_ref.integer_params[0] + 2]] = true;
            out_states[0].bool_params[dynamic_ref.integer_params[0] - 1 + static_ref.integer_params[2]] = false;
        }
        else {
            out_states[0].stuck = true;
        }
        return true;
    }
    if (action_ref.action_name == "goto") {
        out_states.resize(1);
        out_probs.resize(1);
        out_states[0] = dynamic_ref;
        out_probs[0] = 1.0;
        if (static_ref.bool_params[dynamic_ref.integer_params[0] + (static_ref.integer_params[1] * action_ref.integer_params[0])]){
            out_states[0].integer_params[0] = action_ref.integer_params[0];
            if (out_states[0].integer_params[0] == static_ref.integer_params[1] - 1) {
                //If this is the drop off location, it checks that each type has been sampled or there wasn't a good rock of that rock type to be sampled.
                for (int i = 0; i < static_ref.integer_params[2]; ++i) {
                    if (!out_states[0].bool_params[i]) {
                        for (int current_rock = 0; current_rock < static_ref.integer_params[0]; ++current_rock) {
                            if (out_states[0].bool_params[i + static_ref.integer_params[2]] && static_ref.integer_params[i + 3]) {
                                out_states[0].stuck = true;
                                return true;
                            }
                        }
                    }
                }
                out_states[0].terminated = true;
            }
        }
        else {
            out_states[0].stuck = true;
        }
        return true;
    }
    out_states.clear();
    out_probs.clear();
    return false;
}
bool GraphRockSampleDomain::observation_distribution(const DynamicState &dynamic_ref, const StaticState &static_ref, const Action &action_ref, std::vector<Observation> &observation_dist, std::vector<float> &observation_probs) const
{
    if (dynamic_ref.terminated || dynamic_ref.stuck) {
        observation_dist.resize(1);
        observation_probs.resize(1);
        observation_probs[0] = 1.0f;
        observation_dist[0].bool_params.clear();
    }
    else if (action_ref.action_name == "scan") {
        observation_dist.resize(2);
        observation_probs.resize(2);
        observation_dist[0].bool_params.resize(1);
        observation_dist[1].bool_params.resize(1);
        observation_probs[0] = static_ref.double_params[((static_ref.integer_params[1] - 1) * static_ref.integer_params[0]) + dynamic_ref.integer_params[0] + ((static_ref.integer_params[1] - 1) * (action_ref.integer_params[0] - 1))];
        observation_probs[1] = 1.0 - observation_probs[0];
        observation_dist[0].bool_params[0] = dynamic_ref.bool_params[static_ref.integer_params[2] + action_ref.integer_params[0] - 1];
        observation_dist[1].bool_params[0] = !observation_dist[0].bool_params[0];
    }
    else if (action_ref.action_name == "sample") {
        observation_dist.resize(1);
        observation_probs.resize(1);
        observation_probs[0] = 1.0f;
        observation_dist[0].bool_params.resize(1);
        observation_dist[0].bool_params[0] = true;
    }
    else {
        observation_dist.resize(1);
        observation_probs.resize(1);
        observation_probs[0] = 1.0f;
        observation_dist[0].bool_params.clear();
    }
    return true;
}

float GraphRockSampleDomain::reward_function(const StaticState &static_ref, const DynamicState &dynamic_ref, const Action &action_ref, const DynamicState &next_state_ref) const
{
    if (dynamic_ref.terminated) {
        return 0.0f;
    }

    return -1.0f;
}
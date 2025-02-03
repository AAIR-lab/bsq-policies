#include "eval_policy_dfs.h"
#include <iostream>

float evalPolicyDFS(BeliefState* belief_state, const Policy& policy, const Problem& problem, const std::vector<float>& parameter_values,
    unsigned int horizon, float discount_factor, OrthotopeList& out_constraint) {

    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;
    Observation current_observation;
    belief_state->build_initial_belief(problem);
    bool is_otherwise_action = true;
    Action otherwise_action;
    for (unsigned int current = 0; current < horizon; ++current) {
        policy.build_policy_distribution(belief_state->static_state, belief_state->state_list, belief_state->probabilities, current_constraint_list, current_action_list, otherwise_action);
        is_otherwise_action = true;
        // std::cout << "Current: " << out_constraint << std::endl;
        // std::cout << "Parameter Values: " << parameter_values[0] << std::endl;
        // std::cout << "========================================" << std::endl;
        // std::cout << "All rules" << std::endl;
        // for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
        //     ++constraint_it, ++action_it) {
            
        //     std::cout << *constraint_it << " --> " << action_it->action_name << std::endl;
        // }

        for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
            ++constraint_it, ++action_it) {
            if (constraint_it->is_satisfied(parameter_values)) {
                out_constraint.and_update(*constraint_it);

                belief_state->apply_action(*action_it);
                if (belief_state->problem_domain->can_create_observation(*action_it)) {
                    belief_state->generate_observation(*action_it, current_observation);
                    belief_state->apply_observation(*action_it, current_observation);
                }
                belief_state->calculate_termination_stuck_percent();
                //belief_state->update_terminated();
                is_otherwise_action = false;
                break;
            }
            else {
                out_constraint.not_update(*constraint_it);
            }
        }

        // if(!out_constraint.is_satisfiable()){
        //     std::cout << "UNSAT" << std::endl;
        //     exit(1);
        // }

        if (is_otherwise_action) {
            belief_state->apply_action(otherwise_action);
            if (belief_state->problem_domain->can_create_observation(otherwise_action)) {
                belief_state->generate_observation(otherwise_action, current_observation);
                belief_state->apply_observation(otherwise_action, current_observation);
            }
            belief_state->calculate_termination_stuck_percent();
            //belief_state->update_terminated();
        }
        if (belief_state->terminated) {
            return ((-1.0f * (current + 1) * belief_state->goal_state_prob)) + ((belief_state->goal_state_prob - 1.0f) * horizon);
        }
    }

    //return belief_state->calculate_expected_value(discount_factor);
    return -1.0f * horizon;
}

void evalPolicyDFS_multi(BeliefState* belief_state, const Policy& policy, const Problem& problem, const std::vector<float>& parameter_values,
    unsigned int horizon, float discount_factor, OrthotopeList& out_constraint, std::list<float> &out_evs, float &out_ev,unsigned int evaluation_number) {

    OrthotopeList temp(out_constraint);
    float temp_ec = 0.0f;

    for(unsigned int i = 0;i < evaluation_number; ++i){
        out_evs.push_back(evalPolicyDFS(belief_state, policy, problem, parameter_values, horizon, discount_factor, temp));
        temp_ec += out_evs.back();
    }
    out_constraint = temp;
    out_ev = temp_ec / (float) evaluation_number;
}

float evalPolicyDFS(BeliefState* belief_state, const Policy& policy, const Problem& problem, const std::vector<float>& parameter_values,
    unsigned int horizon, float discount_factor) {
    std::list<OrthotopeList> current_constraint_list;
    std::list<Action> current_action_list;
    std::list<OrthotopeList>::iterator constraint_it;
    std::list<Action>::iterator action_it;
    Observation current_observation;
    belief_state->build_initial_belief(problem);
    bool is_otherwise_action = true;
    Action otherwise_action;
    for (unsigned int current = 0; current < horizon; ++current) {
        policy.build_policy_distribution(belief_state->static_state, belief_state->state_list, belief_state->probabilities, current_constraint_list, current_action_list, otherwise_action);
        is_otherwise_action = true;
        for (constraint_it = current_constraint_list.begin(), action_it = current_action_list.begin(); constraint_it != current_constraint_list.end();
            ++constraint_it, ++action_it) {
            if (constraint_it->is_satisfied(parameter_values)) {
                belief_state->apply_action(*action_it);
                if (belief_state->problem_domain->can_create_observation(*action_it)) {
                    belief_state->generate_observation(*action_it, current_observation);
                    belief_state->apply_observation(*action_it, current_observation);
                }
                belief_state->calculate_termination_stuck_percent();
                //belief_state->update_terminated();
                is_otherwise_action = false;
                break;
            }
        }

        if (is_otherwise_action) {
            belief_state->apply_action(otherwise_action);
            if (belief_state->problem_domain->can_create_observation(otherwise_action)) {
                belief_state->generate_observation(otherwise_action, current_observation);
                belief_state->apply_observation(otherwise_action, current_observation);
            }
            belief_state->calculate_termination_stuck_percent();
            //belief_state->update_terminated();
        }
        if (belief_state->terminated) {
            return ((-1.0f * (current + 1) * belief_state->goal_state_prob)) + ((belief_state->goal_state_prob - 1.0f) * horizon);
        }
    }

    //return belief_state->calculate_expected_value(discount_factor);
    return -1.0f * horizon;
}
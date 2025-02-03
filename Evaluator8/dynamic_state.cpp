#include "dynamic_state.h"
#include "utility.h"
#include<string.h>

float DynamicState::calculate_expected_value(float discount_factor) const {
    float output = 0.0f;
    for (std::list<float>::const_reverse_iterator it = rewards.crbegin(); it != rewards.crend(); ++it) {
        output *= discount_factor;
        output += *it;
    }
    return output;
}

bool DynamicState::operator== (const DynamicState& ref) const {
    if (rewards != ref.rewards) {
        return false;
    }
    if (string_params != ref.string_params) {
        return false;
    }
    if (integer_params != ref.integer_params) {
        return false;
    }
    if (bool_params != ref.bool_params) {
        return false;
    }
    if (double_params != ref.double_params) {
        return false;
    }
    return terminated == ref.terminated;
}

void add_to_sorted(std::list<DynamicState>& target, std::list<float>& target_probs, const DynamicState& ref, const float& prob_ref)
{
    /*std::list<DynamicState>::iterator target_it = target.begin();
    std::list<float>::iterator prob_it = target_probs.begin();
    for (; target_it != target.end(); ++target_it, ++prob_it) {
        if (*target_it == ref) {
            *prob_it += prob_ref;
            return;
        }
    }
    target.emplace_back(ref);
    target_probs.emplace_back(prob_ref);*/
    std::list<DynamicState>::iterator target_it = target.begin();
    std::list<float>::iterator prob_it = target_probs.begin();
    bool not_placed = true, move_forward = false;
    bool at_string = false, at_double = false, at_int = false, at_bool = false;
    int temp_strcmp_value = 0;
    while (not_placed) {
        if (target_it == target.end()) {
            target.emplace_back(ref);
            target_probs.push_back(prob_ref);
            break;
        }

        if (target_it->terminated) {
            if (!ref.terminated) {
                target.emplace(target_it, ref);
                target_probs.emplace(prob_it, prob_ref);
                return;
            }
        }
        else if (ref.terminated) {
            ++target_it;
            ++prob_it;
            continue;
        }

        move_forward = false;
        if (at_string) {
            goto string_start;
        }
        if (at_double) {
            goto double_start;
        }
        if (at_int) {
            goto integer_start;
        }
        if (at_bool) {
            goto bool_start;
        }

        //TODO: can place states with different rewards together??
        for (std::list<float>::const_iterator reward_it = target_it->rewards.cbegin(), ref_reward_it = ref.rewards.cbegin();
            reward_it != target_it->rewards.cend(); ++reward_it, ++ref_reward_it) {
            if (is_close(*reward_it, *ref_reward_it)) {
                continue;
            }
            else if (*reward_it < *ref_reward_it) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else {
                target.emplace(target_it, ref);
                target_probs.emplace(prob_it, prob_ref);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            at_bool = true;
        }
bool_start:
        for (std::vector<bool>::const_iterator bool_it = target_it->bool_params.cbegin(), ref_bool_it = ref.bool_params.cbegin();
            bool_it != target_it->bool_params.cend(); ++bool_it, ++ref_bool_it) {
            if (*bool_it) {
                if (*ref_bool_it) {
                    continue;
                }
                else {
                    target.emplace(target_it, ref);
                    target_probs.emplace(prob_it, prob_ref);
                    return;
                }
            }
            else if (*ref_bool_it) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else {
                continue;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            at_int = true;
        }
integer_start:
        for (std::vector<int>::const_iterator int_it = target_it->integer_params.cbegin(), ref_int_it = ref.integer_params.cbegin();
            int_it != target_it->integer_params.cend(); ++int_it, ++ref_int_it) {

            if (*int_it < *ref_int_it) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else if (*int_it > *ref_int_it) {
                target.emplace(target_it, ref);
                target_probs.emplace(prob_it, prob_ref);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            at_double = true;
        }
double_start:
        for (std::vector<double>::const_iterator double_it = target_it->double_params.cbegin(), ref_double_it = ref.double_params.cbegin();
            double_it != target_it->double_params.cend(); ++double_it, ++ref_double_it) {

            if (is_close(*double_it, *ref_double_it)) {
                continue;
            }
            else if (*double_it < *ref_double_it) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else {
                target.emplace(target_it, ref);
                target_probs.emplace(prob_it, prob_ref);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            at_string = true;
        }
string_start:
        for (std::vector<std::string>::const_iterator string_it = target_it->string_params.cbegin(), ref_string_it = ref.string_params.cbegin();
            string_it != target_it->string_params.cend(); ++string_it, ++ref_string_it) {
            temp_strcmp_value = string_it->compare(*ref_string_it);
            if (temp_strcmp_value < 0) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else if (temp_strcmp_value > 0) {
                target.emplace(target_it, ref);
                target_probs.emplace(prob_it, prob_ref);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            //Reaching here means the two states are the same!
            *prob_it += prob_ref;
            return;
        }
    }
}
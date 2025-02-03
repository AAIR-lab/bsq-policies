#include "observation.h"
#include "utility.h"

bool Observation::operator== (const Observation& ref) const
{
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
    return true;
}

bool Observation::operator< (const Observation& ref) const
{
    for (std::vector<bool>::const_iterator it = bool_params.begin(), ref_it = ref.bool_params.begin(); it != bool_params.end(); ++it, ++ref_it) {
        if (*it) {
            if (!*ref_it) {
                return false;
            }
        }
        else if (*ref_it) {
            return true;
        }
    }
    for (std::vector<int>::const_iterator it = integer_params.begin(), ref_it = ref.integer_params.begin(); it != integer_params.end(); ++it, ++ref_it) {
        if (*it < *ref_it) {
            return true;
        }
        if (*it > *ref_it) {
            return false;
        }
    }
    for (std::vector<std::string>::const_iterator it = string_params.begin(), ref_it = ref.string_params.begin(); it != string_params.end(); ++it, ++ref_it) {
        if (*it < *ref_it) {
            return true;
        }
        if (*it > *ref_it) {
            return false;
        }
    }

    for (std::vector<double>::const_iterator it = double_params.begin(), ref_it = ref.double_params.begin(); it != double_params.end(); ++it, ++ref_it) {
        if (is_close(*it, *ref_it)) {
            continue;
        }
        return *it < *ref_it;
    }
    return false;
}

int Observation::compare(const Observation& ref) const
{
    for (std::vector<bool>::const_iterator it = bool_params.begin(), ref_it = ref.bool_params.begin(); it != bool_params.end(); ++it, ++ref_it) {
        if (*it) {
            if (!*ref_it) {
                return 1;
            }
        }
        else if (*ref_it) {
            return -1;
        }
    }
    for (std::vector<int>::const_iterator it = integer_params.begin(), ref_it = ref.integer_params.begin(); it != integer_params.end(); ++it, ++ref_it) {
        if (*it < *ref_it) {
            return -1;
        }
        if (*it > *ref_it) {
            return 1;
        }
    }
    for (std::vector<std::string>::const_iterator it = string_params.begin(), ref_it = ref.string_params.begin(); it != string_params.end(); ++it, ++ref_it) {
        if (*it < *ref_it) {
            return -1;
        }
        if (*it > *ref_it) {
            return 1;
        }
    }

    for (std::vector<double>::const_iterator it = double_params.begin(), ref_it = ref.double_params.begin(); it != double_params.end(); ++it, ++ref_it) {
        if (is_close(*it, *ref_it)) {
            continue;
        }
        else if (*it < *ref_it) {
            return -1;
        }
        return 1;
    }
    return 0;
}

void add_to_sorted(std::list<Observation>& target, std::list<float>& target_probs, const Observation& ref_obs, const float& ref_prob)
{
    //TODO: probably more efficient solution.
    std::list<Observation>::iterator target_it = target.begin();
    std::list<float>::iterator prob_it = target_probs.begin();
    bool not_placed = true, move_forward = false;
    int temp_strcmp_value = 0;
    while (not_placed) {
        if (target_it == target.end()) {
            target.emplace_back(ref_obs);
            target_probs.push_back(ref_prob);
            break;
        }
        move_forward = false;
        for (std::vector<bool>::const_iterator bool_it = target_it->bool_params.cbegin(), ref_bool_it = ref_obs.bool_params.cbegin();
            bool_it != target_it->bool_params.cend(); ++bool_it, ++ref_bool_it) {
            if (*bool_it) {
                if (*ref_bool_it) {
                    continue;
                }
                else {
                    target.emplace(target_it, ref_obs);
                    target_probs.emplace(prob_it, ref_prob);
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
        for (std::vector<int>::const_iterator int_it = target_it->integer_params.cbegin(), ref_int_it = ref_obs.integer_params.cbegin();
            int_it != target_it->integer_params.cend(); ++int_it, ++ref_int_it) {

            if (*int_it < *ref_int_it) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else if (*int_it > *ref_int_it) {
                target.emplace(target_it, ref_obs);
                target_probs.emplace(prob_it, ref_prob);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        for (std::vector<double>::const_iterator double_it = target_it->double_params.cbegin(), ref_double_it = ref_obs.double_params.cbegin();
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
                target.emplace(target_it, ref_obs);
                target_probs.emplace(prob_it, ref_prob);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        for (std::vector<std::string>::const_iterator string_it = target_it->string_params.cbegin(), ref_string_it = ref_obs.string_params.cbegin();
            string_it != target_it->string_params.cend(); ++string_it, ++ref_string_it) {
            temp_strcmp_value = string_it->compare(*ref_string_it);
            if (temp_strcmp_value < 0) {
                ++target_it;
                ++prob_it;
                move_forward = true;
                break;
            }
            else if (temp_strcmp_value > 0) {
                target.emplace(target_it, ref_obs);
                target_probs.emplace(prob_it, ref_prob);
                return;
            }
        }
        if (move_forward) {
            continue;
        }
        else {
            //Reaching here means the two states are the same!
            *prob_it += ref_prob;
            return;
        }
    }
}
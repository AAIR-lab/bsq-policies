#include "function_data.h"
#include<string>
#include<stdexcept>

QueryInstance* function_data::build_instance(const std::string &ref, const Problem &problem_ref) const
{
    //The string should have the form [function name](parameter_1,...,parameter_n)
    if (ref.empty()) {
        return nullptr;
    }
    bool negated = ref[0] == '!';
    if (negated) {
        if (!ref.starts_with('!' + function_name)) {
            return nullptr;
        }
    }
    else if (!ref.starts_with(function_name)) {
        return nullptr;
    }
    switch (function_type) {
    case type_bool:
        return build_boolean_instance(ref, negated);
        break;
    case type_double:
        return build_double_instance(ref, negated);
        break;
    case type_int:
        return build_integer_instance(ref, negated);
        break;
    default:
    case type_string:
        return build_string_instance(ref, negated);
        break;
    }
}

QueryInstance* function_data::build_boolean_instance(const std::string &ref, bool negated) const
{
    unsigned int index_offset = 0u, offset_multiplier = 1u;
    size_t found_index, current_index = function_name.size();
    std::string temp_str;
    std::map<std::string, unsigned int>::const_iterator found_it;
    unsigned int max_element = 0u;
    BoolQueryInstance* output = nullptr;
    if (negated) {
        current_index += 2u;
    }
    else {
        current_index += 1u;
    }
    if (!parameter_ranges.empty()) {
        for (int current_parameter = 0; current_parameter < parameter_ranges.size(); ++current_parameter) {
            if (parameter_ranges.size() - 1 == current_parameter) {
                found_index = ref.find_first_of(')', current_index);
            }
            else {
                found_index = ref.find_first_of(',', current_index);
            }

            if (found_index == std::string::npos || found_index == current_index) {
                return nullptr;
            }
            //Having an output value or a non-constant parameter type are currently not supported for boolean functions.
            if (parameter_ranges[current_parameter].output_value || parameter_ranges[current_parameter].type != para_type_constant) {
                return nullptr;
            }
            if (parameter_ranges[current_parameter].allowed_constants.empty()) {
                return nullptr;
            }
            temp_str = ref.substr(current_index, found_index - current_index);
            found_it = parameter_ranges[current_parameter].allowed_constants.find(temp_str);
            if (found_it == parameter_ranges[current_parameter].allowed_constants.cend()) {
                return nullptr;
            }
            index_offset += offset_multiplier * found_it->second;
            offset_multiplier *= parameter_ranges[current_parameter].allowed_constants.size();
            current_index = found_index + 1u;
        }

    }
    else {
        current_index += 1u;
    }
    if (current_index != ref.size()) {
        return nullptr;
    }

    output = new BoolQueryInstance(index_offset + index);
    output->negated = negated;
    output->dynamic = dynamic;
    output->full_observable = full_observable;
    return output;
}

QueryInstance* function_data::build_double_instance(const std::string &ref, bool negated) const
{
    unsigned int index_offset = 0u, offset_multiplier = 1u;
    double target_value = 0.0;
    bool found_target = false;
    size_t found_index, current_index = function_name.size();
    std::string temp_str;
    std::map<std::string, unsigned int>::const_iterator found_it;
    DoubleQueryInstance* output = nullptr;
    if (negated) {
        current_index += 2u;
    }
    else {
        current_index += 1u;
    }
    if (!parameter_ranges.empty()) {
        for (int current_parameter = 0; current_parameter < parameter_ranges.size(); ++current_parameter) {
            if (parameter_ranges.size() - 1 == current_parameter) {
                found_index = ref.find_first_of(')', current_index);
            }
            else {
                found_index = ref.find_first_of(',', current_index);
            }

            if (found_index == std::string::npos || found_index == current_index) {
                return nullptr;
            }

            //Double functions only support a single function target.
            if (parameter_ranges[current_parameter].output_value) {
                if (found_target || parameter_ranges[current_parameter].type != para_type_double) {
                    return nullptr;
                }
            }
            else if (parameter_ranges[current_parameter].type != para_type_constant) {
                return nullptr;
            }
            if (parameter_ranges[current_parameter].allowed_constants.empty()) {
                return nullptr;
            }
            temp_str = ref.substr(current_index, found_index - current_index);
            if (parameter_ranges[current_parameter].output_value) {
                target_value = std::stod(temp_str);
                found_target = true;
            }
            else {
                found_it = parameter_ranges[current_parameter].allowed_constants.find(temp_str);
                if (found_it == parameter_ranges[current_parameter].allowed_constants.cend()) {
                    return nullptr;
                }
                index_offset += offset_multiplier * found_it->second;
                offset_multiplier *= parameter_ranges[current_parameter].allowed_constants.size();
            }
            current_index = found_index + 1u;
        }

    }
    else {
        current_index += 1u;
    }
    if (current_index != ref.size()) {
        return nullptr;
    }
    if (!found_target) {
        return nullptr;
    }

    output = new DoubleQueryInstance(target_value,index_offset + index);
    output->negated = negated;
    output->dynamic = dynamic;
    output->full_observable = full_observable;
    return output;
}

QueryInstance* function_data::build_integer_instance(const std::string &ref, bool negated) const
{
    unsigned int index_offset = 0u, offset_multiplier = 1u;
    int target_value = 0.0, target_multiplier = 1;
    bool found_target_constant = false, found_target_integer = false;
    size_t found_index, current_index = function_name.size();
    std::string temp_str;
    std::map<std::string, unsigned int>::const_iterator found_it;
    IntegerQueryInstance* output = nullptr;
    if (negated) {
        current_index += 2u;
    }
    else {
        current_index += 1u;
    }
    if (!parameter_ranges.empty()) {
        for (int current_parameter = 0; current_parameter < parameter_ranges.size(); ++current_parameter) {
            if (parameter_ranges.size() - 1 == current_parameter) {
                found_index = ref.find_first_of(')', current_index);
            }
            else {
                found_index = ref.find_first_of(',', current_index);
            }

            if (found_index == std::string::npos || found_index == current_index) {
                return nullptr;
            }

            //Double functions only support a single function target.
            if (parameter_ranges[current_parameter].output_value) {
                if (found_target_integer) {
                    return nullptr;
                }
                else if (found_target_constant && parameter_ranges[current_parameter].type != para_type_constant) {
                    return nullptr;
                }
            }
            else if (parameter_ranges[current_parameter].type != para_type_constant) {
                return nullptr;
            }
            temp_str = ref.substr(current_index, found_index - current_index);
            if (parameter_ranges[current_parameter].output_value) {
                if (parameter_ranges[current_parameter].type == para_type_int) {
                    target_value = std::stoi(temp_str);
                    found_target_integer = true;
                }
                else {
                    if (parameter_ranges[current_parameter].allowed_constants.empty()) {
                        return nullptr;
                    }
                    found_it = parameter_ranges[current_parameter].allowed_constants.find(temp_str);
                    if (found_it == parameter_ranges[current_parameter].allowed_constants.cend()) {
                        return nullptr;
                    }
                    target_value += target_multiplier * found_it->second;
                    target_multiplier *= parameter_ranges[current_parameter].allowed_constants.size();
                    found_target_constant = true;
                }
            }
            else {
                found_it = parameter_ranges[current_parameter].allowed_constants.find(temp_str);
                if (found_it == parameter_ranges[current_parameter].allowed_constants.cend()) {
                    return nullptr;
                }
                index_offset += offset_multiplier * found_it->second;
                offset_multiplier *= parameter_ranges[current_parameter].allowed_constants.size();
            }
            current_index = found_index + 1u;
        }

    }
    else {
        current_index += 1u;
    }
    if (current_index != ref.size()) {
        return nullptr;
    }
    if (!found_target_integer && !found_target_constant) {
        return nullptr;
    }

    output = new IntegerQueryInstance(target_value, index_offset + index);
    output->negated = negated;
    output->dynamic = dynamic;
    output->full_observable = full_observable;
    return output;
}
QueryInstance* function_data::build_string_instance(const std::string &ref, bool negated) const
{
    unsigned int index_offset = 0u, offset_multiplier = 1u;
    std::string target_value = "";
    bool found_target = false;
    size_t found_index, current_index = function_name.size();
    std::string temp_str;
    std::map<std::string, unsigned int>::const_iterator found_it;
    StringQueryInstance* output = nullptr;
    if (negated) {
        current_index += 2u;
    }
    else {
        current_index += 1u;
    }
    if (!parameter_ranges.empty()) {
        for (int current_parameter = 0; current_parameter < parameter_ranges.size(); ++current_parameter) {
            if (parameter_ranges.size() - 1 == current_parameter) {
                found_index = ref.find_first_of(')', current_index);
            }
            else {
                found_index = ref.find_first_of(',', current_index);
            }

            if (found_index == std::string::npos || found_index == current_index) {
                return nullptr;
            }

            //Double functions only support a single function target.
            if (parameter_ranges[current_parameter].output_value) {
                if (found_target || parameter_ranges[current_parameter].type != para_type_int) {
                    return nullptr;
                }
            }
            else if (parameter_ranges[current_parameter].type != para_type_constant) {
                return nullptr;
            }
            if (parameter_ranges[current_parameter].allowed_constants.empty()) {
                return nullptr;
            }
            target_value = ref.substr(current_index, found_index - current_index);
            if (parameter_ranges[current_parameter].output_value) {
                target_value = temp_str;
                found_target = true;
            }
            else {
                found_it = parameter_ranges[current_parameter].allowed_constants.find(temp_str);
                if (found_it == parameter_ranges[current_parameter].allowed_constants.cend()) {
                    return nullptr;
                }
                index_offset += offset_multiplier * found_it->second;
                offset_multiplier *= parameter_ranges[current_parameter].allowed_constants.size();
            }
            current_index = found_index + 1u;
        }

    }
    else {
        current_index += 1u;
    }
    if (current_index != ref.size()) {
        return nullptr;
    }
    if (!found_target) {
        return nullptr;
    }

    output = new StringQueryInstance(target_value, index_offset + index);
    output->negated = negated;
    output->dynamic = dynamic;
    output->full_observable = full_observable;
    return output;
}

Action* action_function_data::build_action(const std::string &ref) const
{
    Action temp_action;
    build_action(ref, temp_action);
    return new Action(temp_action);
}

void action_function_data::build_action(const std::string &string_ref, Action &out_ref) const
{
    out_ref.action_name = action_name;
    out_ref.bool_params.clear();
    out_ref.double_params.clear();
    out_ref.integer_params.clear();
    out_ref.string_params.clear();

    //Will have the form: [action_name](parameter 1,..., parameter n)
    if (!string_ref.starts_with(action_name)) {
        throw std::invalid_argument("Action name doesn't match");
    }
    size_t current_location = action_name.size() + 1;
    size_t found_match;
    std::string temp_str;
    std::map<std::string, unsigned int>::const_iterator found_constant;
    for (int i = 0; i < action_parameters.size(); ++i) {
        if (i == action_parameters.size() - 1) {
            found_match = string_ref.find(')', current_location);
        }
        else {
            found_match = string_ref.find(",", current_location);
        }
        if (found_match == std::string::npos) {
            throw std::invalid_argument("Failed to parse action " + action_name);
        }
        temp_str = string_ref.substr(current_location, found_match - current_location);
        switch (action_parameters[i].type) {
        case para_type_bool:
            if (temp_str.size() == 1) {
                if (temp_str[0] == '0') {
                    out_ref.bool_params.push_back(false);
                }
                else if (temp_str[0] == '1') {
                    out_ref.bool_params.push_back(true);
                }
                else {
                    throw std::invalid_argument("Failed to parse to boolean parameter: " + temp_str);
                }
            }
            else if (temp_str == "True" || temp_str == "true") {
                out_ref.bool_params.push_back(true);
            }
            else if (temp_str == "False" || temp_str == "false") {
                out_ref.bool_params.push_back(false);
            }
            else {
                throw std::invalid_argument("Failed to parse to boolean parameter: " + temp_str);
            }
            break;
        case para_type_constant:
            found_constant = action_parameters[i].allowed_constants.find(temp_str);
            if (found_constant == action_parameters[i].allowed_constants.end()) {
                throw std::invalid_argument("Constant isn't supported: " + temp_str);
            }
            out_ref.integer_params.push_back(found_constant->second);
            break;
        case para_type_double:
            out_ref.double_params.push_back(std::stod(temp_str));
            break;
        case para_type_int:
            out_ref.integer_params.push_back(std::stoi(temp_str));
            break;
        default:
            out_ref.string_params.push_back(temp_str);
            break;
        }
    }
}
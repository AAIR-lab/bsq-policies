#include "problem_analysis.h"
#include<iostream>

void print_constants(const Problem& ref) {
    std::cout << "Constants: ";
    for (std::vector<std::string>::const_iterator it = ref.constants.cbegin(); it != ref.constants.cend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void print_problem_functions(const Problem& ref) {
    std::cout << "Functions:\n";
    for (std::map<std::string, function_data>::const_iterator it = ref.function_information.cbegin(); it != ref.function_information.cend(); ++it) {
        std::cout << "Function Name: " << it->first << std::endl;
        std::cout << "Function Is Dynamic: " << it->second.dynamic << std::endl;
        std::cout << "Function Is Full-Observable: " << it->second.full_observable << std::endl;
        std::cout << "Function Type: " << it->second.function_type << std::endl;
        std::cout << "Function Start Index: " << it->second.index << std::endl;
        std::cout << "Function Parameters: ";
        for (std::vector<parameter_info>::const_iterator sit = it->second.parameter_ranges.cbegin(); sit != it->second.parameter_ranges.cend(); ++sit) {
            std::cout << "(" << sit->output_value << "," << sit->type << "{";
            std::map<std::string, unsigned int>::const_iterator map_it = sit->allowed_constants.cbegin();
            while (map_it != sit->allowed_constants.cend()) {
                std::cout << map_it->first << ":" << map_it->second;
                ++map_it;
                if (map_it != sit->allowed_constants.cend()) {
                    std::cout << " ";
                }
            }
            std::cout << "}) ";
        }
        std::cout << std::endl << "-------------" << std::endl;
    }
}

void print_problem_static_state(const Problem& ref) {
    std::cout << "Static State:\n";
    std::cout << "Boolean(" << ref.actual_static_state.bool_params.size() << "):\n";
    int count = 0;
    for (std::vector<bool>::const_iterator it = ref.actual_static_state.bool_params.cbegin(); it != ref.actual_static_state.bool_params.cend(); ++it) {
        std::cout << count << ":" << *it << std::endl;
        ++count;
    }
    std::cout << "Double(" << ref.actual_static_state.double_params.size() << "):\n";
    count = 0;
    for (std::vector<double>::const_iterator it = ref.actual_static_state.double_params.cbegin(); it != ref.actual_static_state.double_params.cend(); ++it) {
        std::cout << count << ":" << *it << std::endl;
        ++count;
    }
    std::cout << "Int(" << ref.actual_static_state.integer_params.size() << "):\n";
    count = 0;
    for (std::vector<int>::const_iterator it = ref.actual_static_state.integer_params.cbegin(); it != ref.actual_static_state.integer_params.cend(); ++it) {
        std::cout << count << ":" << *it << std::endl;
        ++count;
    }
    std::cout << "String(" << ref.actual_static_state.string_params.size() << "):\n";
    count = 0;
    for (std::vector<std::string>::const_iterator it = ref.actual_static_state.string_params.cbegin(); it != ref.actual_static_state.string_params.cend(); ++it) {
        std::cout << count << ":" << *it << std::endl;
        ++count;
    }
}

void print_problem_data_functions(const Problem &ref)
{
    std::cout << "Data Functions:\n";
    for (std::map<std::string, function_data>::const_iterator it = ref.data_function_information.cbegin(); it != ref.data_function_information.cend(); ++it) {
        std::cout << "Function Name: " << it->first << std::endl;
        std::cout << "Function Is Dynamic: " << it->second.dynamic << std::endl;
        std::cout << "Function Is Full-Observable: " << it->second.full_observable << std::endl;
        std::cout << "Function Type: " << it->second.function_type << std::endl;
        std::cout << "Function Start Index: " << it->second.index << std::endl;
        std::cout << "Function Parameters: ";
        for (std::vector<parameter_info>::const_iterator sit = it->second.parameter_ranges.cbegin(); sit != it->second.parameter_ranges.cend(); ++sit) {
            std::cout << "(" << sit->output_value << "," << sit->type << "{";
            std::map<std::string, unsigned int>::const_iterator map_it = sit->allowed_constants.cbegin();
            while (map_it != sit->allowed_constants.cend()) {
                std::cout << map_it->first << ":" << map_it->second;
                ++map_it;
                if (map_it != sit->allowed_constants.cend()) {
                    std::cout << " ";
                }
            }
            std::cout << "}) ";
        }
        std::cout << std::endl << "-------------" << std::endl;
    }
}
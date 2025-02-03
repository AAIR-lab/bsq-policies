#include "nelder_mead.h"
#include<math.h>
#include<chrono>
#include<fstream>
#include<algorithm>


float parameter_point::distance(parameter_point& ref) {
    float output = 0.0f;

    for (std::vector<float>::iterator fit = parameter_values.begin(), sit = ref.parameter_values.begin(); fit != parameter_values.end(); ++fit, ++sit) {
        output += powf(*fit - *sit, 2.0f);
    }
    return powf(output, 0.5f);
}

void parameter_point::average(parameter_point& ref, std::vector<float>& out) {
    out.resize(parameter_values.size());
    for (int i = 0; i < parameter_values.size(); ++i) {
        out[i] = (parameter_values[i] + ref.parameter_values[i]) / 2.0f;
    }
}

NelderMead::NelderMead(BeliefState* belief_state, Policy* policy, Problem* problem, std::string prob_path, std::string solved_path,
    std::vector<std::pair<float, float>>& para_ranges, float discount_fact, unsigned int vertex_count, unsigned int horizon,
    unsigned int particle_number, unsigned int sample_number, unsigned int thread_number,float minimum_distance, std::string it_path) :vertex_count(vertex_count), horizon(horizon), particle_count(particle_number),
    points_checked(0), sample_limit(sample_number), discount_factor(discount_fact), parameter_ranges(para_ranges), thread_number(thread_number),
    problem_path(prob_path), solved_path(solved_path), generator(NULL), verbose(true), belief_state_instance(belief_state), policy_instance(policy), problem_instance(problem),
    evaluator(problem,belief_state->problem_domain,belief_state,policy,horizon,thread_number),min_distance(minimum_distance),iteration_path(it_path)
{
    generator = new std::default_random_engine;
    generator->seed(std::chrono::system_clock::now().time_since_epoch().count());
}
NelderMead::~NelderMead() {
    if (generator) {
        delete generator;
        generator = NULL;
    }
}

void NelderMead::optimize() {
    points_checked = 0;
    vertices.clear();
    parameter_point temp, extend_temp;
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    
    std::list<parameter_point> parameter_list;
    bool added = false;

    std::ofstream it_file;
    it_file.open(iteration_path, std::ios::app);

    std::chrono::high_resolution_clock::time_point end_time;

    for (int i = 0; i < 200; ++i) {
        build_point(temp);
        added = false;
        for(std::list<parameter_point>::iterator it = parameter_list.begin(); it != parameter_list.end(); ++it){
            if(it->expected_value >= temp.expected_value){
                if(it->distance(temp) < min_distance){
                    break;
                }
            }
            else{
                added = true;
                parameter_list.insert(it,temp);
            }
        }
        if(!added && parameter_list.size() < vertex_count){
            parameter_list.push_back(temp);
        }
        else if(parameter_list.size() > vertex_count){
            parameter_list.pop_back();
        }

        end_time = std::chrono::high_resolution_clock::now();
        it_file << "{";
        for (std::vector<float>::iterator it = parameter_list.front().parameter_values.begin(); it != parameter_list.front().parameter_values.end(); ++it) {
            it_file << "[" << *it << "," << *it << "]";
        }
        it_file << "}," << parameter_list.front().expected_value;
        it_file << "," << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;
        }
    for(std::list<parameter_point>::iterator it = parameter_list.begin(); it != parameter_list.end(); ++it){
        add_vertice(*it);
    }

    float best_ev_sum = sum_expected_value();
    float current_ev_sum = best_ev_sum;
    bool continue_itterating = true, temp_bool = false;
    unsigned int iter_since_improvement = 0;
    std::vector<float> stemp, extend_stemp;

    while (continue_itterating) {
        generate_point(stemp);
        build_point(stemp, temp);
        if (temp.expected_value > vertices.front().expected_value) {
            generate_extended_point(extend_stemp);
            build_point(extend_stemp, extend_temp);
            if (extend_temp.expected_value > temp.expected_value) {
                add_vertice(extend_temp);
            }
            else {
                add_vertice(temp);
            }
        }
        else if (!add_vertice(temp)) {
            generate_contract_points(extend_stemp, stemp);
            build_point(stemp, temp);
            build_point(extend_stemp, extend_temp);
            if (extend_temp.expected_value > temp.expected_value) {
                temp_bool = add_vertice(extend_temp);
            }
            else {
                temp_bool = add_vertice(temp);
            }

            if (!temp_bool) {
                contract_vertices();
            }
        }

        current_ev_sum = sum_expected_value();
        if (current_ev_sum > best_ev_sum) {
            best_ev_sum = current_ev_sum;
            iter_since_improvement = 0;
        }
        else {
            iter_since_improvement += 1;
            if (iter_since_improvement >= 5) {
                continue_itterating = false;
            }
        }

        end_time = std::chrono::high_resolution_clock::now();
        it_file << "{";
        for (std::vector<float>::iterator it = vertices.front().parameter_values.begin(); it != vertices.front().parameter_values.end(); ++it) {
            it_file << "[" << *it << "," << *it << "]";
        }
        it_file << "}," << vertices.front().expected_value;
        it_file << "," << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;

    }

    it_file.close();

    std::ofstream output_file;

    output_file.open(solved_path, std::ios::app);
    output_file << "{";
    for (std::vector<float>::iterator it = vertices.front().parameter_values.begin(); it != vertices.front().parameter_values.end(); ++it) {
        output_file << "[" << *it << "," << *it << "]";
    }
    output_file << "}," << vertices.front().expected_value;
    output_file << "," << (double)std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << std::endl;
    output_file.close();

}

bool NelderMead::add_vertice(parameter_point& ref) {
    for (std::list<parameter_point>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
        if (it->expected_value < ref.expected_value) {
            vertices.insert(it, ref);
            if (vertices.size() > vertex_count) {
                vertices.pop_back();
            }
            return true;
        }
    }
    if (vertices.size() < vertex_count) {
        vertices.emplace_back(ref);
        return true;
    }
    return false;
}

void NelderMead::build_point(parameter_point& out) {
    int i = 0;
    out.expected_value = 0.0f;
    out.parameter_values.resize(parameter_ranges.size());
    for (std::vector<std::pair<float, float>>::iterator it = parameter_ranges.begin(); it != parameter_ranges.end(); ++it, ++i) {
        out.parameter_values[i] = std::uniform_real_distribution<>(it->first, it->second)(*generator);
    }
    out.expected_value = evaluator.evaluate(out.parameter_values,sample_limit);
    // for (int i = 0; i < sample_limit; ++i) {
    //     out.expected_value += evalPolicyDFS(belief_state_instance, *policy_instance, *problem_instance, out.parameter_values, horizon, discount_factor);
    // }
    //out.expected_value /= sample_limit;
}

void NelderMead::build_point(std::vector<float>& ref, parameter_point& out) {
    out.parameter_values = ref;
    out.expected_value = 0.0f;
    out.expected_value = evaluator.evaluate(out.parameter_values,sample_limit);
    // for (int i = 0; i < sample_limit; ++i) {
    //     out.expected_value += evalPolicyDFS(belief_state_instance, *policy_instance, *problem_instance, out.parameter_values, horizon, discount_factor);
    // }
    //out.expected_value /= sample_limit;
}

void NelderMead::calculate_remaining_centroid(std::vector<float>& out) {
    std::list<parameter_point>::iterator it = vertices.begin();
    out = it->parameter_values;
    ++it;
    for (int i = 1; i < vertices.size() - 1; ++i, ++it) {
        for (std::vector<float>::iterator x = it->parameter_values.begin(), y = out.begin(); x != it->parameter_values.end(); ++x, ++y) {
            *y += *x;
        }
    }
    for (std::vector<float>::iterator sit = out.begin(); sit != out.end(); ++sit) {
        *sit /= (vertices.size() - 1);
    }
}
void NelderMead::contract_vertices() {
    std::vector<float> temp;
    parameter_point stemp;
    std::vector<parameter_point> new_vertices;
    std::list<parameter_point>::iterator it = vertices.begin();
    ++it;
    for (; it != vertices.end(); ++it) {
        vertices.front().average(*it, temp);
        build_point(temp, stemp);
        new_vertices.emplace_back(stemp);
    }
    while (vertices.size() > 1) {
        vertices.pop_front();
    }
    for (int i = 0; i < new_vertices.size(); ++i) {
        add_vertice(new_vertices[i]);
    }

}
void NelderMead::generate_contract_points(std::vector<float>& first, std::vector<float>& second) {
    std::vector<float> centroid;
    calculate_remaining_centroid(centroid);
    first.resize(parameter_ranges.size());
    second.resize(parameter_ranges.size());
    int i = 0;
    for (std::vector<float>::iterator it = vertices.back().parameter_values.begin(), sit = centroid.begin(); sit != centroid.end(); ++it, ++sit, ++i) {
        first[i] = ((3.0f * (*sit)) + *it) / 4.0f;
        second[i] = ((3.0f * (*it)) + *sit) / 4.0f;
    }
}
void NelderMead::generate_point(std::vector<float>& out) {
    std::vector<float> centroid;
    calculate_remaining_centroid(centroid);
    out.resize(parameter_ranges.size());
    int i = 0;
    for (std::vector<float>::iterator it = vertices.back().parameter_values.begin(), sit = centroid.begin(); sit != centroid.end(); ++it, ++sit, ++i) {
        out[i] = (2.0 * (*sit)) - *it;
    }
}
void NelderMead::generate_extended_point(std::vector<float>& out) {
    std::vector<float> centroid;
    calculate_remaining_centroid(centroid);
    out.resize(parameter_ranges.size());
    int i = 0;
    for (std::vector<float>::iterator it = vertices.back().parameter_values.begin(), sit = centroid.begin(); sit != centroid.end(); ++it, ++sit, ++i) {
        out[i] = (3.0 * (*sit)) - (2.0 * (*it));
    }
}
float NelderMead::sum_expected_value() {
    float output = 0.0f;
    for (std::list<parameter_point>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
        output += it->expected_value;
    }
    return output;
}

#include<chrono>
#include<iostream>
#include<algorithm>
#include "particle_swarm.h"

ParticleSwarm::ParticleSwarm(BeliefState *belief, Policy *policy, Problem *problem, float coef_rate, float cog_init, float cog_min, 
    float momentum, unsigned int patience, std::string results_path, unsigned int sample_limit, 
    float soc_init, float soc_max, unsigned int swarm_size, unsigned int horizon, unsigned int thread_number, 
    std::pair<float,float> momentum_range,std::string it_path):coef_change_rate(coef_rate),
    cognitive_coef_init(cog_init),cognitive_coef_min(cog_min),momentum(momentum),patience(patience),results_file(results_path),
    sample_limit(sample_limit),social_coef_init(soc_init),social_coef_max(soc_max),swarm_size(swarm_size),steps_since_improvement(0u),
    horizon(horizon),thread_number(thread_number),belief_state_instance(belief),policy_instance(policy),problem_instance(problem),generator(NULL),
    current_best(NULL),evaluator(problem,belief->problem_domain,belief,policy,horizon,thread_number),velocity_range(momentum_range),iteration_path(it_path){

    generator = new std::default_random_engine;
    generator->seed(std::chrono::system_clock::now().time_since_epoch().count());
    OrthotopeList par_range;
    policy->get_full_parameter_range(par_range);
    par_range.construct_ranges(parameter_ranges);
}
ParticleSwarm::~ParticleSwarm(){
    if (generator) {
        delete generator;
        generator = NULL;
    }
}
void ParticleSwarm::optimize(){
    steps_since_improvement = 0u;
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    std::ofstream it_file;
    it_file.open(iteration_path, std::ios::app);
    build_initial_particles();


    std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
    it_file << "{";
    for (std::vector<float>::iterator it = all_time_best.parameter_values.begin(); it != all_time_best.parameter_values.end(); ++it) {
        it_file << "[" << *it << "," << *it << "]";
    }
    it_file << "}," << all_time_best.expected_value;
    it_file << "," << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;

    while(steps_since_improvement < patience){
        step();

        end_time = std::chrono::high_resolution_clock::now();
        it_file << "{";
        for (std::vector<float>::iterator it = all_time_best.parameter_values.begin(); it != all_time_best.parameter_values.end(); ++it) {
            it_file << "[" << *it << "," << *it << "]";
        }
        it_file << "}," << all_time_best.expected_value;
        it_file << "," << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;
    
        if(std::chrono::duration_cast<std::chrono::hours>(end_time - start_time).count() >= 1){
            break;
        }
    }
    
    
    //end_time = std::chrono::high_resolution_clock::now();

    it_file.close();
    std::ofstream output_file;

    output_file.open(results_file, std::ios::app);
    output_file << "{";
    for (std::vector<float>::iterator it = all_time_best.parameter_values.begin(); it != all_time_best.parameter_values.end(); ++it) {
        output_file << "[" << *it << "," << *it << "]";
    }
    output_file << "}," << all_time_best.expected_value;
    output_file << "," << (double)std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << std::endl;
    output_file.close();
}

inline void ParticleSwarm::build_initial_particles(){
    particles.resize(swarm_size);
    std::uniform_real_distribution<> dist(0.0f,1.0f);
    for(int i = 0; i < swarm_size; ++i){
        particles[i].parameter_values.resize(parameter_ranges.size());
        particles[i].velocity.resize(parameter_ranges.size());
        for(int j = 0; j < parameter_ranges.size(); ++j){
            particles[i].parameter_values[j] = (dist(*generator)*(parameter_ranges[j].second - parameter_ranges[j].first)) + parameter_ranges[j].first;
            particles[i].velocity[j] = (dist(*generator)*(velocity_range.second - velocity_range.first)) + velocity_range.first;
        }
        particles[i].expected_value = evaluator.evaluate(particles[i].parameter_values,sample_limit);
        particles[i].best_expected_value_seen = particles[i].expected_value;
        particles[i].best_parameter_values_seen = particles[i].parameter_values;
        if(i == 0){
            current_best = &particles[i];
        }
        else if(particles[i].expected_value > current_best->expected_value){
            current_best = &particles[i];
        }
    }
    all_time_best = *current_best;
}

inline void ParticleSwarm::step(){
    float current_cognitive_coefficient = std::max(cognitive_coef_min, cognitive_coef_init - (coef_change_rate * steps_since_improvement));
    float current_social_coefficienct = std::min(social_coef_max, social_coef_init + (coef_change_rate * steps_since_improvement));
    std::uniform_real_distribution<> dist(0.0f,1.0f);

    for(int i = 0; i < swarm_size; ++i){
        for(int j =0; j < parameter_ranges.size(); ++j){
            //Updates velocity using the existing velocity, the best point seen, and the best point seen by this point.
            //TODO: add a repulsion force in the initial timesteps.
            particles[i].velocity[j] *= momentum;
            particles[i].velocity[j] += (all_time_best.parameter_values[j] - particles[i].parameter_values[j]) * (current_social_coefficienct * dist(*generator));
            particles[i].velocity[j] += (particles[i].best_parameter_values_seen[j] - particles[i].parameter_values[j]) * current_cognitive_coefficient * dist(*generator);
            

            // Enforces bound on velocity.
            if(particles[i].velocity[j] < velocity_range.first){
                particles[i].velocity[j] = velocity_range.first;
            }
            else if(particles[i].velocity[j] > velocity_range.second){
                particles[i].velocity[j] = velocity_range.second;
            }
            particles[i].parameter_values[j] += particles[i].velocity[j];

            // Enforces bound on parameters.
            if(particles[i].parameter_values[j] < parameter_ranges[j].first){
                particles[i].parameter_values[j] = parameter_ranges[j].first;
            }
            else if(particles[i].parameter_values[j] > parameter_ranges[j].second){
                particles[i].parameter_values[j] = parameter_ranges[j].second;
            }
        }
        particles[i].expected_value = evaluator.evaluate(particles[i].parameter_values,sample_limit);
        if(particles[i].expected_value > particles[i].best_expected_value_seen){
            particles[i].best_parameter_values_seen = particles[i].parameter_values;
            particles[i].best_expected_value_seen = particles[i].expected_value;
        }
        if(i == 0){
            current_best = &particles[i];
        }
        else if(particles[i].expected_value > current_best->expected_value){
            current_best = &particles[i];
        }
    }

    if(current_best->expected_value > all_time_best.expected_value){
        all_time_best = *current_best;
        steps_since_improvement = 0u;
    }
    else{
        steps_since_improvement += 1u;
    }
}

inline void ParticleSwarm::print()
{
    for(std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); ++it){
        for(int j = 0; j < parameter_ranges.size(); ++j){
            std::cout << it->parameter_values[j] << " ";
        }
        std::cout << std::endl;
        for(int j = 0; j < parameter_ranges.size(); ++j){
            std::cout << it->velocity[j] << " ";
        }
        std::cout << std::endl << it->expected_value;
        std::cout << "\n-------\n";
    }
    std::cout << "-------\n";
    for(int j = 0; j < parameter_ranges.size(); ++j){
        std::cout << all_time_best.parameter_values[j] << " ";
    }
    std::cout << std::endl << all_time_best.expected_value << std::endl;
    std::cout << steps_since_improvement << std::endl;
}
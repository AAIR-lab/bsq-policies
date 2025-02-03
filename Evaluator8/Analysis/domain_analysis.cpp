#include "domain_analysis.h"
#include "../Example Problems/City Exploration/city_exploration_problem.h"
#include "../Example Problems/City Exploration/city_exploration_domain.h"

void test_city_exploration_domain_observations() {
	CityExploration test;
	CityExplorationDomain test_domain;
	Action test_action;
	test_action.action_name = "Scan";
	StaticState temp_static;
	std::list<DynamicState> temp_dynamic_states;
	std::list<float> temp_probs;
	std::vector<Observation> temp_observations;
	std::vector<float> temp_obs_probs;

	char c = 'x';
	std::cout << "built domain and problem" << std::endl;
	test.generate_initial_belief(temp_static, temp_dynamic_states, temp_probs);
	std::cout << "built initial belief" << std::endl;
	for (std::list<DynamicState>::iterator it = temp_dynamic_states.begin(); it != temp_dynamic_states.end(); ++it) {
		std::cout << it->integer_params[0] << "::" << std::endl;
		test_domain.observation_distribution(*it, temp_static, test_action, temp_observations, temp_obs_probs);
		for (int i = 0; i < temp_observations.size(); ++i) {
			std::cout << temp_observations[i].integer_params[0] << ": " << temp_obs_probs[i] << std::endl;
		}
		std::cin >> c;
	}
}
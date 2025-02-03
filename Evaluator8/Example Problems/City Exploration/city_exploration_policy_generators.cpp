#include "city_exploration_policy_generators.h"

#include<vector>
#include<fstream>

void city_explore_pol1_policy_generator()
{
	unsigned int x_range = 6, y_range = 5;
	std::vector<std::pair<unsigned int, unsigned int>> bank_locations, store_locations;
	std::vector<std::pair<unsigned int, unsigned int>>::iterator it;
	std::ofstream output_file;
	bank_locations.emplace_back(5, 0);
	bank_locations.emplace_back(0, 3);
	store_locations.emplace_back(2, 1);

	output_file.open("Example Problems/City Exploration/Policies/CityExplorationPolicy1.txt");
	output_file << "//Automatically generated policy for the City Exploration domain assuming the following:\n";
	output_file << "// -Grid with dimensions of " << x_range << "-by-" << y_range << " starting a 0.\n";
	output_file << "// -Stores are located at locations: ";
	it = store_locations.begin();
	while (it != store_locations.end()) {
		output_file << "(" << it->first << "," << it->second << ")";
		++it;
		if (it != store_locations.end()) {
			output_file << ",";
		}
	}
	output_file << ".\n";
	output_file << "// -Banks are located at locations: ";
	it = bank_locations.begin();
	while (it != bank_locations.end()) {
		output_file << "(" << it->first << "," << it->second << ")";
		++it;
		if (it != bank_locations.end()) {
			output_file << ",";
		}
	}
	output_file << ".\n";
	output_file << "//High-level description of the policy:\n";
	output_file << "// -If a bank has been visited:\n";
	output_file << "//  -If the agent believes it's at any store, visit.\n";
	output_file << "//  -Else, using the current ordering of stores as the order of preference and using the action preference order of left, right, up, down, if it's safe to move towards a store do so.\n";
	output_file << "// -If a bank hasn't been visited:\n";
	output_file << "//  -If the agent believes it's at any bank, visit.\n";
	output_file << "//  -Else, using the current ordering of banks as the order of preference and using the action preference order of left, right, up, down, if it's safe to move towards a bank do so.\n";
	output_file << "// -If it can't perform any other action, scan.\n";

	output_file << "\n//Parameters:\n";
	output_file << "P1 = [0.0,1.0]\n";
	output_file << "P2 = [0.0,1.0]\n";
	output_file << "\n//Policy:\n";
	output_file << "//If a bank has been visited and the probability that I'm at a store is greater than P2, than visit.\n";
	//current_location(2, 1)] >= P2)-- > Visit()
	output_file << "if (AND visited_bank(),P[";
	if (store_locations.size() == 1) {
		output_file << "current_location(" << store_locations[0].first << "," << store_locations[0].second << ")] >= P2) --> Visit()\n\n";
	}
	else if (store_locations.size() > 1) {
		output_file << "(OR ";
		it = store_locations.begin();
		while (it != store_locations.end()) {
			output_file << "current_location(" << it->first << "," << it->second << ")";
			++it;
			if (it != store_locations.end()) {
				output_file << ",";
			}
		}
		output_file << ")] >= P2) --> Visit()\n\n";
	}

	for (int current_store = 0; current_store < store_locations.size(); ++current_store) {
		if (store_locations[current_store].first != x_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is right of store " << current_store + 1 << " and is safe to move left is greater than P1, move left.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = store_locations[current_store].first + 1; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x - 1 << "," << current_y << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Left()\n\n";
		}
		if (store_locations[current_store].first != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is left of store " << current_store + 1 << " and is safe to move right is greater than P1, move right.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < store_locations[current_store].first; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x + 1 << "," << current_y << "))";
					if (current_x != store_locations[current_store].first - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Right()\n\n";
		}
		if (store_locations[current_store].second != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is down of store " << current_store + 1 << " and is safe to move up is greater than P1, move up.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < store_locations[current_store].second; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y + 1 << "))";
					if (current_x != x_range - 1 || current_y != store_locations[current_store].second - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Up()\n\n";
		}
		if (store_locations[current_store].second != y_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is up of store " << current_store + 1 << " and is safe to move down is greater than P1, move down.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = store_locations[current_store].second + 1; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y - 1 << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Down()\n\n";
		}
	}

	output_file << "//If a bank has not been visited and the probability that I'm at a bank is greater than P2, than visit.\n";
	//current_location(2, 1)] >= P2)-- > Visit()
	output_file << "elif (AND !visited_bank(),P[";
	if (bank_locations.size() == 1) {
		output_file << "current_location(" << bank_locations[0].first << "," << bank_locations[0].second << ")] >= P2) --> Visit()\n\n";
	}
	else if (bank_locations.size() > 1) {
		output_file << "(OR ";
		it = bank_locations.begin();
		while (it != bank_locations.end()) {
			output_file << "current_location(" << it->first << "," << it->second << ")";
			++it;
			if (it != bank_locations.end()) {
				output_file << ",";
			}
		}
		output_file << ")] >= P2) --> Visit()\n\n";
	}

	for (int current_bank = 0; current_bank < bank_locations.size(); ++current_bank) {
		if (bank_locations[current_bank].first != x_range - 1) {
			output_file << "//Else, if a bank has not been visited and the probability that the agent is in a location that is right of bank " << current_bank + 1 << " and is safe to move left is greater than P1, move left.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = bank_locations[current_bank].first + 1; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x - 1 << "," << current_y << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Left()\n\n";
		}
		if (bank_locations[current_bank].first != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is left of bank " << current_bank + 1 << " and is safe to move right is greater than P1, move right.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < bank_locations[current_bank].first; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x + 1 << "," << current_y << "))";
					if (current_x != bank_locations[current_bank].first - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Right()\n\n";
		}
		if (bank_locations[current_bank].second != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is down of bank " << current_bank + 1 << " and is safe to move up is greater than P1, move up.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < bank_locations[current_bank].second; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y + 1 << "))";
					if (current_x != x_range - 1 || current_y != bank_locations[current_bank].second - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Up()\n\n";
		}
		if (bank_locations[current_bank].second != y_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is up of bank " << current_bank + 1 << " and is safe to move down is greater than P1, move down.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = bank_locations[current_bank].second + 1; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y - 1 << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Down()\n\n";
		}
	}

	output_file << "//Otherwise, just scan.\n";
	output_file << "else Scan()\n";
	output_file.close();
}

void city_explore_pol2_policy_generator()
{
	unsigned int x_range = 6, y_range = 5;
	std::vector<std::pair<unsigned int, unsigned int>> bank_locations, store_locations;
	std::vector<std::pair<unsigned int, unsigned int>>::iterator it;
	std::ofstream output_file;
	bank_locations.emplace_back(5, 0);
	bank_locations.emplace_back(0, 3);
	store_locations.emplace_back(2, 1);

	output_file.open("Example Problems/City Exploration/Policies/CityExplorationPolicy2.txt");
	output_file << "//Automatically generated policy for the City Exploration domain assuming the following:\n";
	output_file << "// -Grid with dimensions of " << x_range << "-by-" << y_range << " starting a 0.\n";
	output_file << "// -Stores are located at locations: ";
	it = store_locations.begin();
	while (it != store_locations.end()) {
		output_file << "(" << it->first << "," << it->second << ")";
		++it;
		if (it != store_locations.end()) {
			output_file << ",";
		}
	}
	output_file << ".\n";
	output_file << "// -Banks are located at locations: ";
	it = bank_locations.begin();
	while (it != bank_locations.end()) {
		output_file << "(" << it->first << "," << it->second << ")";
		++it;
		if (it != bank_locations.end()) {
			output_file << ",";
		}
	}
	output_file << ".\n";
	output_file << "//High-level description of the policy:\n";
	output_file << "// -If the belief that the agent is at any location is too low, scan.\n";
	output_file << "// -If a bank has been visited:\n";
	output_file << "//  -If the agent believes it's at any store, visit.\n";
	output_file << "//  -Else, using the current ordering of stores as the order of preference and using the action preference order of left, right, up, down, if it's safe to move towards a store do so.\n";
	output_file << "// -If a bank hasn't been visited:\n";
	output_file << "//  -If the agent believes it's at any bank, visit.\n";
	output_file << "//  -Else, using the current ordering of banks as the order of preference and using the action preference order of left, right, up, down, if it's safe to move towards a bank do so.\n";
	output_file << "// -If it can't perform any other action, scan.\n";

	output_file << "\n//Parameters:\n";
	output_file << "P1 = [0.0,1.0]\n";
	output_file << "P2 = [0.0,1.0]\n";
	output_file << "P3 = [0.0,1.0]\n";
	output_file << "\n//Policy:\n";

	output_file << "//If the probability of the agent being at all individual locations is less than P3, scan.\n";
	output_file << "if (AND ";
	bool first = true;
	for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
		for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
			if (first) {
				first = false;
			}
			else {
				output_file << ",";
			}
			output_file << "P[current_location(" << current_x << "," << current_y << ")] < P3";
		}
	}
	output_file << ") --> Scan()\n\n";

	output_file << "//If a bank has been visited and the probability that I'm at a store is greater than P2, than visit.\n";
	//current_location(2, 1)] >= P2)-- > Visit()
	output_file << "elif (AND visited_bank(),P[";
	if (store_locations.size() == 1) {
		output_file << "current_location(" << store_locations[0].first << "," << store_locations[0].second << ")] >= P2) --> Visit()\n\n";
	}
	else if (store_locations.size() > 1) {
		output_file << "(OR ";
		it = store_locations.begin();
		while (it != store_locations.end()) {
			output_file << "current_location(" << it->first << "," << it->second << ")";
			++it;
			if (it != store_locations.end()) {
				output_file << ",";
			}
		}
		output_file << ")] >= P2) --> Visit()\n\n";
	}

	for (int current_store = 0; current_store < store_locations.size(); ++current_store) {
		if (store_locations[current_store].first != x_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is right of store " << current_store + 1 << " and is safe to move left is greater than P1, move left.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = store_locations[current_store].first + 1; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x - 1 << "," << current_y << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Left()\n\n";
		}
		if (store_locations[current_store].first != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is left of store " << current_store + 1 << " and is safe to move right is greater than P1, move right.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < store_locations[current_store].first; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x + 1 << "," << current_y << "))";
					if (current_x != store_locations[current_store].first - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Right()\n\n";
		}
		if (store_locations[current_store].second != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is down of store " << current_store + 1 << " and is safe to move up is greater than P1, move up.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < store_locations[current_store].second; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y + 1 << "))";
					if (current_x != x_range - 1 || current_y != store_locations[current_store].second - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Up()\n\n";
		}
		if (store_locations[current_store].second != y_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is up of store " << current_store + 1 << " and is safe to move down is greater than P1, move down.\n";
			output_file << "elif (AND visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = store_locations[current_store].second + 1; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y - 1 << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Down()\n\n";
		}
	}

	output_file << "//If a bank has not been visited and the probability that I'm at a bank is greater than P2, than visit.\n";
	//current_location(2, 1)] >= P2)-- > Visit()
	output_file << "elif (AND !visited_bank(),P[";
	if (bank_locations.size() == 1) {
		output_file << "current_location(" << bank_locations[0].first << "," << bank_locations[0].second << ")] >= P2) --> Visit()\n\n";
	}
	else if (bank_locations.size() > 1) {
		output_file << "(OR ";
		it = bank_locations.begin();
		while (it != bank_locations.end()) {
			output_file << "current_location(" << it->first << "," << it->second << ")";
			++it;
			if (it != bank_locations.end()) {
				output_file << ",";
			}
		}
		output_file << ")] >= P2) --> Visit()\n\n";
	}

	for (int current_bank = 0; current_bank < bank_locations.size(); ++current_bank) {
		if (bank_locations[current_bank].first != x_range - 1) {
			output_file << "//Else, if a bank has not been visited and the probability that the agent is in a location that is right of bank " << current_bank + 1 << " and is safe to move left is greater than P1, move left.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = bank_locations[current_bank].first + 1; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x - 1 << "," << current_y << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Left()\n\n";
		}
		if (bank_locations[current_bank].first != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is left of bank " << current_bank + 1 << " and is safe to move right is greater than P1, move right.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < bank_locations[current_bank].first; ++current_x) {
				for (unsigned int current_y = 0; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x + 1 << "," << current_y << "))";
					if (current_x != bank_locations[current_bank].first - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Right()\n\n";
		}
		if (bank_locations[current_bank].second != 0) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is down of bank " << current_bank + 1 << " and is safe to move up is greater than P1, move up.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = 0; current_y < bank_locations[current_bank].second; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y + 1 << "))";
					if (current_x != x_range - 1 || current_y != bank_locations[current_bank].second - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Up()\n\n";
		}
		if (bank_locations[current_bank].second != y_range - 1) {
			output_file << "//Else, if a bank has been visited and the probability that the agent is in a location that is up of bank " << current_bank + 1 << " and is safe to move down is greater than P1, move down.\n";
			output_file << "elif (AND !visited_bank(),P[(OR ";
			for (unsigned int current_x = 0; current_x < x_range; ++current_x) {
				for (unsigned int current_y = bank_locations[current_bank].second + 1; current_y < y_range; ++current_y) {
					output_file << "(AND current_location(" << current_x << "," << current_y << "),is_safe(" << current_x << "," << current_y - 1 << "))";
					if (current_x != x_range - 1 || current_y != y_range - 1) {
						output_file << ",";
					}
				}
			}
			output_file << ")] >= P1) --> Down()\n\n";
		}
	}

	output_file << "//Otherwise, just scan.\n";
	output_file << "else Scan()\n";
	output_file.close();
}
#include "lane_merger_policy_generator.h"
#include<fstream>
#include<iostream>

void lane_merger_policy1_generator(unsigned int merger_length, unsigned int agent_start_location, unsigned int other_car_start_location, unsigned int agent_start_speed, 
	unsigned int speed_limit, std::pair<unsigned int, unsigned int> &other_car_speed_range)
{
	std::ofstream output_file;
	output_file.open("Example Problems/Lane Merger/Policies/lane_merger_policy1.txt");

	output_file << "//Parameters:\n";
	output_file << "P1 = [0.0,1.0]\n";
	output_file << "//Policy:\n";

	bool not_first_element = false;

	for (int current_speed = 0u; current_speed <= speed_limit; ++current_speed) {
		for (int current_location = agent_start_location; current_location <= merger_length; ++current_location) {
			if (current_speed == 0u && current_location == agent_start_location) {
				output_file << "if ";
			}
			else {
				output_file << "elif ";
			}
			not_first_element = false;
			output_file << "(AND current_location(" << current_location << "),current_speed(" << current_speed << "),P[(OR ";
			for (int other_location = other_car_start_location; other_location <= merger_length; ++other_location) {
				//Means that, regardless of the speed, the other car cannot catch-up or slow-down enough to prevent a merger
				if ((current_location) > (other_location + (int)other_car_speed_range.second + 1) || (current_location + current_speed) + 1 < (other_location)) {
					if (not_first_element) {
						output_file << ",";
					}
					else {
						not_first_element = true;
					}
					output_file << "other_car_location(" << other_location << ")";
				}
				else {
					//Otherwise only include speeds with the car location that satisfy this property.
					for (int other_speed = other_car_speed_range.first; other_speed <= other_car_speed_range.second; ++other_speed) {
						if ((((current_location + current_speed) - (other_location)) < -1) || (((current_location)-(other_location + other_speed)) > 1)) {
							if (not_first_element) {
								output_file << ",";
							}
							else {
								not_first_element = true;
							}
							output_file << "(AND other_car_location(" << other_location << "),other_car_speed(" << other_speed << "))";
						}
					}
				}
			}
			output_file << ")] >= P1) --> merge()\n";
		}
	}

	output_file << "elif (OR ";
	for (int current_speed = 1u; current_speed < speed_limit; ++current_speed) {
		output_file << "current_speed(" << current_speed << "),";
	}
	output_file << "current_speed(" << speed_limit << ")) --> decrease_speed()\n";
	output_file << "else keep_speed()\n";

	output_file.close();

}

void lane_merger_policy2_generator(unsigned int merger_length, unsigned int agent_start_location, unsigned int other_car_start_location, unsigned int agent_start_speed, 
	unsigned int speed_limit, std::pair<unsigned int, unsigned int> &other_car_speed_range, int distance)
{
	std::ofstream output_file;
	output_file.open("Lane Merger/Policies/lane_merger_policy2.txt");

	output_file << "//Parameters:\n";
	for(int i = 1; i <= distance; ++i){
		output_file << "P" << i <<" = [0.0,1.0] \n";
	}
	output_file << "//Policy:\n";

	bool not_first_element = false;

	for(int i = distance; i >= 1; --i){
		for (int current_speed = 0u; current_speed <= speed_limit; ++current_speed) {
			for (int current_location = agent_start_location; current_location <= merger_length; ++current_location) {
				if (current_speed == 0u && current_location == agent_start_location && i == distance) {
					output_file << "if ";
				}
				else {
					output_file << "elif ";
				}
				not_first_element = false;
				output_file << "(AND current_location(" << current_location << "),current_speed(" << current_speed << "),P[(OR ";
				for (int other_location = other_car_start_location; other_location <= merger_length; ++other_location) {
					//Means that, regardless of the speed, the other car cannot catch-up or slow-down enough to prevent a merger
					if ((current_location) > (other_location + (int)other_car_speed_range.second + i) || (current_location + current_speed) + i < (other_location)) {
						if (not_first_element) {
							output_file << ",";
						}
						else {
							not_first_element = true;
						}
						output_file << "other_car_location(" << other_location << ")";
					}
					else {
						//Otherwise only include speeds with the car location that satisfy this property.
						for (int other_speed = other_car_speed_range.first; other_speed <= other_car_speed_range.second; ++other_speed) {
							if ((((current_location + current_speed) - (other_location)) < -i) || (((current_location)-(other_location + other_speed)) > i)) {
								if (not_first_element) {
									output_file << ",";
								}
								else {
									not_first_element = true;
								}
								output_file << "(AND other_car_location(" << other_location << "),other_car_speed(" << other_speed << "))";
							}
						}
					}
				}
				output_file << ")] >= P" << i << ") --> merge()\n";
			}
		}
	}

	output_file << "elif (OR ";
	for (int current_speed = 1u; current_speed < speed_limit; ++current_speed) {
		output_file << "current_speed(" << current_speed << "),";
	}
	output_file << "current_speed(" << speed_limit << ")) --> decrease_speed()\n";
	output_file << "else keep_speed()\n";

	output_file.close();

}

void lane_merger_policy3_generator(unsigned int merger_length, unsigned int agent_start_location, unsigned int other_car_start_location, unsigned int agent_start_speed, 
	unsigned int speed_limit, std::pair<unsigned int, unsigned int> &other_car_speed_range)
{
	std::ofstream output_file;
	output_file.open("Lane Merger/Policies/lane_merger_policy3.txt");

	output_file << "//Parameters:\n";
	output_file << "P1 = [0.0,1.0] \n";
	output_file << "P2 = [0.0,1.0] \n";
	output_file << "//Policy:\n";

	bool not_first_element = false;

	for (int current_speed = 0u; current_speed <= speed_limit; ++current_speed) {
		for (int current_location = agent_start_location; current_location <= merger_length; ++current_location) {
			if (current_speed == 0u && current_location == agent_start_location) {
				output_file << "if ";
			}
			else {
				output_file << "elif ";
			}
			not_first_element = false;
			output_file << "(AND current_location(" << current_location << "),current_speed(" << current_speed << "),P[(OR ";
			for (int other_location = other_car_start_location; other_location <= merger_length; ++other_location) {
				//Means that, regardless of the speed, the other car cannot catch-up or slow-down enough to prevent a merger
				if ((current_location) > (other_location + (int)other_car_speed_range.second + 2) || (current_location + current_speed) + 2 < (other_location)) {
					if (not_first_element) {
						output_file << ",";
					}
					else {
						not_first_element = true;
					}
					output_file << "other_car_location(" << other_location << ")";
				}
				else {
					//Otherwise only include speeds with the car location that satisfy this property.
					for (int other_speed = other_car_speed_range.first; other_speed <= other_car_speed_range.second; ++other_speed) {
						if ((((current_location + current_speed) - (other_location)) < -2) || (((current_location)-(other_location + other_speed)) > 2)) {
							if (not_first_element) {
								output_file << ",";
							}
							else {
								not_first_element = true;
							}
							output_file << "(AND other_car_location(" << other_location << "),other_car_speed(" << other_speed << "))";
						}
					}
				}
			}
			output_file << ")] >= P1) --> merge()\n";
		}
	}

	output_file << "elif (AND (OR ";
	for (int current_speed = 1u; current_speed < speed_limit; ++current_speed) {
		output_file << "current_speed(" << current_speed << "),";
	}
	output_file << "current_speed(" << speed_limit << ")),P[(OR ";
	bool not_first_or = false;

	for (int current_location = agent_start_location; current_location <= merger_length; ++current_location) {
		for (int other_location = other_car_start_location; other_location <= merger_length; ++other_location) {
			if(abs(current_location - other_location) <= 1){
				if(not_first_or){
					output_file << ",";
				}
				else{
					not_first_or = true;
				}
				output_file << "(AND current_location(" << current_location << "),other_car_location(" << other_location << "))";
			}
		}
	}

	output_file << ")] >= P2) --> decrease_speed()\n";
	output_file << "else keep_speed()\n";

	output_file.close();

}
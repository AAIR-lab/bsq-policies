# Belief-State Query Policies for Planning With Preferences Under Partial Observabililty

Thank you for taking a look at our code!

## Requirements

The code used for running the experiments performed in our paper was implemented in C++ using Microsoft Visual Studios compiled using C++20 Standard (/std::c++20). The solution file [change name].sln is provided for running the code.

## Running the Main Experiments

Compiling the code will construct an executable for running the main results produced in this paper with the same hyperparameters used (see the run_all_experiments() function in main.cpp). However, note that this code will probably require running for weeks to complete if it finishes (please refer to the known issues section below). 

## Running the Solver

We suggest using the run_solver() function defined in main.cpp or using it as a reference. This function takes the following parameters:
1. selected problem: For the defined problems, these are LM (Lane Merger), GRS (Graph Rock Sample), SR (Spaceship Repair), and SV (Store Visit).
2. selected solver: For the defined solvers, these are softmax (Boltzmann Exploration), certainty_max (Confidence Maximization),epsilon_greedy (Epsilon Greedy),global_thompson (Global Thompson Sampling), and local_thompson (Local Thompson Sampling)
3. solution file path: The path to save the solutions.
* Each line of this file is the solved partition: [the interval of the partition],[the expected cost of the partition],[solve time]
4. interval file path: The path to save all the partitions constructed by PRS.
* Each line is a different partition: [the interval of the partition],[the expected cost of the partition],[number of times the partition has been sampled]
5. time samples path: The path to save the recorded hypothesized optimal partition values while solving.
* Each line of this file is a recorded partition: [the interval of the partition],[the expected cost of the partition],[time when this partition was taken]
6. run count: Number of times to run the solver.
7. horizon: Horizon to solve the problem and BSQ preference for.
8. minimum time: The minimum time (in seconds) to run the solver for.
9. maximum time: The maximum time (in seconds) to run the solver for. This extra time is only used if the hypothesized optimal partition has less than 3 samples.
10. thread number: Number of worker threads to use while solving.
11. time sample count: The number of times to record the hypothesized optimal partition while solving.

## Running the Evaluator

We suggest using the policy_evaluator() function defined in main.cpp or using it as a reference. This function takes the following parameters:
1. selected problem: For the defined problems, these are LM (Lane Merger), GRS (Graph Rock Sample), SR (Spaceship Repair), and SV (Store Visit).
2. selected solver: For the defined solvers, these are softmax (Boltzmann Exploration), certainty_max (Confidence Maximization),epsilon_greedy (Epsilon Greedy),global_thompson (Global Thompson Sampling), and local_thompson (Local Thompson Sampling)
3. solution file path: The path to the solutions being evaluated.
4. results file_path: the path to save the evaluation results.
* Each line of this file is the evaluation for one of the solutions in the solution file: [expected cost], [expected cost standard deviation], [goal achievement rate], [goal achievement standard deviation], [percent "preference satisfaction"], [standard deviation "preference satisfaction"]
* "Preference satisfaction" is actually the percent probability of avoiding unsafe scenarios (e.g., not sampling an unsafe rock in Graph Rock Sample). We do not use these results!
5. horizon: Horizon to solve the problem and BSQ preference for.
6. evaluation count: number of runs to evaluate each solution on.
7. random seed: random seed to use for generating the initial states for each run.
8. thread number: number of worker threads to use for evaluation.

## Running RCompliant

We suggest using the rcompliant_policy_evaluator() function defined in main.cpp or using it as a reference. This function take the following parameters:
1. selected problem: For the defined problems, these are LM (Lane Merger), GRS (Graph Rock Sample), SR (Spaceship Repair), and SV (Store Visit).
2. results file_path: the path to save the evaluation results.
* Each line of this file is the evaluation for one of the solutions in the solution file: [expected cost], [expected cost standard deviation], [goal achievement rate], [goal achievement standard deviation], [percent "preference satisfaction"], [standard deviation "preference satisfaction"]
* "Preference satisfaction" is actually the percent probability of avoiding unsafe scenarios (e.g., not sampling an unsafe rock in Graph Rock Sample). We do not use these results!
3. horizon: Horizon to solve the problem and BSQ preference for.
4. evaluation count: number of runs to evaluate each solution on.
5. random seed: random seed to use for generating the initial states for each run.
6. thread number: number of worker threads to use for evaluation.
7. policy count: number of policies to run RCompliant for.

## Construct the Spaceship Repair Heatmap

The test_spaceship_heatmap() function in main.cpp can be used to reproduce the results used to make the Spaceship Repair heatmap from the paper. This program will produce a file at the path ../Results/heatmap_spaceship_repair.txt that contains the parameter values and the calculated expected cost.

## Known Issues

This version of the code will error out randomly. Tracing this error, this occurs when the Partition Refinement Search algorithm (called Interval Solver in the code) attempts to remove a new partition from an empty list (see line 407 of interval_solver.cpp as an example). This bug has to do with somehow the list being emptied before popping even with a lock. However, note this bug does not affect the results.

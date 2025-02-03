# Belief-State Query Policies for User-Aligned POMDPs

Code repository for the ideas and experiments from the paper:

**Belief-State Query Policies for User-Aligned POMDPs.**

Daniel Bramblett and Siddharth Srivastava.

*Thirty-Eighth Annual Conference on Neural Information Processing Systems, 2024.*

## Insruction for running the code

This code is written in C++ using **Microsoft Visual Studios** compiled using **C++20 Standard** (`/std::c++20`). The solution file `Evaluator8.sln` is provided for running the code.

## Running all the experiments

Compiling the provided code will construct an executable for running the main results produced in this paper with the same hyperparameters used.  Refer to the `run_all_experiments()` function in `main.cpp`.

## Running Partition Refinement Search

For running **Partition Refinement Search**, refer to the `run_solver()` function in `main.cpp` for the required parameter and function calls.

## Creating an evaluation domain/problem

Refer to the `Example Problem` directory for examples on how to implement new domains and problems. 

Belief states consist of two components: constant parameters and dynamic parameters that together define the state.
- **Problem instances** define the initial belief state and provide function mappings for querying the state space and executing actions.
- **Domain instances** implement functions such as the next-state distribution and observation distribution from the current state and executed action.

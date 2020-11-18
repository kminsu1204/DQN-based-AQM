# DQN-based-AQM
This repository provides the code used in the following paper:

Minsu Kim, "Deep Reinforcement Learning based Active Queue Management for IoT Networks," Master's Thesis at Ryerson Univesrity

The paper is available at:
https://digital.library.ryerson.ca/islandora/object/RULA%3A8551

## Get Started
This work is based on the below open sources:
1. ns-3: https://www.nsnam.org/
2. tiny-dnn: https://github.com/tiny-dnn/tiny-dnn

This repository includes 3 files below:
### dqn-queue-disc.h
This header file defines the queue discipline that is used and deployed under traffic-control. Please note that the proper path to tiny_dnn.h should be defined.

### dqn-queue-disc.cc
This file implements all methods of DqnQueueDisc class.

### iot_topology.cc
This file implements the simulated IoT networks for testing. It takes multiple arguments through a command line to execute the simulation. Please check the arguments defined with "CommandLine cmd" variable.

## Running the program
Regarding how to run/execute the program, please refer to the tutorial documentation provided by ns-3.

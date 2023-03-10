#ifndef DEFINES_H
#define DEFINES_H

// #define SIMULATION_WIDTH 238
// #define SIMULATION_HEIGHT 130
#define SIMULATION_WIDTH 64
#define SIMULATION_HEIGHT 64

#define CELL_WIDTH 12
#define CELL_HEIGHT 12

// N times from 100, mutation will happen
#define MUTATION_PERCENT 25

#define GENOME_LENGTH 32

#define START_ENERGY 5.f

#define REPRODUCTION_REQUIRED_ENERGY 16

// maximum cell age
#define CELL_MAX_AGE 2048

#define PHOTOSYNTHESIS_ENERGY 1

// how many energy is required to make an attack
#define ATTACK_REQUIRED_ENERGY MOVEMENT_COST * 2

// fraction of attacked cell's energy attacker will get
#define ATTACK_ENERGY 0.5

// how much energy is required for cell to move
#define MOVEMENT_COST 1
#define TURN_COST MOVEMENT_COST * 0.5f
#define SWAP_COST MOVEMENT_COST * 4

#define NOOP_COST 0.1f

#endif

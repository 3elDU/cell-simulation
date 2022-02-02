#ifndef DEFINES_H
#define DEFINES_H

#define SIMULATION_WIDTH 380
#define SIMULATION_HEIGHT 200

#define CELL_WIDTH 5
#define CELL_HEIGHT 5

// N times from 100, mutation will happen
#define MUTATION_PERCENT 15

#define GENOME_LENGTH 64

#define START_ENERGY 12.f

#define REPRODUCTION_REQUIRED_ENERGY 100

// maximum cell age
#define CELL_MAX_AGE 1200

#define PHOTOSYNTHESIS_ENERGY 5

// how many energy is required to make an attack
#define ATTACK_REQUIRED_ENERGY 5

// how many percents of attacked cell's energy attacker will get
#define ATTACK_ENERGY 80

// how much energy is required for cell to move
#define MOVEMENT_COST 2

#define NOOP_COST 0.2f

#endif
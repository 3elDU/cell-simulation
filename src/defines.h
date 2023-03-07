#ifndef DEFINES_H
#define DEFINES_H

#define SIMULATION_WIDTH 238
#define SIMULATION_HEIGHT 130

#define CELL_WIDTH 8
#define CELL_HEIGHT 8

// N times from 100, mutation will happen
#define MUTATION_PERCENT 1

#define GENOME_LENGTH 256

#define START_ENERGY 10.f

#define REPRODUCTION_REQUIRED_ENERGY 80

// maximum cell age
#define CELL_MAX_AGE 1200

#define PHOTOSYNTHESIS_ENERGY 5

// how many energy is required to make an attack
#define ATTACK_REQUIRED_ENERGY 1

// how many percents of attacked cell's energy attacker will get
#define ATTACK_ENERGY 50

// how much energy is required for cell to move
#define MOVEMENT_COST 1

#define NOOP_COST 0.2f

#endif

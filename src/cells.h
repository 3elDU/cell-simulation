#ifndef CELLS_H
#define CELLS_H

#include <stdbool.h>
#include <stdint.h>
#include "defines.h"

#define CELLS_ConvertCoords(state, x, y) y * state->width + x

enum gen_instruction
{
	// do nothing
	NOOP,

	// movement
	TURN_LEFT,
	TURN_RIGHT,
	MOVE_FORWARDS,

	PHOTOSYNTHESIS,

	// gives e energy to cell in front
	GIVE_ENERGY,

	// attacks cell in front
	// if cell class is hunter cell, it kills the cell immediately
	ATTACK_CELL,

	// eats dead cell in front
	RECYCLE_DEAD_CELL,

	// jump to instruction b1 if cell has more than e energy
	// otherwise, jump to instruction b2
	CHECK_ENERGY,

	// if left -> b1
	// if right ->
	CHECK_ROTATION,

	// jump to instruction BX, if there's alive cell in front
	JMP_IF_FACING_ALIVE_CELL,

	// jump to instruction BX, if there's dead cell in front
	JMP_IF_FACING_DEAD_CELL,

	// jump to instruction BX, if facing empty space
	JMP_IF_FACING_VOID,

	// if cell's genome is at least CX% similar to other's cell genome, jumping to instruction bx
	JMP_IF_FACING_RELATIVE,

	// works, if cell has more than REPRODUCTION_REQUIRED_ENERGY energy
	MAKE_CHILD
};

enum direction
{
	LEFT,
	RIGHT,
	UP,
	DOWN,
};

enum cell_food_source
{
	FOOD_SOURCE_PHOTOSYNTHESIS,
	FOOD_SOURCE_MEAT,
	FOOD_SOURCE_DEAD_CELLS,
	FOOD_SOURCE_UNKNOWN,
};

struct instruction
{
	enum gen_instruction command;

	// those are "arguments" for main instruction
	// opt is a boolean. in some instructions, alters it
	// e stores energy from 0 to REPRODUCTION_REQUIRED_ENERGY, and is used in JMP_IF_LESS/JMP_IF_GREATER
	// b1 and b2 are branches for JMP instructions
	// b3 and b4 are branches for CHECK_ROTATION
	// if the JMP condition is true, jumps to instruction b1
	// otherwise, to instruction b2
	bool opt;
	uint8_t e;	// energy condition
	uint8_t b1; // branch 1
	uint8_t b2; // branch 2
	uint8_t b3; // branch 3
	uint8_t b4; // branch 4
};

// returns randomly generated instruction
struct instruction cells_generate_instruction();

struct cell
{
	struct instruction genome[GENOME_LENGTH];
	unsigned currentInstruction;

	enum direction direction;

	float energy;
	bool alive;
	bool empty;
	unsigned age;

	unsigned x;
	unsigned y;

	float r;
	float g;
	float b;

	unsigned photosynthesisCount;
	unsigned attackCount;
	unsigned eatingDeadCount;
};

// returns randomly generated cell
struct cell cells_generate_cell(const unsigned x, const unsigned y);

// returns empty cell
struct cell cells_generate_empty_cell(const unsigned x, const unsigned y);

// independent cell simulation
struct cells_state
{
	unsigned width;
	unsigned height;

	struct cell *cells;
	unsigned *updateOrder;
};

// updates given cell
void cells_update_cell(struct cells_state *state, struct cell cell);

// Returns NULL if fails
struct cells_state *cells_init(const unsigned width, const unsigned height);

// Un-allocates memory and sets state to NULL
void cells_quit(struct cells_state *state);

// Updates the simulation
void cells_update_state(struct cells_state *state);

/*
	Returns cell at given position.
	If position is invalid, returns NULL
*/
struct cell *cells_get_cell(const struct cells_state *state, const unsigned x, const unsigned y);

/*
	Sets cell at given position.
	If position or CELLS_Cell pointer is invalid, doesn't do anything
*/
void cells_set_cell(struct cells_state *state, const unsigned x, const unsigned y, const struct cell cell);

// returns number of alive cells in given state
unsigned cells_count_alive_cells(const struct cells_state *state);

enum cell_food_source cells_get_cell_food_source(struct cell cell);

#endif
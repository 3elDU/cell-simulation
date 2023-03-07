#ifndef CELLS_H
#define CELLS_H

#include <stdbool.h>
#include <stdint.h>
#include "defines.h"

#define CELLS_ConvertCoords(state, x, y) y * state->width + x

enum CELLS_GenCommand
{

	// do nothing
	NOOP,

	// movement
	TURN_LEFT,
	TURN_RIGHT,
	MOVE_FORWARDS,

	// swaps with cell in front
	SWAP_PLACES,

	// generates 5 energy
	PHOTOSYNTHESIS,

	// attacks cell in front
	ATTACK_CELL,

	// kills the cell in front, but gets 2x less energy than by attacking
	// available only for "hunter" cells
	KILL_CELL,

	// jump to instruction BX
	JMP,

	// jump to instruction BX, if energy is less than AX
	JMP_IF_LESS,

	// jump to instruction BX, if energy is greater than AX
	JMP_IF_GREATER,

	// jump to instruction BX, if there's alive cell in front
	JMP_IF_FACING_ALIVE_CELL,

	// jump to instruction BX, if there's dead cell in front
	JMP_IF_FACING_DEAD_CELL,

	// jump to instruction BX, if facing empty space
	JMP_IF_FACING_VOID,

	// if cell's genome is at least CX% similar to other's cell genome, jumping to instruction bx
	JMP_IF_FACING_RELATIVE,

	// leaves dead cell on the ground.
	// dead cell contains AX percents of cell's energy
	SUICIDE,

	// works, if cell has more than REPRODUCTION_REQUIRED_ENERGY energy
	// cell gives the child CX percents of it's own energy
	MAKE_CHILD
};

enum CELLS_Direction
{
	LEFT,
	UP,
	RIGHT,
	DOWN
};

struct CELLS_Instruction
{
	enum CELLS_GenCommand command;

	// those are "arguments" for main instruction
	unsigned ax; // value from 0 to 128
	unsigned bx; // value from 0 to 63
	float cx;	 // floating point value from 0.0 to 100.0
};

// returns randomly generated instruction
struct CELLS_Instruction CELLS_GenerateInstruction();

struct CELLS_Cell
{
	struct CELLS_Instruction genome[GENOME_LENGTH];
	unsigned currentInstruction;

	enum CELLS_Direction direction;

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
struct CELLS_Cell CELLS_GenerateCell(const unsigned x, const unsigned y);

// returns empty cell
struct CELLS_Cell CELLS_GenerateEmptyCell(const unsigned x, const unsigned y);

// independent cell simulation
struct CELLS_State
{
	unsigned width;
	unsigned height;

	struct CELLS_Cell *cells;
	unsigned *updateOrder;
};

// updates given cell
void CELLS_UpdateCell(struct CELLS_State *state, struct CELLS_Cell cell);

// Returns NULL if fails
struct CELLS_State *CELLS_Init(const unsigned width, const unsigned height);

// Un-allocates memory and sets state to NULL
void CELLS_Quit(struct CELLS_State *state);

// Updates the simulation
void CELLS_Update(struct CELLS_State *state);

/*
	Returns cell at given position.
	If position is invalid, returns NULL
*/
struct CELLS_Cell *CELLS_GetCell(const struct CELLS_State *state, const unsigned x, const unsigned y);

/*
	Sets cell at given position.
	If position or CELLS_Cell pointer is invalid, doesn't do anything
*/
void CELLS_SetCell(struct CELLS_State *state, const unsigned x, const unsigned y, const struct CELLS_Cell cell);

// returns number of alive cells in given state
unsigned CELLS_CountAliveCells(const struct CELLS_State *state);

#endif
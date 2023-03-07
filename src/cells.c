#include "cells.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct CELLS_Instruction CELLS_GenerateInstruction()
{
	struct CELLS_Instruction instruction;

	instruction.command = CELLS_Random(0, MAKE_CHILD);

	instruction.ax = CELLS_Random(0, 128);
	instruction.bx = CELLS_Random(0, 63);
	instruction.cx = CELLS_Random(0, 100000) / 1000.f;

	return instruction;
}

struct CELLS_Cell CELLS_GenerateCell(const unsigned x, const unsigned y)
{
	struct CELLS_Cell cell;

	// generating genome
	for (int i = 0; i < GENOME_LENGTH; i++)
	{
		cell.genome[i] = CELLS_GenerateInstruction();
	}

	cell.currentInstruction = 0;
	cell.direction = CELLS_Random(0, 3);
	cell.energy = START_ENERGY;
	cell.alive = true;
	cell.empty = false;
	cell.age = 0;

	cell.x = x, cell.y = y;

	cell.r = CELLS_Random(0, 255);
	cell.g = CELLS_Random(0, 255);
	cell.b = CELLS_Random(0, 255);

	return cell;
}

struct CELLS_Cell CELLS_GenerateEmptyCell(const unsigned x, const unsigned y)
{
	struct CELLS_Cell cell;

	cell.alive = false;
	cell.empty = true;
	cell.age = 0;

	cell.x = x, cell.y = y;

	return cell;
}

void CELLS_UpdateCell(struct CELLS_State *state, struct CELLS_Cell cell)
{
	if (state == NULL)
		return;

	bool suicide = false;

	if (!cell.alive)
		return;

	unsigned nextInstruction = cell.currentInstruction + 1;
	float consumedEnergy = NOOP_COST;

	int facingX = cell.x, facingY = cell.y;

	if (cell.direction == LEFT)
		facingX--;
	else if (cell.direction == RIGHT)
		facingX++;
	else if (cell.direction == UP)
		facingY--;
	else if (cell.direction == DOWN)
		facingY++;

	if (facingX == -1)
		facingX = state->width - 1;
	else if (facingX == state->width)
		facingX = 0;

	if (facingY == -1)
		facingY = state->height - 1;
	else if (facingY == state->height)
		facingY = 0;

	struct CELLS_Cell *frontCell;

	switch (cell.genome[cell.currentInstruction].command)
	{
	case NOOP:
		break;

	case TURN_LEFT:
		if (cell.direction - 1 == -1)
			cell.direction = DOWN;
		else
			cell.direction--;

		consumedEnergy += MOVEMENT_COST * 0.5f;
		break;

	case TURN_RIGHT:
		if (cell.direction + 1 == 4)
			cell.direction = LEFT;
		else
			cell.direction++;

		consumedEnergy += MOVEMENT_COST * 0.5f;
		break;

	case MOVE_FORWARDS:
		// checking if space, where cell wants to move is empty
		// if it is not, cell won't move
		if (CELLS_GetCell(state, facingX, facingY)->empty)
		{

			CELLS_SetCell(state, cell.x, cell.y, CELLS_GenerateEmptyCell(cell.x, cell.y));
			cell.x = facingX, cell.y = facingY;
		}
		else if (!CELLS_GetCell(state, facingX, facingY)->alive)
		{
			// if space, where cell wants to move is occupied by dead cell, eating it
			cell.energy += CELLS_GetCell(state, facingX, facingY)->energy;

			CELLS_SetCell(state, cell.x, cell.y, CELLS_GenerateEmptyCell(cell.x, cell.y));
			cell.x = facingX, cell.y = facingY;

			cell.eatingDeadCount++;
		}

		consumedEnergy += MOVEMENT_COST;

		break;

	case SWAP_PLACES:
		frontCell = CELLS_GetCell(state, facingX, facingY);

		frontCell->x = cell.x;
		frontCell->y = cell.y;

		CELLS_SetCell(state, cell.x, cell.y, *frontCell);

		cell.x = facingX, cell.y = facingY;

		consumedEnergy += MOVEMENT_COST * 2;

		break;

	case PHOTOSYNTHESIS:
		if (cell.attackCount > cell.photosynthesisCount)
			consumedEnergy -= PHOTOSYNTHESIS_ENERGY / 2;
		else
			consumedEnergy -= PHOTOSYNTHESIS_ENERGY;

		cell.photosynthesisCount++;

		break;

	case ATTACK_CELL:
		// requires at least 10 energy
		if (cell.energy < ATTACK_REQUIRED_ENERGY)
		{
			break;
		}

		cell.energy -= ATTACK_REQUIRED_ENERGY;

		frontCell = CELLS_GetCell(state, facingX, facingY);

		if (frontCell->alive)
		{

			unsigned takenEnergy;

			if (cell.photosynthesisCount > cell.attackCount)
				takenEnergy = frontCell->energy * (ATTACK_ENERGY * 0.01f) * 0.25f;
			else
				takenEnergy = frontCell->energy * (ATTACK_ENERGY * 0.01f) * 0.5f;

			frontCell->energy *= 0.25f;

			cell.energy += takenEnergy;
		}

		cell.attackCount++;

		break;

	case KILL_CELL:
		// requires at least 10 energy
		if (cell.energy < ATTACK_REQUIRED_ENERGY || cell.attackCount < cell.photosynthesisCount)
		{
			break;
		}

		cell.energy -= ATTACK_REQUIRED_ENERGY;

		frontCell = CELLS_GetCell(state, facingX, facingY);

		if (frontCell->alive)
		{

			unsigned takenEnergy;

			takenEnergy = frontCell->energy * (ATTACK_ENERGY * 0.01f) * 0.8f;

			frontCell->energy = 0;
			frontCell->alive = false;

			cell.energy += takenEnergy;
		}

		cell.attackCount++;

		break;

	case JMP:
		nextInstruction = cell.genome[cell.currentInstruction].bx;
		break;

	case JMP_IF_LESS:
		if (cell.energy < cell.genome[cell.currentInstruction].ax)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_GREATER:
		if (cell.energy > cell.genome[cell.currentInstruction].ax)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_ALIVE_CELL:
		if (CELLS_GetCell(state, facingX, facingY)->alive)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_VOID:
		if (CELLS_GetCell(state, facingX, facingY)->empty)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_DEAD_CELL:
		if (!CELLS_GetCell(state, facingX, facingY)->alive && !CELLS_GetCell(state, facingX, facingY)->empty)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_RELATIVE:
	{
		// checking genome similarity
		unsigned similarGenes = 0;

		struct CELLS_Instruction *ours = cell.genome;
		struct CELLS_Instruction *theirs = CELLS_GetCell(state, facingX, facingY)->genome;

		for (int i = 0; i < GENOME_LENGTH; i++)
		{

			if (ours[i].command == theirs[i].command &&
				ours[i].ax == theirs[i].ax &&
				ours[i].bx == theirs[i].bx &&
				ours[i].cx == theirs[i].cx)
				similarGenes++;
		}

		float genomeSimilarity = similarGenes * (100.f / GENOME_LENGTH);

		if (genomeSimilarity >= cell.genome[cell.currentInstruction].cx)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		// printf("%u similar genes, %f similarity, required %f\n", similarGenes, genomeSimilarity, cell.genome[cell.currentInstruction].cx);

		break;
	}

	case SUICIDE:
		suicide = true;
		break;

	case MAKE_CHILD:
		// skipping instruction, if cell doesn't have enough energy
		if (cell.energy < REPRODUCTION_REQUIRED_ENERGY)
			break;

		// checking for available space
		if (!CELLS_GetCell(state, facingX, facingY)->alive)
		{

			struct CELLS_Cell child = {};

			child.alive = true;
			child.empty = false;

			child.x = facingX;
			child.y = facingY;

			child.age = 1;

			child.direction = CELLS_Random(0, 3);

			child.r = cell.r, child.g = cell.g, child.b = cell.b;

			// giving CX percents of cell's own energy to the child
			child.energy = cell.energy * (cell.genome[cell.currentInstruction].cx / 100.f);
			cell.energy *= 1 - cell.genome[cell.currentInstruction].cx / 100.f;

			// printf("Child energy %f; New parent energy %f\n", child.energy, cell.energy);

			// copying parent genome to the child
			for (int i = 0; i < GENOME_LENGTH; i++)
			{

				// mutation can happen
				if (CELLS_Random(1, 100) <= MUTATION_PERCENT)
				{
					// mutating
					child.genome[i] = CELLS_GenerateInstruction();

					// changing child's color a bit
					unsigned colorToChange = CELLS_Random(0, 2);

					if (colorToChange == 0)
						child.r += CELLS_Random(-16, 16);
					if (colorToChange == 1)
						child.g += CELLS_Random(-16, 16);
					if (colorToChange == 2)
						child.b += CELLS_Random(-16, 16);
				}
				else
					child.genome[i] = cell.genome[i];
			}

			child.r = CELLS_Clamp(child.r, 0, 255);
			child.g = CELLS_Clamp(child.g, 0, 255);
			child.b = CELLS_Clamp(child.b, 0, 255);

			// if there's dead cell at child's position, child gets it's energy
			if (!CELLS_GetCell(state, facingX, facingY)->alive && !CELLS_GetCell(state, facingX, facingY)->empty)
			{
				child.energy += CELLS_GetCell(state, facingX, facingY)->energy;
			}

			CELLS_SetCell(state, facingX, facingY, child);
		}

		break;

	default:
		break;
	}

	if (nextInstruction >= GENOME_LENGTH)
		nextInstruction = 0;
	cell.currentInstruction = nextInstruction;

	cell.energy -= consumedEnergy;

	// cell can die because of age
	if (cell.age > CELL_MAX_AGE)
		suicide = true;

	if (cell.energy <= 0 || suicide)
	{
		cell.alive = false;
		cell.energy *= 0.8;
	}

	cell.age++;

	CELLS_SetCell(state, cell.x, cell.y, cell);
}

// returns NULL if fails
struct CELLS_State *CELLS_Init(const unsigned width, const unsigned height)
{

	struct CELLS_State *state = calloc(1, sizeof(struct CELLS_State));
	if (state == NULL)
		goto failed;

	state->width = width;
	state->height = height;

	state->cells = calloc(width * height, sizeof(struct CELLS_Cell));
	if (state->cells == NULL)
		goto failed;

	state->updateOrder = calloc(width * height, sizeof(unsigned));
	if (state->updateOrder == NULL)
		goto failed;

	// fill the map with cells
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{

			// every 5th pixel has a cell in it
			if (CELLS_Random(1, 5) == 1)
				CELLS_SetCell(state, i, j, CELLS_GenerateCell(i, j));
			else
				CELLS_SetCell(state, i, j, CELLS_GenerateEmptyCell(i, j));
		}
	}

	// generate random update order
	for (int i = 0; i < state->width * state->height; i++)
	{
		state->updateOrder[i] = i;
	}
	CELLS_Shuffle(state->updateOrder, state->width * state->height, state->width * state->height * 2);

	return state;

failed:
	free(state->cells);
	free(state->updateOrder);
	free(state);
	return NULL;
}

void CELLS_Quit(struct CELLS_State *state)
{
	free(state->cells);
	free(state->updateOrder);
	free(state);

	state = NULL;
}

void CELLS_Update(struct CELLS_State *state)
{
	for (int i = 0; i < state->width * state->height; i++)
	{

		unsigned coordinate = state->updateOrder[i];

		CELLS_UpdateCell(state, state->cells[coordinate]);
	}
}

struct CELLS_Cell *CELLS_GetCell(const struct CELLS_State *state, const unsigned x, const unsigned y)
{
	if (x >= state->width || y >= state->height)
		return NULL;

	return &state->cells[CELLS_ConvertCoords(state, x, y)];
}

void CELLS_SetCell(struct CELLS_State *state, const unsigned x, const unsigned y, const struct CELLS_Cell cell)
{
	if (x >= state->width || y >= state->height)
		return;

	state->cells[CELLS_ConvertCoords(state, x, y)] = cell;
}

unsigned CELLS_CountAliveCells(const struct CELLS_State *state)
{
	unsigned count = 0;

	for (int i = 0; i < state->width; i++)
	{
		for (int j = 0; j < state->height; j++)
		{
			if (CELLS_GetCell(state, i, j)->alive)
				count++;
		}
	}

	return count;
}

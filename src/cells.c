#include "cells.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct instruction cells_generate_instruction()
{
	struct instruction instruction;

	instruction.command = util_random(0, MAKE_CHILD);

	instruction.opt = rand() > RAND_MAX / 2;
	instruction.e = util_random(0, REPRODUCTION_REQUIRED_ENERGY * 2);
	instruction.b1 = util_random(0, GENOME_LENGTH);
	instruction.b2 = util_random(0, GENOME_LENGTH);
	instruction.b3 = util_random(0, GENOME_LENGTH);
	instruction.b4 = util_random(0, GENOME_LENGTH);

	return instruction;
}

struct cell cells_generate_cell(const unsigned x, const unsigned y)
{
	struct cell cell;

	// generating genome
	for (int i = 0; i < GENOME_LENGTH; i++)
	{
		cell.genome[i] = cells_generate_instruction();
	}

	cell.currentInstruction = 0;
	cell.direction = util_random(0, 3);
	cell.energy = START_ENERGY;
	cell.alive = true;
	cell.empty = false;
	cell.age = 0;

	cell.x = x, cell.y = y;

	cell.r = util_random(0, 255);
	cell.g = util_random(0, 255);
	cell.b = util_random(0, 255);

	return cell;
}

struct cell cells_generate_empty_cell(const unsigned x, const unsigned y)
{
	struct cell cell;

	cell.alive = false;
	cell.empty = true;
	cell.age = 0;

	cell.x = x, cell.y = y;

	return cell;
}

void cells_update_cell(struct cells_state *state, struct cell cell)
{
	assert(state);

	if (!cell.alive)
		return;

	struct instruction currentInstruction = cell.genome[cell.currentInstruction];
	unsigned nextInstruction = cell.currentInstruction + 1;
	float consumedEnergy = NOOP_COST;

	int facingX = cell.x, facingY = cell.y;
	switch (cell.direction)
	{
	case LEFT:
		facingX = cell.x - 1;
		break;
	case RIGHT:
		facingX = cell.x + 1;
		break;
	case UP:
		facingY = cell.y - 1;
		break;
	case DOWN:
		facingY = cell.y + 1;
	}

	if (facingX == -1)
		facingX = state->width - 1;
	else if (facingX == state->width)
		facingX = 0;

	if (facingY == -1)
		facingY = state->height - 1;
	else if (facingY == state->height)
		facingY = 0;

	struct cell *frontCell;

	switch (cell.genome[cell.currentInstruction].command)
	{
	case NOOP:
		break;

	case TURN_LEFT:
		if (cell.direction - 1 == -1)
			cell.direction = DOWN;
		else
			cell.direction--;

		consumedEnergy += TURN_COST;
		break;

	case TURN_RIGHT:
		if (cell.direction + 1 == 4)
			cell.direction = LEFT;
		else
			cell.direction++;

		consumedEnergy += TURN_COST;
		break;

	case MOVE_FORWARDS:
		// checking if space, where cell wants to move is empty
		// if it is not, cell won't move
		if (cells_get_cell(state, facingX, facingY)->empty)
		{
			cells_set_cell(state, cell.x, cell.y, cells_generate_empty_cell(cell.x, cell.y));
			cell.x = facingX, cell.y = facingY;
		}

		consumedEnergy += MOVEMENT_COST;

		break;

	case PHOTOSYNTHESIS:
		if (cell.attackCount > cell.photosynthesisCount)
			consumedEnergy -= PHOTOSYNTHESIS_ENERGY / 2;
		else
			consumedEnergy -= PHOTOSYNTHESIS_ENERGY;

		cell.photosynthesisCount++;

		break;

	case GIVE_ENERGY:
		frontCell = cells_get_cell(state, facingX, facingY);

		if (!frontCell->alive || frontCell->empty)
			break;

		float energyToGive = currentInstruction.e;
		if (energyToGive > cell.energy)
			energyToGive = cell.energy;

		frontCell->energy += energyToGive;
		consumedEnergy += energyToGive;

		break;

	case ATTACK_CELL:
		if (cell.energy < ATTACK_REQUIRED_ENERGY)
		{
			break;
		}

		cell.energy -= ATTACK_REQUIRED_ENERGY;

		frontCell = cells_get_cell(state, facingX, facingY);

		if (!frontCell->alive)
			break;

		float takenEnergy;

		enum cell_food_source food_source = cells_get_cell_food_source(cell);

		if (food_source == FOOD_SOURCE_MEAT)
		{
			// if option is true, kill the cell in fromt
			if (currentInstruction.opt)
			{
				cell.energy -= ATTACK_REQUIRED_ENERGY;
				takenEnergy = frontCell->energy * ATTACK_ENERGY;
				frontCell->alive = false;
				// cells_set_cell(state, facingX, facingY, cells_generate_empty_cell(facingX, facingY));
			}
			else
				takenEnergy = frontCell->energy * ATTACK_ENERGY;
		}
		else
			takenEnergy = frontCell->energy * (ATTACK_ENERGY / 2.f);

		frontCell->energy -= takenEnergy * 1.5f;
		cell.energy += takenEnergy;

		cell.attackCount++;

		break;

	case RECYCLE_DEAD_CELL:
		frontCell = cells_get_cell(state, facingX, facingY);

		if (frontCell->empty || frontCell->alive)
			break;

		cells_set_cell(state, facingX, facingY, cells_generate_empty_cell(facingX, facingY));
		consumedEnergy -= frontCell->energy;
		cell.eatingDeadCount++;

		break;

	case CHECK_ENERGY:
		if (cell.energy > currentInstruction.e)
			nextInstruction = currentInstruction.b1;
		else
			nextInstruction = currentInstruction.b2;

		break;

	case CHECK_ROTATION:
		switch (cell.direction)
		{
		case LEFT:
			nextInstruction = currentInstruction.b1;
		case RIGHT:
			nextInstruction = currentInstruction.b2;
		case UP:
			nextInstruction = currentInstruction.b3;
		case DOWN:
			nextInstruction = currentInstruction.b4;
		}

		break;

	case JMP_IF_FACING_ALIVE_CELL:
		if (cells_get_cell(state, facingX, facingY)->alive)
			nextInstruction = currentInstruction.b1;
		else
			nextInstruction = currentInstruction.b2;

		break;

	case JMP_IF_FACING_VOID:
		if (cells_get_cell(state, facingX, facingY)->empty)
			nextInstruction = currentInstruction.b1;
		else
			nextInstruction = currentInstruction.b2;

		break;

	case JMP_IF_FACING_DEAD_CELL:
		if (!cells_get_cell(state, facingX, facingY)->alive && !cells_get_cell(state, facingX, facingY)->empty)
			nextInstruction = currentInstruction.b1;
		else
			nextInstruction = currentInstruction.b2;

		break;

	case JMP_IF_FACING_RELATIVE:
	{
		// checking genome similarity
		unsigned similarGenes = 0;

		struct instruction *ours = cell.genome;
		struct instruction *theirs = cells_get_cell(state, facingX, facingY)->genome;

		for (int i = 0; i < GENOME_LENGTH; i++)
		{

			if (ours[i].command == theirs[i].command)
				similarGenes++;
		}

		// If at least GENOME_LENGTH-1 genes the same, treat cells as relatives
		if (similarGenes >= GENOME_LENGTH - 1)
			nextInstruction = currentInstruction.b1;
		else
			nextInstruction = currentInstruction.b2;

		break;
	}

	case MAKE_CHILD:
		// skipping instruction, if cell doesn't have enough energy
		if (cell.energy < REPRODUCTION_REQUIRED_ENERGY)
			break;

		if (!cells_get_cell(state, facingX, facingY)->empty)
			break;

		struct cell child = {};

		child.alive = true;
		child.empty = false;

		child.x = facingX;
		child.y = facingY;

		child.age = 1;

		child.direction = util_random(0, 3);

		child.r = cell.r, child.g = cell.g, child.b = cell.b;

		child.energy = START_ENERGY;

		// printf("Child energy %f; New parent energy %f\n", child.energy, cell.energy);

		memcpy(child.genome, cell.genome, sizeof(child.genome));

		// mutation can happen
		if (util_random(1, 100) <= MUTATION_PERCENT)
		{
			unsigned geneToMutate = util_random(0, GENOME_LENGTH - 1);

			child.genome[geneToMutate].command = util_random(0, MAKE_CHILD);
			child.genome[geneToMutate].opt = rand() > (RAND_MAX / 2);
			child.genome[geneToMutate].e += util_random(-3, 3);
			child.genome[geneToMutate].b1 += util_random(-2, 2);
			child.genome[geneToMutate].b2 += util_random(-2, 2);
			child.genome[geneToMutate].b3 += util_random(-2, 2);
			child.genome[geneToMutate].b4 += util_random(-2, 2);

			child.genome[geneToMutate].e = util_clamp(child.genome[geneToMutate].e, 0, REPRODUCTION_REQUIRED_ENERGY);
			child.genome[geneToMutate].b1 = util_clamp(child.genome[geneToMutate].e, 0, GENOME_LENGTH - 1);
			child.genome[geneToMutate].b2 = util_clamp(child.genome[geneToMutate].e, 0, GENOME_LENGTH - 1);
			child.genome[geneToMutate].b3 = util_clamp(child.genome[geneToMutate].e, 0, GENOME_LENGTH - 1);
			child.genome[geneToMutate].b4 = util_clamp(child.genome[geneToMutate].e, 0, GENOME_LENGTH - 1);

			// changing child's color a bit
			unsigned colorToChange = util_random(0, 2);

			if (colorToChange == 0)
				child.r += util_random(-16, 16);
			if (colorToChange == 1)
				child.g += util_random(-16, 16);
			if (colorToChange == 2)
				child.b += util_random(-16, 16);
		}

		child.r = util_clamp(child.r, 0, 255);
		child.g = util_clamp(child.g, 0, 255);
		child.b = util_clamp(child.b, 0, 255);

		// if there's dead cell at child's position, child gets it's energy
		if (!cells_get_cell(state, facingX, facingY)->alive && !cells_get_cell(state, facingX, facingY)->empty)
		{
			child.energy += cells_get_cell(state, facingX, facingY)->energy;
		}

		cells_set_cell(state, facingX, facingY, child);

		break;

	default:
		break;
	}

	if (nextInstruction >= GENOME_LENGTH)
		nextInstruction %= GENOME_LENGTH;
	cell.currentInstruction = nextInstruction;

	cell.energy -= consumedEnergy;

	if (cell.age > CELL_MAX_AGE || cell.energy < 0)
	{
		cell.alive = false;
	}

	cell.age++;

	cells_set_cell(state, cell.x, cell.y, cell);
}

struct cells_state *cells_init()
{
	struct cells_state *state = calloc(1, sizeof(struct cells_state));
	assert(state);

	state->width = SIMULATION_HEIGHT;
	state->height = SIMULATION_HEIGHT;

	// fill the map with cells
	for (int i = 0; i < SIMULATION_WIDTH; i++)
	{
		for (int j = 0; j < SIMULATION_HEIGHT; j++)
		{

			// every 5th pixel has a cell in it
			if (util_random(1, 5) == 1)
				cells_set_cell(state, i, j, cells_generate_cell(i, j));
			else
				cells_set_cell(state, i, j, cells_generate_empty_cell(i, j));
		}
	}

	return state;
}

void cells_quit(struct cells_state *state)
{
	free(state);

	state = NULL;
}

void cells_update_state(struct cells_state *state)
{
	for (int i = 0; i < state->width; i++)
	{
		for (int j = 0; j < state->height; j++)
		{
			cells_update_cell(state, state->cells[i][j]);
		}
	}
}

struct cell *cells_get_cell(struct cells_state *state, const unsigned x, const unsigned y)
{
	if (x >= state->width || y >= state->height)
		return NULL;

	return &state->cells[x][y];
}

void cells_set_cell(struct cells_state *state, const unsigned x, const unsigned y, const struct cell cell)
{
	if (x >= state->width || y >= state->height)
		return;

	state->cells[x][y] = cell;
}

unsigned cells_count_alive_cells(struct cells_state *state)
{
	unsigned count = 0;

	for (int i = 0; i < state->width; i++)
	{
		for (int j = 0; j < state->height; j++)
		{
			if (cells_get_cell(state, i, j)->alive)
				count++;
		}
	}

	return count;
}

enum cell_food_source cells_get_cell_food_source(struct cell cell)
{
	float max = util_max(3, (float[]){cell.photosynthesisCount, cell.attackCount, cell.eatingDeadCount});

	if (max == cell.photosynthesisCount)
	{
		return FOOD_SOURCE_PHOTOSYNTHESIS;
	}
	else if (max == cell.attackCount)
	{
		return FOOD_SOURCE_MEAT;
	}
	else if (max == cell.eatingDeadCount)
	{
		return FOOD_SOURCE_DEAD_CELLS;
	}
	else
	{
		return FOOD_SOURCE_UNKNOWN;
	}
}
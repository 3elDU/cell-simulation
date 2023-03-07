#include "cells.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct instruction cells_generate_instruction()
{
	struct instruction instruction;

	instruction.command = util_random(0, MAKE_CHILD);

	instruction.ax = util_random(0, 128);
	instruction.bx = util_random(0, 63);
	instruction.cx = util_random(0, 100000) / 1000.f;

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
		if (cells_get_cell(state, facingX, facingY)->empty)
		{

			cells_set_cell(state, cell.x, cell.y, cells_generate_empty_cell(cell.x, cell.y));
			cell.x = facingX, cell.y = facingY;
		}
		else if (!cells_get_cell(state, facingX, facingY)->alive)
		{
			// if space, where cell wants to move is occupied by dead cell, eating it
			cell.energy += cells_get_cell(state, facingX, facingY)->energy;

			cells_set_cell(state, cell.x, cell.y, cells_generate_empty_cell(cell.x, cell.y));
			cell.x = facingX, cell.y = facingY;

			cell.eatingDeadCount++;
		}

		consumedEnergy += MOVEMENT_COST;

		break;

	case SWAP_PLACES:
		frontCell = cells_get_cell(state, facingX, facingY);

		frontCell->x = cell.x;
		frontCell->y = cell.y;

		cells_set_cell(state, cell.x, cell.y, *frontCell);

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

		frontCell = cells_get_cell(state, facingX, facingY);

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

		frontCell = cells_get_cell(state, facingX, facingY);

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
		if (cells_get_cell(state, facingX, facingY)->alive)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_VOID:
		if (cells_get_cell(state, facingX, facingY)->empty)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_DEAD_CELL:
		if (!cells_get_cell(state, facingX, facingY)->alive && !cells_get_cell(state, facingX, facingY)->empty)
			nextInstruction = cell.genome[cell.currentInstruction].bx;

		break;

	case JMP_IF_FACING_RELATIVE:
	{
		// checking genome similarity
		unsigned similarGenes = 0;

		struct instruction *ours = cell.genome;
		struct instruction *theirs = cells_get_cell(state, facingX, facingY)->genome;

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
		if (!cells_get_cell(state, facingX, facingY)->alive)
		{

			struct cell child = {};

			child.alive = true;
			child.empty = false;

			child.x = facingX;
			child.y = facingY;

			child.age = 1;

			child.direction = util_random(0, 3);

			child.r = cell.r, child.g = cell.g, child.b = cell.b;

			// giving CX percents of cell's own energy to the child
			child.energy = cell.energy * (cell.genome[cell.currentInstruction].cx / 100.f);
			cell.energy *= 1 - cell.genome[cell.currentInstruction].cx / 100.f;

			// printf("Child energy %f; New parent energy %f\n", child.energy, cell.energy);

			// copying parent genome to the child
			for (int i = 0; i < GENOME_LENGTH; i++)
			{

				// mutation can happen
				if (util_random(1, 100) <= MUTATION_PERCENT)
				{
					// mutating
					child.genome[i] = cells_generate_instruction();

					// changing child's color a bit
					unsigned colorToChange = util_random(0, 2);

					if (colorToChange == 0)
						child.r += util_random(-16, 16);
					if (colorToChange == 1)
						child.g += util_random(-16, 16);
					if (colorToChange == 2)
						child.b += util_random(-16, 16);
				}
				else
					child.genome[i] = cell.genome[i];
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

	cells_set_cell(state, cell.x, cell.y, cell);
}

// returns NULL if fails
struct cells_state *cells_init(const unsigned width, const unsigned height)
{
	struct cells_state *state = calloc(1, sizeof(struct cells_state));
	if (state == NULL)
		goto failed;

	state->width = width;
	state->height = height;

	state->cells = calloc(width * height, sizeof(struct cell));
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
			if (util_random(1, 5) == 1)
				cells_set_cell(state, i, j, cells_generate_cell(i, j));
			else
				cells_set_cell(state, i, j, cells_generate_empty_cell(i, j));
		}
	}

	// generate random update order
	for (int i = 0; i < state->width * state->height; i++)
	{
		state->updateOrder[i] = i;
	}
	util_shuffle(state->updateOrder, state->width * state->height, state->width * state->height * 2);

	return state;

failed:
	free(state->cells);
	free(state->updateOrder);
	free(state);
	return NULL;
}

void cells_quit(struct cells_state *state)
{
	free(state->cells);
	free(state->updateOrder);
	free(state);

	state = NULL;
}

void cells_update_state(struct cells_state *state)
{
	for (int i = 0; i < state->width * state->height; i++)
	{

		unsigned coordinate = state->updateOrder[i];

		cells_update_cell(state, state->cells[coordinate]);
	}
}

struct cell *cells_get_cell(const struct cells_state *state, const unsigned x, const unsigned y)
{
	if (x >= state->width || y >= state->height)
		return NULL;

	return &state->cells[CELLS_ConvertCoords(state, x, y)];
}

void cells_set_cell(struct cells_state *state, const unsigned x, const unsigned y, const struct cell cell)
{
	if (x >= state->width || y >= state->height)
		return;

	state->cells[CELLS_ConvertCoords(state, x, y)] = cell;
}

unsigned cells_count_alive_cells(const struct cells_state *state)
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

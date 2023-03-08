#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include "cells.h"
#include "defines.h"
#include "util.h"

enum RENDERING_MODE
{
	RENDER_ENERGY,
	RENDER_RELATIVES,
	RENDER_AGE,
	RENDER_ENERGY_SOURCE
};

SDL_Window *win;
SDL_Renderer *ren;

SDL_Texture *loadImage(const char *path)
{

	SDL_Texture *texture = NULL;

	SDL_Surface *surf = IMG_Load(path);
	assert(surf);

	SDL_Surface *optimizedSurf = SDL_ConvertSurface(surf, SDL_GetWindowSurface(win)->format, 0);
	assert(optimizedSurf);
	SDL_FreeSurface(surf);

	texture = SDL_CreateTextureFromSurface(ren, optimizedSurf);
	if (texture != NULL)
	{
		SDL_FreeSurface(optimizedSurf);
	}

	return texture;
}

void init()
{
	// init SDL
	assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	int imgFlags = IMG_INIT_PNG;
	assert(IMG_Init(imgFlags) == imgFlags);

	// create window
	win = SDL_CreateWindow(
		"Cell simulation",												// title
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,					// x and y position
		SIMULATION_WIDTH * CELL_WIDTH, SIMULATION_HEIGHT * CELL_HEIGHT, // width and height
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE							// flags
	);
	assert(win);

	// create renderer
	ren = SDL_CreateRenderer(
		win,
		-1,
		SDL_RENDERER_ACCELERATED);
	assert(ren);
}

bool quit()
{
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	ren = NULL;
	win = NULL;

	SDL_Quit();
	IMG_Quit();
}

void render(struct cells_state *state, enum RENDERING_MODE renderingMode)
{
	// clear the screen with black color
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);

	// render the map
	for (int i = 0; i < SIMULATION_WIDTH; i++)
	{
		for (int j = 0; j < SIMULATION_HEIGHT; j++)
		{

			struct cell *cell = cells_get_cell(state, i, j);

			if (cell->empty)
				continue;

			unsigned r, g, b;

			if (cell->alive)
			{

				if (renderingMode == RENDER_RELATIVES)
					r = cell->r, g = cell->g, b = cell->b;
				else if (renderingMode == RENDER_ENERGY)
					r = 30 + cell->energy, g = r, b = 0;
				else if (renderingMode == RENDER_AGE)
					r = 0, g = 0, b = 50 + 255.f * ((float)cell->age / (float)CELL_MAX_AGE);
				else if (renderingMode == RENDER_ENERGY_SOURCE)
					if (cell->attackCount > cell->photosynthesisCount && cell->attackCount > cell->eatingDeadCount)
						r = 255, g = 0, b = 0;
					else if (cell->photosynthesisCount > cell->attackCount && cell->photosynthesisCount > cell->eatingDeadCount)
						r = 0, g = 255, b = 0;
					else if (cell->eatingDeadCount > cell->attackCount && cell->eatingDeadCount > cell->photosynthesisCount)
						r = 0, g = 255, b = 255;
					else
						r = 255, g = 255, b = 255;
			}
			else
			{
				r = 70, g = 70, b = 70;
			}

			r = util_clamp(r, 0, 255);
			g = util_clamp(g, 0, 255);
			b = util_clamp(b, 0, 255);

			SDL_SetRenderDrawColor(ren, r, g, b, 255);
			SDL_Rect fillRect = {i * CELL_WIDTH, j * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT};
			SDL_RenderFillRect(ren, &fillRect);
		}
	}

	SDL_RenderPresent(ren);
}

int main(int argc, char *argv[])
{
	srand(time(0));
	init();

	// initialize the simulation
	struct cells_state *state = cells_init(SIMULATION_WIDTH, SIMULATION_HEIGHT);
	assert(state);

	enum RENDERING_MODE renderingMode = RENDER_RELATIVES;
	long long unsigned iterations = 0;
	bool exit = false;
	bool paused = false;
	bool headless = false;
	struct timeval frame_start, frame_end;

	while (!exit)
	{
		// measure time at the beginning of frame
		gettimeofday(&frame_start, NULL);

		// handle events
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			// user requests quit
			if (e.type == SDL_QUIT)
				exit = true;

			else if (e.type == SDL_KEYDOWN)
			{

				switch (e.key.keysym.sym)
				{
					FILE *f;

				case SDLK_r:
					// re-initialize state, when R is pressed
					cells_quit(state);
					state = cells_init(SIMULATION_WIDTH, SIMULATION_HEIGHT);
					iterations = 0;

					break;

				case SDLK_SPACE:
					paused = !paused;
					break;

				case SDLK_h:
					headless = !headless;
					break;

				// Step by one frame
				case SDLK_f:
					cells_update_state(state);
					break;

				case SDLK_1:
					renderingMode = RENDER_ENERGY;
					break;

				case SDLK_2:
					renderingMode = RENDER_RELATIVES;
					break;

				case SDLK_3:
					renderingMode = RENDER_AGE;
					break;

				case SDLK_4:
					renderingMode = RENDER_ENERGY_SOURCE;
					break;

				case SDLK_s:
					// save map to the file
					f = fopen("save.bin", "w");

					fwrite(state->cells, sizeof(struct cell), SIMULATION_WIDTH * SIMULATION_HEIGHT, f);

					fclose(f);

					break;

				case SDLK_l:
					// load map from the file
					f = fopen("save.bin", "r");

					fread(state->cells, sizeof(struct cell), SIMULATION_WIDTH * SIMULATION_HEIGHT, f);

					fclose(f);

				default:
					break;
				}
			}
		}

		if (!headless)
			render(state, renderingMode);

		if (!paused)
		{
			cells_update_state(state);
			iterations++;

			gettimeofday(&frame_end, NULL);
			int fps = 1.f / ((frame_end.tv_usec - frame_start.tv_usec) / 1000000.f);
			if (fps == -1)
			{ // sometimes this happens, fps is reported as -1 for some reason
				fps = 0;
			}

			if (iterations % 10 == 0)
				printf("[ iteration %llu ] [ fps %d ] Alive cells: %u\n", iterations, fps, cells_count_alive_cells(state));
		}
		else
			SDL_Delay(100); // when on pause, run at 10 frames per second
	}

	cells_quit(state);
	quit();

	return 0;
}

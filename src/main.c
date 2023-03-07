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
	if (surf == NULL)
	{
		printf("IMG_Load() failed with error '%s'. file - %s\n", IMG_GetError(), path);
		return NULL;
	}

	SDL_Surface *optimizedSurf = SDL_ConvertSurface(surf, SDL_GetWindowSurface(win)->format, 0);
	if (optimizedSurf == NULL)
	{
		printf("SDL_ConvertSurface() failed with error '%s'\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return NULL;
	}
	SDL_FreeSurface(surf);

	texture = SDL_CreateTextureFromSurface(ren, optimizedSurf);
	if (texture != NULL)
	{
		SDL_FreeSurface(optimizedSurf);
	}

	return texture;
}

bool init()
{
	// init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init() error - %s\n", SDL_GetError());
		goto failed;
	}

	int imgFlags = IMG_INIT_PNG;
	if (IMG_Init(imgFlags) != imgFlags)
	{
		printf("Cannot initalize SDL_image - '%s'\n", IMG_GetError());
		goto failed;
	}

	// create window
	win = SDL_CreateWindow(
		"Cell simulation",												// title
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,					// x and y position
		SIMULATION_WIDTH * CELL_WIDTH, SIMULATION_HEIGHT * CELL_HEIGHT, // width and height
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE							// flags
	);
	if (win == NULL)
	{
		printf("SDL_CreateWindow() error - %s\n", SDL_GetError());
		goto failed;
	}

	// create renderer
	ren = SDL_CreateRenderer(
		win,
		-1,
		SDL_RENDERER_ACCELERATED);
	if (ren == NULL)
	{
		printf("SDL_CreateRenderer() error - %s\n", SDL_GetError());
		goto failed;
	}

	return true;

failed:
	win = NULL;
	ren = NULL;
	return false;
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

int main(int argc, char *argv[])
{
	// initialize random
	srand(time(0));

	// init sdl and everything
	if (!init())
	{
		printf("SDL initialization failed\n");
		return EXIT_FAILURE;
	}

	// initialize the simulation
	struct CELLS_State *state = CELLS_Init(SIMULATION_WIDTH, SIMULATION_HEIGHT);
	if (state == NULL)
	{
		return EXIT_FAILURE;
	}

	enum RENDERING_MODE currentRenderingMode = RENDER_RELATIVES;

	long long unsigned iterations = 0;

	// main loop
	bool exit = false;
	bool paused = false;
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
					CELLS_Quit(state);
					state = CELLS_Init(SIMULATION_WIDTH, SIMULATION_HEIGHT);
					iterations = 0;

					break;

				case SDLK_SPACE:
					paused = !paused;
					break;

				case SDLK_1:
					currentRenderingMode = RENDER_ENERGY;
					break;

				case SDLK_2:
					currentRenderingMode = RENDER_RELATIVES;
					break;

				case SDLK_3:
					currentRenderingMode = RENDER_AGE;
					break;

				case SDLK_4:
					currentRenderingMode = RENDER_ENERGY_SOURCE;
					break;

				case SDLK_s:
					// save map to the file
					f = fopen("save.bin", "w");

					fwrite(state->cells, sizeof(struct CELLS_Cell), SIMULATION_WIDTH * SIMULATION_HEIGHT, f);

					fclose(f);

					break;

				case SDLK_l:
					// load map from the file
					f = fopen("save.bin", "r");

					fread(state->cells, sizeof(struct CELLS_Cell), SIMULATION_WIDTH * SIMULATION_HEIGHT, f);

					fclose(f);

				default:
					break;
				}
			}
		}

		// clear the screen with black color
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		// render the map
		for (int i = 0; i < SIMULATION_WIDTH; i++)
		{
			for (int j = 0; j < SIMULATION_HEIGHT; j++)
			{

				struct CELLS_Cell *cell = CELLS_GetCell(state, i, j);

				if (cell->empty)
					continue;

				unsigned r, g, b;

				if (cell->alive)
				{

					if (currentRenderingMode == RENDER_RELATIVES)
						r = cell->r, g = cell->g, b = cell->b;
					else if (currentRenderingMode == RENDER_ENERGY)
						r = 30 + cell->energy, g = r, b = 0;
					else if (currentRenderingMode == RENDER_AGE)
						r = 0, g = 0, b = 50 + 255.f * ((float)cell->age / (float)CELL_MAX_AGE);
					else if (currentRenderingMode == RENDER_ENERGY_SOURCE)
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

				r = CELLS_Clamp(r, 0, 255);
				g = CELLS_Clamp(g, 0, 255);
				b = CELLS_Clamp(b, 0, 255);

				SDL_SetRenderDrawColor(ren, r, g, b, 255);
				SDL_Rect fillRect = {i * CELL_WIDTH, j * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT};
				SDL_RenderFillRect(ren, &fillRect);
			}
		}

		// update the screen
		SDL_RenderPresent(ren);

		if (!paused)
		{
			CELLS_Update(state);
			iterations++;

			gettimeofday(&frame_end, NULL);
			int fps = 1.f / ((frame_end.tv_usec - frame_start.tv_usec) / 1000000.f);
			if (fps == -1)
			{ // sometimes this happens, fps is reported as -1 for some reason
				fps = 0;
			}

			printf("[ iteration %llu ] [ fps %d ] Alive cells: %u\n", iterations, fps, CELLS_CountAliveCells(state));
		}
		else
			SDL_Delay(100); // when on pause, run at 10 frames per second
	}

	CELLS_Quit(state);
	quit();

	return 0;
}

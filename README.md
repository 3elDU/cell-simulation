# Cell evolution
Evolution simulation, written in C

### Dependencies
- SDL2

### Platforms
- Linux
- Probably works on Macos
- ~Windows~

### Building
- `git clone https://github.com/3elDU/cell-simulation.git`
- `cd cell-simulation`
- `make`

### Keys
- S - save simulation to the file ( save.bin by default, but you can change it in main.c )
- L - load simulation from the file.
- Space - pause/unpause
- R - reload the map

### Code architecture
Some variables can be configured at compile-time. They are located in `src/defines.h`.
You can change `SIMULATION_WIDTH` and `SIMULATION_HEIGHT` to fit your computer power or other needs.
If the window is too big or too small, you can change `CELL_WIDTH` and `CELL_HEIGHT`.
Every cell is like a tiny virtual machine. It has its own memory ( genome ), instruction pointer, and all cell-like things.
It can move, generate energy, eat other cells, reproduct. There are even conditional jumps.
Everything works because of [natural selection](https://en.wikipedia.org/wiki/Natural_selection).
When cell makes it's own copy, there's a change for each gene to mutate ( as said earlier, you can change it in `src/defines.h` ).
So, child will be different from it's parent, and so on. There's a lot of cells, so at least some have "lucky" enough genome to survive.
They evolve, reproduct, kill each other, and do all that "live" things. Since everything is written in C, simulation runs very smoothly.

If something breaks for you, don't be afraid to create an issue.
If you want to add something new, create a PR, i'll be happy your additions.

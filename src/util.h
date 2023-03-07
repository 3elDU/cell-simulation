#ifndef UTIL_H
#define UTIL_H

#define CELLS_Clamp(val, min, max) (val > max) ? max : ((val < min) ? min : val)
#define CELLS_Random(from, to) from + rand() % (to + 1 - from)

void CELLS_Shuffle(unsigned *arr, const unsigned length, const unsigned times);

#endif
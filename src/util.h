#ifndef UTIL_H
#define UTIL_H

int CELLS_Random(int from, int to);

float CELLS_Clampf(float val, float min, float max);
int CELLS_Clampi(int val, int min, int max);
unsigned CELLS_Clampu(unsigned val, unsigned min, unsigned max);

void CELLS_Shuffle(unsigned* arr, const unsigned length, const unsigned times);

#endif
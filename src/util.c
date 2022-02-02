#include "util.h"
#include <stdlib.h>

int CELLS_Random(int from, int to) {
	return from+rand()%(to+1-from);
}

float CELLS_Clampf(float val, float min, float max) {
	if (val > max) return max;
	else if (val < min) return min;
	else return val;
}

int CELLS_Clampi(int val, int min, int max) {
	if (val > max) return max;
	else if (val < min) return min;
	else return val;
}

unsigned CELLS_Clampu(unsigned val, unsigned min, unsigned max) {
	if (val > max) return max;
	else if (val < min) return min;
	else return val;
}

// yes, this code is very slooow,
// but it is only used once in cell state initialization
void CELLS_Shuffle(unsigned* arr, const unsigned length, const unsigned times) {
	for (int i = 0; i < times; i++) {
		int j = CELLS_Random(0, length-1);
		int w = CELLS_Random(0, length-1);

		if (j == w) continue;

		unsigned temp = arr[j];
		arr[j] = arr[w];
		arr[w] = temp;
	}
}
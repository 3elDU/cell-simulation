#ifndef UTIL_H
#define UTIL_H

#define util_clamp(val, min, max) (val > max) ? max : ((val < min) ? min : val)
#define util_random(from, to) from + rand() % (to + 1 - from)

void util_shuffle(unsigned *arr, const unsigned length, const unsigned times);

#endif
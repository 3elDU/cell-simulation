#ifndef UTIL_H
#define UTIL_H

#define util_clamp(val, min, max) (val > max) ? max : ((val < min) ? min : val)
#define util_random(from, to) from + rand() % (to + 1 - from)

#define assert(expr) \
    if (expr)        \
        ;            \
    else             \
        util_panic("file %s: %d: %s - assert failed for '%s'", __FILE__, __LINE__, __func__, #expr)

void util_shuffle(unsigned *arr, const unsigned length, const unsigned times);
__attribute__((noreturn)) void util_panic(const char *fmt, ...);

inline float util_max(unsigned int size, float values[size])
{
    if (size == 0)
        return -1;

    float max = 0;
    for (int i = 0; i < size; i++)
        if (values[i] > max)
            max = values[i];
    return max;
}

#endif
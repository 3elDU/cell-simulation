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

#endif
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// yes, this code is very slooow,
// but it is only used once in cell state initialization
void util_shuffle(unsigned *arr, const unsigned length, const unsigned times)
{
	for (int i = 0; i < times; i++)
	{
		int j = util_random(0, length - 1);
		int w = util_random(0, length - 1);

		if (j == w)
			continue;

		unsigned temp = arr[j];
		arr[j] = arr[w];
		arr[w] = temp;
	}
}

__attribute__((noreturn)) void util_panic(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char buf[16384];
	vsnprintf(buf, 16384, fmt, args);

	va_end(args);

	fprintf(stderr, "PANIC; REASON: %s\n", buf);
	exit(EXIT_FAILURE);
}
#include <stdint.h>
#include <time.h>

#include "util.h"

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	return (uint16_t) ((r << 11) | (g << 5) | b);
}

void __nanosleep(const struct timespec *req, struct timespec *rem)
{
	struct timespec _rem;
	if (nanosleep(req, rem) == -1)
		__nanosleep(rem, &_rem);
}

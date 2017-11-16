#include <stdint.h>
#include <time.h>

#include "util.h"

static struct timespec req = {0}, rem = {0};

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

void __ssleep(int sec)
{
	req.tv_sec = 1;
	req.tv_nsec = 0;
	__nanosleep(&req, &rem);
}

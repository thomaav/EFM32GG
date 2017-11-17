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

struct decimal_string number_to_dstring(uint32_t number)
{
	struct decimal_string dstring;

	int i = 0; int j;
	// special case for 0 points
	if (!number) {
		dstring.digits[0] = 0;
		dstring.length = 1;
	} else {
		while (number != 0) {
			dstring.digits[i++] = number % 10;
			number /= 10;
		}

		dstring.length = i;
	}

	// now reverse the string we read in
	i = dstring.length - 1;
	j = 0;
	while (i > j) {
		uint8_t tmp = dstring.digits[i];
		dstring.digits[i--] = dstring.digits[j];
		dstring.digits[j++] = tmp;
	}

	return dstring;
}

#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "efm32gg.h"
#include "timer.h"

void setup_timer(uint32_t sample_rate)
{
	uint16_t max_tick = 14000000 / (NOTE*2);

	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
	*TIMER1_TOP = max_tick;
	*TIMER1_CMD = 0x1;
}

void disable_timer()
{
	*CMU_HFPERCLKEN0 &= ~(CMU2_HFPERCLKEN0_TIMER1);
}

void enable_timer()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
}

#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "efm32gg.h"

// setup for timer to generate interrupts
void setup_Timer()
{
	// general setup for Timer1 interrupts. we enable interrupts
	// when the 16-bit register ticks to 62500 (0xF424) since 14
	// 000 000 / 62500 = 224, which makes ticking a bit simpler
	// 0x13D means 14 000 000 / 44100 = 317 (0x13D)

	uint16_t max_tick = 14000000 / SAMPLE_RATE;

	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;	// enable clock
	*TIMER1_TOP = max_tick;
	*TIMER1_IEN = 0x1;	// enable interrupt generation
	*TIMER1_CMD = 0x1;	// start timer
}

void disable_timer()
{
	*CMU_HFPERCLKEN0 &= ~(CMU2_HFPERCLKEN0_TIMER1);
}

void enable_timer()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
}

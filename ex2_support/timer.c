#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/*
 * function to setup the timer
 */
void setupTimer(uint16_t period)
{
	// general setup for Timer1 interrupts. we enable interrupts
	// when the 16-bit register ticks to 62500 (0xF424) since 14
	// 000 000 / 62500 = 224, which makes ticking a bit simpler
	// 0x13D means 14 000 000 / 44100 = 317 (0x13D)

	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1; // enable clock
	*TIMER1_TOP = 0x13D;
	*TIMER1_IEN = 0x1; // enable interrupt generation
	*TIMER1_CMD = 0x1; // start timer
}

#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/*
 * function to setup the timer
 */
void setupTimer(uint16_t period)
{
	// general setup for Timer1 interrupts
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
	*TIMER1_TOP = 0xFFFF; // enable interrupts when 16-bit register ticks to max value
	*TIMER1_IEN = 0x1; // enable interrupt generation
	*TIMER1_CMD = 0x1; // start timer
}

#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "dac.h"

void setup_DAC()
{
	// general setup for DAC
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0;
	*DAC0_CTRL = 0x50010;	// 473.5KHz prescale clocking, enable output to amp
	*DAC0_CH0CTRL = 0x5;	// PRS input
	*DAC0_CH1CTRL = 0x5;	// PRS input
}

void enable_DAC()
{
	*DAC0_CH0CTRL = 1;
	*DAC0_CH1CTRL = 1;
}

void disable_DAC()
{
	*DAC0_CH0CTRL = 0;
	*DAC0_CH1CTRL = 0;
}

#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

// setup to enable DAC left and right channels
void setup_DAC()
{
	// general setup for DAC
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0; // (1 << 17) to enable clock
	*DAC0_CTRL = 0x50010; // 473.5KHz prescale clocking, enable output to amp
	*DAC0_CH0CTRL = 0x1; // enable left and right DAC channels
	*DAC0_CH1CTRL = 0x1; // enable left and right DAC channels

	// if wanted, push initial sample to DAC0_CH{0,1}DATA
	;
}

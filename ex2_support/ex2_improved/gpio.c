#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

// setup for LEDs, buttons, and enabling interrupts
void setup_GPIO()
{
	// general setup for GPIO
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	// (1 << 13) to enable clock
	*GPIO_PA_CTRL = 0x1;	// high drive strenght for LEDs

	// set LEDs (A8-A15) to output mode
	*GPIO_PA_MODEH = 0x55555555;

	// set buttons (C1-C7) to input mode and enable internal pull-up
	*GPIO_PC_MODEL = 0x33333333;
	*GPIO_PC_DOUT = 0xFF;

	// enable GPIO interrupts
	*GPIO_EXTIPSELL = 0x22222222;
	*GPIO_EXTIFALL = 0xFF;	// interrupts on 1->0 transition for buttons
	*GPIO_IEN = 0xFF;
}

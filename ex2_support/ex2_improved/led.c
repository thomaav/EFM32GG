#include <stdint.h>
#include <stdbool.h>

#include "led.h"
#include "efm32gg.h"

// toggle given leds
void toggle_led(uint32_t led)
{
	*GPIO_PA_DOUT ^= led;
}

// enable the leds (i.e. turn it on)
void enable_led(uint32_t led)
{
	*GPIO_PA_DOUT = *GPIO_PA_DOUT & ~led;
}

// enable the given leds (i.e. turn it off)
void disable_led(uint32_t led)
{
	*GPIO_PA_DOUT |= led;
}

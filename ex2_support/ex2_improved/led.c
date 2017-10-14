#include <stdint.h>
#include <stdbool.h>

#include "led.h"
#include "efm32gg.h"

void toggle_led(uint32_t led)
{
	*GPIO_PA_DOUT ^= led;
}

void enable_led(uint32_t led)
{
	*GPIO_PA_DOUT = *GPIO_PA_DOUT & ~led;
}

void disable_led(uint32_t led)
{
	*GPIO_PA_DOUT |= led;
}

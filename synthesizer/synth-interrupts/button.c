#include <stdint.h>
#include <stdbool.h>

#include "button.h"
#include "efm32gg.h"

uint32_t get_buttons_pressed()
{
	return *GPIO_PC_DIN;
}

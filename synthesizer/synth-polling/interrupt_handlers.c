#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "efm32gg.h"

void GPIO_IRQHandler()
{
	extern _Bool button_press;
	extern uint32_t buttons_pressed;
	button_press = 1;
	buttons_pressed = *GPIO_PC_DIN;

	// clear interrupt
	*GPIO_IFC = *GPIO_IF;
}

/*
 * TIMER1 interrupt handler
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	;
}

/*
 * GPIO even pin interrupt handler
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	// we don't care whether it is even or odd
	GPIO_IRQHandler();
}

/*
 * GPIO odd pin interrupt handler
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	// we don't care whether it is even or odd
	GPIO_IRQHandler();
}

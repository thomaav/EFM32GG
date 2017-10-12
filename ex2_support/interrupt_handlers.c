#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void GPIO_IRQHandler()
{
	// write button values to leds to test
	uint32_t btn_values = *GPIO_PC_DIN;
	*GPIO_PA_DOUT = (btn_values << 8);

	// clear interrupt
	*GPIO_IFC = *GPIO_IF;
}

/*
 * TIMER1 interrupt handler
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	/*
	 * TODO feed new samples to the DAC remember to clear the pending
	 * interrupt by writing 1 to TIMER1_IFC
	 */
	// use global tick counter from ex2.c
	extern uint16_t tick_counter;

	if (++tick_counter >= 224) {
		tick_counter = 0;
	}

	// clear interrupt flag
	*TIMER1_IFC = 0x1;
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

#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void GPIO_IRQHandler()
{
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

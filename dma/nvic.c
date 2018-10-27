#include "efm32gg.h"
#include "nvic.h"

void setup_NVIC()
{
	// to enable GPIO-interrupts, write bits 1 and 11, DMA is bit 0
	*ISER0 |= 0x803;
}

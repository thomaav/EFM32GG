#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "button.h"
#include "dma.h"
#include "efm32gg.h"
#include "led.h"

void GPIO_IRQHandler()
{
	*GPIO_IFC = *GPIO_IF;
	toggle_led(0x0000FF00);
}

void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	GPIO_IRQHandler();
}

void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_IRQHandler();
}

void __attribute__ ((interrupt)) DMA_IRQHandler() {
	int ch = 0;
	uint32_t buf = (uint32_t) ping;

	if (curbuf) {
		ch = 16;
		buf = (uint32_t) pong;
	}

	curbuf = !curbuf;

	dma[ch].src_end = buf;
	dma[ch].dst_end = (uint32_t) DAC0_COMBDATA;

	dma[ch].ctrl.n_minus_1 = BUFSIZE - 1;
	dma[ch].ctrl.cycle_ctrl = 3;

	*DMA_CHUSEBURSTC = 1;
	*DMA_IFC = 1;
}

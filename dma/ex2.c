#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "button.h"
#include "dac.h"
#include "dma.h"
#include "efm32gg.h"
#include "gpio.h"
#include "nvic.h"
#include "timer.h"

void init_system()
{
	setup_GPIO();
	setup_DAC();
	setup_timer(SAMPLE_RATE);
	setup_NVIC();
	setup_DMA();
}

int main(void)
{
	init_system();

	while (true) {
		;
	}

	return 0;
}

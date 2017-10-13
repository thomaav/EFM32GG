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
	// globals needed
	extern uint16_t tick_counter;
	extern uint16_t max_amplitude;
	extern _Bool square_high_treble;
	extern _Bool square_high_bass;
	extern int16_t msec_left;

	// fetch current note
	extern uint16_t current_treble_note;
	extern uint16_t current_bass_note;

	uint16_t samples_per_period_treble = SAMPLE_RATE / current_treble_note;
	uint16_t samples_per_period_bass = SAMPLE_RATE / current_bass_note;

	++tick_counter;

	// count msec
	if (!(tick_counter % (SAMPLE_RATE / 100))) {
		if (msec_left > 0) {
			if (msec_left - 10 < 0) {
				msec_left = 0;
			} else {
				msec_left -= 10;
			}
		}
	}

	if (!msec_left) {
		*TIMER1_IFC = 0x1;
		return;
	}

	if (!(tick_counter % (samples_per_period_treble / 2))) {
		square_high_treble = !square_high_treble;
	}

	if (!(tick_counter % (samples_per_period_bass / 2))) {
		square_high_bass = !square_high_bass;
	}

	// create a square sound function
	if (square_high_treble) {
		*DAC0_CH0DATA = max_amplitude;
	} else {
		*DAC0_CH0DATA = 0x000;
	}

	if (square_high_bass) {
		*DAC0_CH1DATA = max_amplitude;
	} else {
		*DAC0_CH1DATA = 0x000;
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

#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "button.h"
#include "efm32gg.h"

void GPIO_IRQHandler()
{
	// we need the player to possible change the current melody
	extern struct player sound_player;

	uint32_t buttons_pressed = *GPIO_PC_DIN;

	if (buttons_pressed == SW1) {
		set_current_melody(&sound_player, windows_xp_startup_melody);
	} else if (buttons_pressed == SW2) {
		;
	} else if (buttons_pressed == SW3) {
		;
	} else if (buttons_pressed == SW4) {
		;
	} else if (buttons_pressed == SW5) {
		;
	} else if (buttons_pressed == SW6) {
		;
	}

	// clear interrupt
	*GPIO_IFC = *GPIO_IF;
}

/*
 * TIMER1 interrupt handler
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	// sound player needed
	extern struct player sound_player;
	extern struct melody empty_melody;

	// globals needed
	extern uint16_t tick_counter;
	extern uint16_t max_amplitude;

	extern _Bool square_high_treble;
	extern _Bool square_high_bass;

	// fetch current note
	extern uint16_t current_bass_note;

	++tick_counter;

	// test that all is good by blinking the LEDs every second
	if (tick_counter == SAMPLE_RATE) {
		tick_counter = 0;
		*GPIO_PA_DOUT ^= 0xFFFF;
	}

	if (!sound_player.current_melody.notes) {
		*TIMER1_IFC = 0x1;
		return;
	}

	struct melody *current_melody = &(sound_player.current_melody);
	if (!(tick_counter % (SAMPLE_RATE / 100))) {
		if (sound_player.msec_left_current_note > 0) {
			if (sound_player.msec_left_current_note - 10 <= 0) {
				if (current_melody->current_note < current_melody->length) {
					++current_melody->current_note;
					uint16_t next_note_idx = current_melody->current_note;
					uint16_t next_note_length = current_melody->notes[next_note_idx].length;
					sound_player.msec_left_current_note = next_note_length;
				} else {
					set_current_melody(&sound_player, empty_melody);
				}
			} else {
				sound_player.msec_left_current_note -= 10;
			}
		}
	}

	uint16_t next_treble_note_frequency = current_melody->notes[current_melody->current_note].frequency;
	uint16_t samples_per_period_treble = SAMPLE_RATE / next_treble_note_frequency;
	uint16_t samples_per_period_bass = SAMPLE_RATE / current_bass_note;

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

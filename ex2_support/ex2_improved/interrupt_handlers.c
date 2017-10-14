#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "button.h"
#include "efm32gg.h"
#include "led.h"
#include "melody.h"

// we don't care whether the interrupt is odd or even, just call this
void GPIO_IRQHandler()
{
	// we need the player to possible change the current melody
	extern struct player sound_player;

	// fetch which buttons are pressed at interrupt-time
	uint32_t buttons_pressed = get_buttons_pressed();

	switch (buttons_pressed) {
	case SW1:
		set_current_melody(&sound_player, windows_xp_startup_melody);
		break;
	case SW2:
		set_current_melody(&sound_player, mario_game_over_melody);
		break;
	case SW3:
		set_current_melody(&sound_player, mario_1up_melody);
		break;
	case SW4:
		set_current_melody(&sound_player, mario_power_up_melody);
		break;
	case SW5:
		set_current_melody(&sound_player, laser_shot_melody);
		break;
	case SW6:
		set_current_melody(&sound_player, explosion_melody);
		break;
	case SW7:
		break;
	case SW8:
		break;
	default:
		break;
	}

	// clear interrupt
	*GPIO_IFC = *GPIO_IF;
}

void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	// clear the interrupt
	*TIMER1_IFC = 0x1;

	// sound player needed
	extern struct player sound_player;
	extern struct melody empty_melody;

	// fetch current melody of the player for readability
	struct melody *current_melody = &(sound_player.current_melody);

	// flags for toggling high/low for square wave
	extern _Bool square_high_treble;
	extern _Bool square_high_bass;

	// we tick regardless of whether music is actually played
	extern uint16_t tick_counter;
	++tick_counter;

	// toggle led every second to see all is good for debugging
	// purposes
	if (tick_counter == SAMPLE_RATE) {
		tick_counter = 0;
		toggle_led(ALL);
	}

	// if there are no treble notes to play, stop
	if (!(sound_player.current_melody.treble_notes)) {
		return;
	}

	// here we assume that treble and bass are the same lengths,
	// with same length notes, which means pauses must be added if
	// nothing is to be played in one of them
	// TODO: separate counters for each?

	// we count msec every 10th msec (1/100th of the sample rate),
	// as it is more precise than every ms wen the sample rate is
	// 44100, which we are most likely using
	if (!(tick_counter % (SAMPLE_RATE / 100))) {
		if (sound_player.msec_left_current_note - 10 <= 0) {
			// here we need to check whether the note we
			// just played was the last note of the melody
			++(current_melody->current_note);
			if (current_melody->current_note < current_melody->length) {
				uint16_t next_note_idx = current_melody->current_note;
				uint16_t next_note_length = current_melody->treble_notes[next_note_idx].length;
				sound_player.msec_left_current_note = next_note_length;
			} else {
				set_current_melody(&sound_player, empty_melody);
			}
		} else {
			sound_player.msec_left_current_note -= 10;
		}
	}

	uint16_t next_treble_note_frequency = current_melody->treble_notes[current_melody->current_note].frequency;
	uint16_t next_bass_note_frequency = current_melody->bass_notes[current_melody->current_note].frequency;
	uint16_t samples_per_period_treble = SAMPLE_RATE / next_treble_note_frequency;
	uint16_t samples_per_period_bass = SAMPLE_RATE / next_bass_note_frequency;

	if (!(tick_counter % (samples_per_period_treble / 2))) {
		square_high_treble = !square_high_treble;
	}

	if (!(tick_counter % (samples_per_period_bass / 2))) {
		square_high_bass = !square_high_bass;
	}

	*DAC0_CH0DATA = square_high_treble ? MAX_AMPLITUDE : 0x000;
	*DAC0_CH1DATA = square_high_bass ? MAX_AMPLITUDE : 0x000;
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

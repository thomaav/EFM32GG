#include <stdint.h>
#include <stdbool.h>

#include "audio.h"

struct melody create_melody(uint16_t *notes, int16_t *note_lengths, uint16_t length)
{
	struct melody new_melody;

	new_melody.notes = lisa_notes;
	new_melody.note_lengths = lisa_note_lengths;
	new_melody.length = lisa_length;

	new_melody.current_note_idx = 0;
	new_melody.current_note_length_idx = 0;
	new_melody.msec_left = new_melody.note_lengths[0];

	return new_melody;
}

uint16_t lisa_notes[22] = {
	C4, D4, E4, F4,
	G4, G4,
	A4, A4, A4, A4,
	G4,
	F4, F4, F4, F4,
	E4, E4,
	D4, D4, D4, D4,
	C4
};

int16_t lisa_note_lengths[22] = {
	FOURTH, FOURTH, FOURTH, FOURTH,
	HALF, HALF,
	FOURTH, FOURTH, FOURTH, FOURTH,
	WHOLE,
	FOURTH, FOURTH, FOURTH, FOURTH,
	HALF, HALF,
	FOURTH, FOURTH, FOURTH, FOURTH,
	WHOLE
};

uint16_t lisa_length = 22;

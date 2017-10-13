#include <stdint.h>
#include <stdbool.h>

#include "audio.h"

struct melody create_melody(uint16_t *notes, int16_t *note_lengths, uint16_t length)
{
	struct melody new_melody;

	new_melody.notes = notes;
	new_melody.note_lengths = note_lengths;
	new_melody.length = length;

	new_melody.current_note_idx = 0;
	new_melody.current_note_length_idx = 0;
	new_melody.msec_left = new_melody.note_lengths[0];

	return new_melody;
}

// notes for lisa gikk til skolen
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

// notes for the windows XP startup sound
uint16_t windows_xp_startup_notes[7] = {
	Eb5, Eb4, Bb4, PAUSE, Ab4, Eb5, Bb4
};
int16_t windows_xp_startup_note_lengths[7] = {
	EIGTH, SIXTEENTH, SIXTEENTH, SIXTEENTH,
	EIGTH + SIXTEENTH, EIGTH, FOURTH + EIGTH
};
uint16_t windows_xp_startup_length = 7;

// notes for game over in mario
uint16_t mario_game_over_notes[15] = {
	C4, PAUSE, G3, PAUSE, E3,
	A3, B3, A3, Ab3, Bb3, Ab3,
	G3, D3, E3, E3
};
int16_t mario_game_over_note_lengths[15] = {
	FOURTH, EIGTH, EIGTH, FOURTH, FOURTH,
	HALF / TRIPLET, HALF / TRIPLET, HALF / TRIPLET,
	HALF / TRIPLET, HALF / TRIPLET, HALF / TRIPLET,
	SIXTEENTH, SIXTEENTH, EIGTH, HALF
};
uint16_t mario_game_over_length = 15;

// mario 1up sound
uint16_t mario_1up_notes[6] = {
	E4, G4, E5, C5, D5, G5
};
int16_t mario_1up_note_lengths[6] = {
	SIXTEENTH, SIXTEENTH, SIXTEENTH,
	SIXTEENTH, SIXTEENTH, SIXTEENTH
};
uint16_t mario_1up_length = 6;

// laser for space invaders
uint16_t laser_shot_notes[9] = {
	C5, Db5, D5, Eb5, E5, Eb5, D5, Db5, C5
};
int16_t laser_shot_note_lengths[9] = {
	10, 10, 10, 10, 10, 10, 10, 10, 10
};
uint16_t laser_shot_length = 9;

// explosion space invaders
uint16_t explosion_notes[18] = {
	C2, Db2, D2, Eb2, E2, Eb2, D2, Db2, C2,
	C2, Db2, D2, Eb2, E2, Eb2, D2, Db2, C2
};
int16_t explosion_note_lengths[18] = {
	10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10
};
uint16_t explosion_length = 18;

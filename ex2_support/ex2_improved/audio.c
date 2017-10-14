#include <stdint.h>
#include <stdbool.h>

#include "audio.h"

// create a melody from treble and bass notes, and return it do not
// allocate the melody on the heap and return a pointer as one would
// usually do, as we had some trouble with linking when we used malloc
struct melody create_melody(struct note *treble_notes, struct note *bass_notes, uint16_t length)
{
	struct melody new_melody;

	new_melody.treble_notes = treble_notes;
	new_melody.bass_notes = bass_notes;
	new_melody.length = length;
	new_melody.current_note = 0;

	return new_melody;
}

// set the current melody of the player pointed to to be a new melody
void set_current_melody(struct player *sound_player, struct melody melody)
{
	sound_player->current_melody = melody;
	sound_player->msec_left_current_note = melody.treble_notes[0].length;
}

// these are constant, use them in the global scope, and initialize
// melodies in setup_melodies
struct note windows_xp_startup_melody_treble_notes[7] = {
	{ .frequency = Eb5, .length = EIGTH },
	{ .frequency = Eb4, .length = SIXTEENTH },
	{ .frequency = Bb4, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = Ab4, .length = EIGTH + SIXTEENTH },
	{ .frequency = Eb5, .length = EIGTH },
	{ .frequency = Bb4, .length = FOURTH + EIGTH }
};

struct note windows_xp_startup_melody_bass_notes[7] = {
	{ .frequency = Eb4, .length = EIGTH },
	{ .frequency = Eb3, .length = SIXTEENTH },
	{ .frequency = Bb3, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = Ab3, .length = EIGTH + SIXTEENTH },
	{ .frequency = Eb4, .length = EIGTH },
	{ .frequency = Bb3, .length = FOURTH + EIGTH }
};

struct note mario_game_over_melody_treble_notes[15] = {
	{ .frequency = C4, .length = FOURTH },
	{ .frequency = PAUSE, .length = EIGTH },
	{ .frequency = G3, .length = EIGTH },
	{ .frequency = PAUSE, .length = FOURTH },
	{ .frequency = E3, .length = FOURTH },

	{ .frequency = A3, .length = HALF / TRIPLET },
	{ .frequency = B3, .length = HALF / TRIPLET },
	{ .frequency = A3, .length = HALF / TRIPLET },
	{ .frequency = Ab3, .length = HALF / TRIPLET },
	{ .frequency = Bb3, .length = HALF / TRIPLET },
	{ .frequency = Ab3, .length = HALF / TRIPLET },

	{ .frequency = G3, .length = SIXTEENTH },
	{ .frequency = D3, .length = SIXTEENTH },
	{ .frequency = E3, .length = EIGTH },
	{ .frequency = E3, .length = HALF }
};

struct note mario_game_over_melody_bass_notes[15] = {
	{ .frequency = E3, .length = FOURTH },
	{ .frequency = PAUSE, .length = EIGTH },
	{ .frequency = C3, .length = EIGTH },
	{ .frequency = PAUSE, .length = FOURTH },
	{ .frequency = PAUSE, .length = FOURTH },

	{ .frequency = F2, .length = HALF / TRIPLET },
	{ .frequency = F2, .length = HALF / TRIPLET },
	{ .frequency = F2, .length = HALF / TRIPLET },
	{ .frequency = Db2, .length = HALF / TRIPLET },
	{ .frequency = Db2, .length = HALF / TRIPLET },
	{ .frequency = Db2, .length = HALF / TRIPLET },

	{ .frequency = C2, .length = SIXTEENTH },
	{ .frequency = C2, .length = SIXTEENTH },
	{ .frequency = C2, .length = EIGTH },
	{ .frequency = C2, .length = HALF }
};

struct note mario_1up_melody_treble_notes[6] = {
	{ .frequency = E4, .length = SIXTEENTH },
	{ .frequency = G4, .length = SIXTEENTH },
	{ .frequency = E5, .length = SIXTEENTH },
	{ .frequency = C5, .length = SIXTEENTH },
	{ .frequency = D5, .length = SIXTEENTH },
	{ .frequency = G5, .length = SIXTEENTH },
};

struct note mario_1up_melody_bass_notes[6] = {
	{ .frequency = E4, .length = SIXTEENTH },
	{ .frequency = G4, .length = SIXTEENTH },
	{ .frequency = E5, .length = SIXTEENTH },
	{ .frequency = C5, .length = SIXTEENTH },
	{ .frequency = D5, .length = SIXTEENTH },
	{ .frequency = G5, .length = SIXTEENTH },
};

struct note mario_power_up_melody_treble_notes[15] = {
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = D4, .length = SIXTEENTH },
	{ .frequency = G4, .length = SIXTEENTH },
	{ .frequency = B4, .length = SIXTEENTH },

	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = Eb4, .length = SIXTEENTH },
	{ .frequency = Ab4, .length = SIXTEENTH },
	{ .frequency = C5, .length = SIXTEENTH },

	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = F4, .length = SIXTEENTH },
	{ .frequency = Bb4, .length = SIXTEENTH },
	{ .frequency = D5, .length = SIXTEENTH },
};

struct note mario_power_up_melody_bass_notes[15] = {
	{ .frequency = G3, .length = SIXTEENTH },
	{ .frequency = B3, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },

	{ .frequency = Ab3, .length = SIXTEENTH },
	{ .frequency = C4, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },

	{ .frequency = Bb3, .length = SIXTEENTH },
	{ .frequency = D4, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
};

struct note laser_shot_melody_treble_notes[9] = {
	{ .frequency = C5, .length = 10 },
	{ .frequency = Db5, .length = 10 },
	{ .frequency = D5, .length = 10 },
	{ .frequency = Eb5, .length = 10 },
	{ .frequency = E5, .length = 10 },
	{ .frequency = Eb5, .length = 10 },
	{ .frequency = D5, .length = 10 },
	{ .frequency = Db5, .length = 10 },
	{ .frequency = C5, .length = 10 },
};

struct note laser_shot_melody_bass_notes[9] = {
	{ .frequency = C4, .length = 10 },
	{ .frequency = Db4, .length = 10 },
	{ .frequency = D4, .length = 10 },
	{ .frequency = Eb4, .length = 10 },
	{ .frequency = E4, .length = 10 },
	{ .frequency = Eb4, .length = 10 },
	{ .frequency = D4, .length = 10 },
	{ .frequency = Db4, .length = 10 },
	{ .frequency = C4, .length = 10 },
};

struct note explosion_melody_treble_notes[18] = {
	{ .frequency = C2, .length = 10 },
	{ .frequency = Db2, .length = 10 },
	{ .frequency = D2, .length = 10 },
	{ .frequency = Eb2, .length = 10 },
	{ .frequency = E2, .length = 10 },
	{ .frequency = Eb2, .length = 10 },
	{ .frequency = D2, .length = 10 },
	{ .frequency = Db2, .length = 10 },
	{ .frequency = C2, .length = 10 },

	{ .frequency = C2, .length = 10 },
	{ .frequency = Db2, .length = 10 },
	{ .frequency = D2, .length = 10 },
	{ .frequency = Eb2, .length = 10 },
	{ .frequency = E2, .length = 10 },
	{ .frequency = Eb2, .length = 10 },
	{ .frequency = D2, .length = 10 },
	{ .frequency = Db2, .length = 10 },
	{ .frequency = C2, .length = 10 },
};

struct note explosion_melody_bass_notes[18] = {
	{ .frequency = C1, .length = 10 },
	{ .frequency = Db1, .length = 10 },
	{ .frequency = D1, .length = 10 },
	{ .frequency = Eb1, .length = 10 },
	{ .frequency = E1, .length = 10 },
	{ .frequency = Eb1, .length = 10 },
	{ .frequency = D1, .length = 10 },
	{ .frequency = Db1, .length = 10 },
	{ .frequency = C1, .length = 10 },

	{ .frequency = C1, .length = 10 },
	{ .frequency = Db1, .length = 10 },
	{ .frequency = D1, .length = 10 },
	{ .frequency = Eb1, .length = 10 },
	{ .frequency = E1, .length = 10 },
	{ .frequency = Eb1, .length = 10 },
	{ .frequency = D1, .length = 10 },
	{ .frequency = Db1, .length = 10 },
	{ .frequency = C1, .length = 10 },
};

// bind melodies
void setup_melodies()
{
	windows_xp_startup_melody = create_melody(windows_xp_startup_melody_treble_notes,
						  windows_xp_startup_melody_bass_notes,
						  7);
	mario_game_over_melody = create_melody(mario_game_over_melody_treble_notes,
					       mario_game_over_melody_bass_notes,
					       15);
	mario_1up_melody = create_melody(mario_1up_melody_treble_notes,
					 mario_1up_melody_bass_notes,
					 6);
	mario_power_up_melody = create_melody(mario_power_up_melody_treble_notes,
					      mario_power_up_melody_bass_notes,
					      15);
	laser_shot_melody = create_melody(laser_shot_melody_treble_notes,
					  laser_shot_melody_bass_notes,
					  9);
	explosion_melody = create_melody(explosion_melody_treble_notes,
					 explosion_melody_bass_notes,
					 18);
}

// what we used before is below, kept for the sake of the report
// this used blocking, which was not as nice

/* void _play_sounds(uint16_t treble_note, uint16_t bass_note, uint16_t msec, _Bool legato) */
/* { */
/* 	// add BREATH between notes by playing nothing the last BREATH */
/* 	// ms of the sound */
/* 	if (!legato) { */
/* 		msec -= BREATH; */
/* 	} */

/* 	current_treble_note = treble_note; */
/* 	current_bass_note = bass_note; */
/* 	msec_left = msec; */

/* 	// block until the interrupts have finished playing */
/* 	while (msec_left) { */
/* 		__asm__("wfi"); */
/* 	} */

/* 	// if we don't want legato, add a pause of length BREATH, */
/* 	// don't change bass */
/* 	if (!legato) { */
/* 		msec_left = BREATH; */
/* 		current_treble_note = 0; */

/* 		while (msec_left) { */
/* 			__asm__("wfi"); */
/* 		} */
/* 	} */

/* 	return; */
/* } */

/* void play_sounds(uint16_t treble_note, uint16_t bass_note, uint16_t msec) */
/* { */
/* 	_play_sounds(treble_note, bass_note, msec, false); */
/* } */

/* void _play_sound(uint16_t note, uint16_t msec, _Bool legato) */
/* { */
/* 	// add BREATH between notes by playing nothing the last BREATH */
/* 	// ms of the sound */
/* 	if (!legato) { */
/* 		msec -= BREATH; */
/* 	} */

/* 	current_treble_note = note; */
/* 	msec_left = msec; */

/* 	// block until the interrupts have finished playing */
/* 	while (msec_left) { */
/* 		__asm__("wfi"); */
/* 	} */

/* 	// if we don't want legato, add a pause of length BREATH */
/* 	if (!legato) { */
/* 		msec_left = BREATH; */
/* 		current_treble_note = 0; */

/* 		while (msec_left) { */
/* 			__asm__("wfi"); */
/* 		} */
/* 	} */

/* 	return; */
/* } */

/* void play_sound(uint16_t note, uint16_t msec) */
/* { */
/* 	_play_sound(note, msec, false); */
/* } */


/* void lisa_gikk_til_skolen() */
/* { */
/* 	play_sounds(C4, C3, FOURTH); */
/* 	play_sounds(D4, C3, FOURTH); */
/* 	play_sounds(E4, C3, FOURTH); */
/* 	play_sounds(F4, C3, FOURTH); */

/* 	play_sounds(G4, C3, HALF); */
/* 	play_sounds(G4, C3, HALF); */

/* 	play_sounds(A4, F3, FOURTH); */
/* 	play_sounds(A4, F3, FOURTH); */
/* 	play_sounds(A4, F3, FOURTH); */
/* 	play_sounds(A4, F3, FOURTH); */

/* 	play_sounds(G4, C3, WHOLE); */

/* 	play_sounds(F4, G3, FOURTH); */
/* 	play_sounds(F4, G3, FOURTH); */
/* 	play_sounds(F4, G3, FOURTH); */
/* 	play_sounds(F4, G3, FOURTH); */

/* 	play_sounds(E4, C3, HALF); */
/* 	play_sounds(E4, C3, HALF); */

/* 	play_sounds(D4, G3, FOURTH); */
/* 	play_sounds(D4, G3, FOURTH); */
/* 	play_sounds(D4, G3, FOURTH); */
/* 	play_sounds(D4, G3, FOURTH); */

/* 	play_sounds(C4, C3, WHOLE); */
/* } */

/* void mario_game_over() */
/* { */
/* 	play_sounds(C4, E3, FOURTH); */
/* 	play_sounds(PAUSE, PAUSE, EIGTH); */
/* 	play_sounds(G3, C3, EIGTH); */
/* 	play_sounds(PAUSE, PAUSE, FOURTH); */
/* 	play_sounds(E3, PAUSE, FOURTH); */

/* 	play_sounds(A3, F2, HALF / TRIPLET); */
/* 	play_sounds(B3, F2, HALF / TRIPLET); */
/* 	play_sounds(A3, F2, HALF / TRIPLET); */
/* 	play_sounds(Ab3, Db2, HALF / TRIPLET); */
/* 	play_sounds(Bb3, Db2, HALF / TRIPLET); */
/* 	play_sounds(Ab3, Db2, HALF / TRIPLET); */

/* 	play_sounds(G3, C2, SIXTEENTH); */
/* 	_play_sounds(D3, C2, SIXTEENTH, true); */
/* 	_play_sounds(E3, C2, EIGTH, true); */
/* 	_play_sounds(E3, C2, HALF, true); */
/* } */

/* void mario_1up() */
/* { */
/* 	play_sounds(E4, E4, SIXTEENTH); */
/* 	play_sounds(G4, G4, SIXTEENTH); */
/* 	play_sounds(E5, E5, SIXTEENTH); */
/* 	play_sounds(C5, C5, SIXTEENTH); */
/* 	play_sounds(D5, D5, SIXTEENTH); */
/* 	play_sounds(G5, G5, SIXTEENTH); */
/* } */

/* void mario_power_up() */
/* { */
/* 	play_sounds(PAUSE, G3, SIXTEENTH); */
/* 	play_sounds(PAUSE, B3, SIXTEENTH); */
/* 	play_sounds(D4, PAUSE, SIXTEENTH); */
/* 	play_sounds(G4, PAUSE, SIXTEENTH); */
/* 	play_sounds(B4, PAUSE, SIXTEENTH); */

/* 	play_sounds(PAUSE, Ab3, SIXTEENTH); */
/* 	play_sounds(PAUSE, C4, SIXTEENTH); */
/* 	play_sounds(Eb4, PAUSE, SIXTEENTH); */
/* 	play_sounds(Ab4, PAUSE, SIXTEENTH); */
/* 	play_sounds(C5, PAUSE, SIXTEENTH); */

/* 	play_sounds(PAUSE, Bb3, SIXTEENTH); */
/* 	play_sounds(PAUSE, D4, SIXTEENTH); */
/* 	play_sounds(F4, PAUSE, SIXTEENTH); */
/* 	play_sounds(Bb4, PAUSE, SIXTEENTH); */
/* 	play_sounds(D5, PAUSE, SIXTEENTH); */
/* } */

/* void windows_xp_startup() */
/* { */
/* 	play_sounds(Eb5, Eb4, EIGTH); */
/* 	play_sounds(Eb4, Eb3, SIXTEENTH); */
/* 	play_sounds(Bb4, Bb3, SIXTEENTH); */
/* 	play_sounds(PAUSE, PAUSE, SIXTEENTH); */
/* 	play_sounds(Ab4, Ab3, EIGTH + SIXTEENTH); */
/* 	play_sounds(Eb5, Eb4, EIGTH); */
/* 	play_sounds(Bb4, Bb3, FOURTH + EIGTH); */
/* } */

/* void laser_shot() */
/* { */
/* 	_play_sounds(C5, C4, 10, true); */
/* 	_play_sounds(Db5, Db4, 10, true); */
/* 	_play_sounds(D5, D4, 10, true); */
/* 	_play_sounds(Eb5, Eb4, 10, true); */
/* 	_play_sounds(E5, E4, 10, true); */
/* 	_play_sounds(Eb5, Eb4, 10, true); */
/* 	_play_sounds(D5, D4, 10, true); */
/* 	_play_sounds(Db5, Db4, 10, true); */
/* 	_play_sounds(C5, C4, 10, true); */
/* } */

/* void explosion() */
/* { */
/* 	// play laser shot sound three octaves lower, twice */
/* 	for (uint8_t i = 0; i < 2; ++i) { */
/* 		_play_sounds(C2, C1, 10, true); */
/* 		_play_sounds(Db2, Db1, 10, true); */
/* 		_play_sounds(D2, D1, 10, true); */
/* 		_play_sounds(Eb2, Eb1, 10, true); */
/* 		_play_sounds(E2, E1, 10, true); */
/* 		_play_sounds(Eb2, Eb1, 10, true); */
/* 		_play_sounds(D2, D1, 10, true); */
/* 		_play_sounds(Db2, Db1, 10, true); */
/* 		_play_sounds(C2, C1, 10, true); */
/* 	} */
/* } */

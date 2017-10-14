#include <stdbool.h>
#include <stdint.h>

#include "audio.h"
#include "melody.h"

// these are constant, use them in the global scope, and initialize
// melodies in setup_melodies
struct note windows_xp_startup_melody_treble_notes[7] = {
	{ .frequency = Eb5, .length = EIGHTH },
	{ .frequency = Eb4, .length = SIXTEENTH },
	{ .frequency = Bb4, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = Ab4, .length = EIGHTH + SIXTEENTH },
	{ .frequency = Eb5, .length = EIGHTH },
	{ .frequency = Bb4, .length = FOURTH + EIGHTH }
};

struct note windows_xp_startup_melody_bass_notes[7] = {
	{ .frequency = Eb4, .length = EIGHTH },
	{ .frequency = Eb3, .length = SIXTEENTH },
	{ .frequency = Bb3, .length = SIXTEENTH },
	{ .frequency = PAUSE, .length = SIXTEENTH },
	{ .frequency = Ab3, .length = EIGHTH + SIXTEENTH },
	{ .frequency = Eb4, .length = EIGHTH },
	{ .frequency = Bb3, .length = FOURTH + EIGHTH }
};

struct note mario_game_over_melody_treble_notes[15] = {
	{ .frequency = C4, .length = FOURTH },
	{ .frequency = PAUSE, .length = EIGHTH },
	{ .frequency = G3, .length = EIGHTH },
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
	{ .frequency = E3, .length = EIGHTH },
	{ .frequency = E3, .length = HALF }
};

struct note mario_game_over_melody_bass_notes[15] = {
	{ .frequency = E3, .length = FOURTH },
	{ .frequency = PAUSE, .length = EIGHTH },
	{ .frequency = C3, .length = EIGHTH },
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
	{ .frequency = C2, .length = EIGHTH },
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

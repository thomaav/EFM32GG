#define SAMPLE_RATE 44100
#define BPM 120
#define BPS (BPM / 60)
#define BREATH 10

#define WHOLE (HALF * 2)
#define HALF (FOURTH * 2)
#define FOURTH (1000 / BPS)
#define EIGHTH (FOURTH / 2)
#define SIXTEENTH (EIGHTH / 2)
#define THIRTYSECOND (SIXTEENTH / 2)
#define TRIPLET 3
#define PAUSE 0

#define C1   32
#define Db1  34
#define D1   36
#define Eb1  38
#define E1   41
#define F1   43
#define Gb1  46
#define G1   49
#define Ab1  51
#define A1   55
#define Bb1  58
#define B1   61

#define C2   65
#define Db2  69
#define D2   73
#define Eb2  77
#define E2   82
#define F2   87
#define Gb2  92
#define G2   98
#define Ab2 103
#define A2  110
#define Bb2 116
#define B2  123

#define C3  130
#define Db3 138
#define D3  146
#define Eb3 155
#define E3  164
#define F3  174
#define Gb3 185
#define G3  196
#define Ab3 207
#define A3  220
#define Bb3 233
#define B3  246

#define C4  261
#define Db4 277
#define D4  293
#define Eb4 311
#define E4  329
#define F4  349
#define Gb4 369
#define G4  392
#define Ab4 415
#define A4  440
#define Bb4 466
#define B4  493

#define C5  523
#define Db5 554
#define D5  587
#define Eb5 622
#define E5  659
#define F5  698
#define Gb5 739
#define G5  783
#define Ab5 830
#define A5  880
#define Bb5 932
#define B5  987

// songs
struct melody
{
	// meta
	uint16_t *notes;
	int16_t *note_lengths;
	uint16_t length;

	// current playthrough
	uint16_t current_note_idx;
	uint16_t current_note_length_idx;
	int16_t msec_left;
};

// simple way to instantiate struct melody
struct melody create_melody(uint16_t *notes, int16_t *note_lengths, uint16_t length);

// songs/melodies
uint16_t lisa_notes[22];
int16_t lisa_note_lengths[22];
uint16_t lisa_length;

uint16_t windows_xp_startup_notes[7];
int16_t windows_xp_startup_note_lengths[7];
uint16_t windows_xp_startup_length;

uint16_t mario_game_over_notes[15];
int16_t mario_game_over_note_lengths[15];
uint16_t mario_game_over_length;

// mario 1up sound
uint16_t mario_1up_notes[6];
int16_t mario_1up_note_lengths[6];
uint16_t mario_1up_length;

uint16_t laser_shot_notes[9];
int16_t laser_shot_note_lengths[9];
uint16_t laser_shot_length;

uint16_t explosion_notes[18];
int16_t explosion_note_lengths[18];
uint16_t explosion_length;

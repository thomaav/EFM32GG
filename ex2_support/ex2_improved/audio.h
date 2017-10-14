#define SAMPLE_RATE 44100
#define BPM 120
#define BPS (BPM / 60)
#define BREATH 10

#define WHOLE (HALF * 2)
#define HALF (FOURTH * 2)
#define FOURTH (1000 / BPS)
#define EIGTH (FOURTH / 2)
#define SIXTEENTH (EIGTH / 2)
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

struct note
{
	// meta
	uint16_t frequency;
	int16_t length;
};

struct melody
{
	// meta
	struct note *treble_notes;
	struct note *bass_notes;
	uint16_t length;

	// current playthrough
	uint16_t current_note;
};

struct player
{
	struct melody current_melody;
	int16_t msec_left_current_note;
};

struct melody create_melody(struct note *treble_notes, struct note *bass_notes, uint16_t length);
void set_current_melody(struct player *sound_player, struct melody melody);

void setup_melodies();

// songs / melodies
struct note windows_xp_startup_melody_treble_notes[7];
struct note windows_xp_startup_melody_bass_notes[7];
struct melody windows_xp_startup_melody;

// what we used before is below, kept for the sake of the report

/* void _play_sounds(uint16_t treble_note, uint16_t bass_note, uint16_t msec, _Bool legato); */
/* void play_sounds(uint16_t treble_note, uint16_t bass_note, uint16_t msec); */
/* void _play_sound(uint16_t note, uint16_t msec, _Bool legato); */
/* void play_sound(uint16_t note, uint16_t msec); */

// sounds
/* void lisa_gikk_til_skolen(); */
/* void mario_game_over(); */
/* void mario_1up(); */
/* void mario_power_up(); */
/* void windows_xp_startup(); */
/* void laser_shot(); */
/* void explosion(); */

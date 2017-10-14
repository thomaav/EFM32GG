// we set BPM statically, as using a dynamic BPM for each melody we
// create would require more work to keep the precision of our
// counting acceptable
#define SAMPLE_RATE 44100
#define BPM 120
#define BPS (BPM / 60)
#define BREATH 10 // used if legato is not wanted
#define MAX_AMPLITUDE 0xF

// definition for note durations based on our BPM
#define WHOLE (HALF * 2)
#define HALF (FOURTH * 2)
#define FOURTH (1000 / BPS)
#define EIGHTH (FOURTH / 2)
#define SIXTEENTH (EIGHTH / 2)
#define THIRTYSECOND (SIXTEENTH / 2)
#define TRIPLET 3 // a triplet divides any single note into three equal ones
#define PAUSE 0

// given frequencies for common notes
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

// base struct for a note
struct note
{
	uint16_t frequency;
	int16_t length;
};

// a melody keeps arrays of treble and bass notes for two channels
// (left, right), however, this requires that they are the same
// length, so keep this in mind when creating melodies
struct melody
{
	// meta
	struct note *treble_notes;
	struct note *bass_notes;
	uint16_t length;

	// current playthrough
	uint16_t current_note;
};

// we want a player in our main program that keeps track of what is
// the current melody of of the program, and how much is left of the
// note that we are currently playing
struct player
{
	struct melody current_melody;
	int16_t msec_left_current_note;
};

struct melody create_melody(struct note *treble_notes, struct note *bass_notes, uint16_t length);
void set_current_melody(struct player *sound_player, struct melody melody);

// setup melodies is needed to actually bind our melodies correctly at
// runtime, as structs cannot be usted for const initiation in C
void setup_melodies();

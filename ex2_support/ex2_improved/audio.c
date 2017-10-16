#include <stdint.h>
#include <stdbool.h>

#include "audio.h"
#include "melody.h"

// create a melody from treble and bass notes, and return it do not
// allocate the melody on the heap and return a pointer as one would
// usually do, as we had some trouble with linking when we used malloc
struct melody
create_melody(struct note *treble_notes, struct note *bass_notes,
	      uint16_t length)
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

// bind melodies
void setup_melodies()
{
	windows_xp_startup_melody =
	    create_melody(windows_xp_startup_melody_treble_notes,
			  windows_xp_startup_melody_bass_notes, 7);
	mario_game_over_melody =
	    create_melody(mario_game_over_melody_treble_notes,
			  mario_game_over_melody_bass_notes, 15);
	mario_1up_melody =
	    create_melody(mario_1up_melody_treble_notes,
			  mario_1up_melody_bass_notes, 6);
	mario_power_up_melody =
	    create_melody(mario_power_up_melody_treble_notes,
			  mario_power_up_melody_bass_notes, 15);
	laser_shot_melody =
	    create_melody(laser_shot_melody_treble_notes,
			  laser_shot_melody_bass_notes, 9);
	explosion_melody =
	    create_melody(explosion_melody_treble_notes,
			  explosion_melody_bass_notes, 18);
}

// needed to sleep CPU when no music is played
void disable_timer();
void enable_timer();
void enable_DAC();
void disable_DAC();

void turn_off_music_peripherals()
{
	*SCR = 0x6;
	disable_timer();
	disable_DAC();
}

void turn_on_music_peripherals()
{
	*SCR = 0x2;
	enable_timer();
	enable_DAC();
}

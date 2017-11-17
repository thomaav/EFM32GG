#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "framebuffer.h"
#include "gamepad.h"
#include "signal.h"
#include "tetris.h"
#include "util.h"

// cheat to sleep $level length in main loop
extern struct player player;

int main(int argc, char *argv[])
{
	// reseed every run
	srand(time(NULL));

	// setup framebuffer
	if (setup_screen() == -1) {
		printf("setup_screen() failed.\n");
		exit(EXIT_FAILURE);
	}

	// setup to read gamepad
	if (gp_init()) {
		printf("Could not initialize gamepad. Quitting.\n");
		exit(EXIT_FAILURE);
	}

	// register async notification on SIGIO with /dev/gamepad
	register_SIGIO(gpfd, gp_handler);

	initiate_tetris();

	for (;;) {
		// wrap this code in a mutex to deferr signal handling
		// (we know we are not spawning any more threads), so
		// this is a very simple way to do it without
		// p_thread. if we don't do this, getting a signal
		// during a tick would lead to some weird hiccups in
		// the movement at seemingly random times
		gp_mutex = 1;
		tick_tetris_and_blit();
		gp_mutex = 0;

		if (sigio_exec_deferred) {
			sigio_exec_deferred = 0;
			gp_handler(SIGIO);
		}

		// yes - we do need the rem pointer here as well, as
		// nanosleep expects a pointer to a const struct, and
		// a non-const struct (it writes the remaining time
		// back into &rem)
		//
		// remove 100 msec for every level (highest level is 9
		// for 900 msec)
		__mssleep(1000 - 100 * player.level);
	}

	if (teardown_screen() == -1) {
		printf("teardown_screen() failed.\n");
		return -1;
	}

	exit(EXIT_SUCCESS);
}

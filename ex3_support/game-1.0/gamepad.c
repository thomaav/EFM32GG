#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gamepad.h"

int gpfd;
uint8_t gp_state;

bool tetris_tick_mutex;
bool sigio_exec_deferred;
uint8_t gp_deferred_state;

void (*gp_state_handler)(uint8_t);

int gp_init()
{
	gpfd = open("/dev/gamepad", O_RDWR);
	if (gpfd == -1) {
		printf("open() for gpfd failed with error [%s].\n", strerror(errno));
		return -1;
	}

	return 0;
}

void gp_handler(int sig)
{
	// we need to do this even if we are deferring, as we need to
	// know what the state of the button was
	if (read(gpfd, &gp_state, GPBUF_SIZE) < 0) {
		printf("read() for gpfd failed with error [%s].\n", strerror(errno));
		return;
	}

	// sometimes we get an interrupt without the buttons having
	// been pushed (unclear why) - ignore these
	if (!gp_state)
		return;

	// if we are locked out of executing and getting deferred, set
	// the deferred state as well
	if (tetris_tick_mutex) {
		sigio_exec_deferred = 1;
		gp_deferred_state = gp_state;
		return;
	}

	// are we handling a deferred handling?
	if (gp_deferred_state) {
		gp_state = gp_deferred_state;
		gp_deferred_state = 0;
	}

	if (gp_state_handler) {
		(*gp_state_handler)(gp_state);
	}
}

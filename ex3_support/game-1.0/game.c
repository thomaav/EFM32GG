#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "framebuffer.h"

#define GPBUF_SIZE 1

uint8_t gp_state;
int gpfd;

uint16_t black;
uint16_t white;
uint16_t red;
uint16_t green;
uint16_t blue;

#define TILE_SIZE 10
#define GAME_WIDTH (WIDTH / TILE_SIZE)
#define GAME_HEIGHT (HEIGHT / TILE_SIZE)

uint16_t player_x;
uint16_t player_y;

void gp_handler(int sig)
{
	printf("Inside gp_handler.\n");

	if (read(gpfd, &gp_state, GPBUF_SIZE) < 0) {
		printf("read() for gpfd failed with error [%s].\n", strerror(errno));
		return;
	}

	printf("gp_state: %d.\n", gp_state);

	if (gp_state == 1) {
		paint_screen(red);
	} else if (gp_state == 2) {
		paint_screen(green);
	} else if (gp_state == 4) {
		paint_screen(blue);
	} else {
		paint_region(green, 10, 10, 20, 20);
	}
}

int main(int argc, char *argv[])
{
	black = 0x0000;
	white = 0xFFFF;
	red = 0xF000;
	green = 0x0FF0;
	blue = 0x000F;

	// setup framebuffer
	if (setup_screen() == -1) {
		printf("setup_screen() failed.\n");
		return -1;
	}

	// setup to read gamepad
	gpfd = open("/dev/gamepad", O_RDWR);
	if (gpfd == -1) {
		printf("open() for gpfd failed with error [%s].\n", strerror(errno));
		return -1;
	}

	// register async notification on SIGIO
	struct sigaction gp_action;
	memset(&gp_action, 0, sizeof(gp_action));
	gp_action.sa_handler = gp_handler;
	gp_action.sa_flags = 0;

	sigaction(SIGIO, &gp_action, NULL);

	fcntl(gpfd, F_SETOWN, getpid());
	int oflags = fcntl(gpfd, F_GETFL);
	fcntl(gpfd, F_SETFL, oflags | FASYNC);

	// setup done, let's start playing
	uint16_t ticker = 0;

	// player
	uint16_t player_x = 50;
	uint16_t player_y = 0;

	for (;;) {
		// 16000000 = 16 000 000
		nanosleep((const struct timespec[]){{0, 16000000L}}, NULL);

		if (++ticker == 60) {
			ticker = 0;
			printf("A second has passed.\n");

			paint_region(black, player_x, player_y, TILE_SIZE, TILE_SIZE);
			player_y = (player_y + TILE_SIZE) % HEIGHT;
			paint_region(green, player_x, player_y, TILE_SIZE, TILE_SIZE);
			update_region(player_x, player_y - TILE_SIZE, TILE_SIZE, TILE_SIZE * 2);
		}
	}

	if (teardown_screen() == -1) {
		printf("teardown_screen() failed.\n");
		return -1;
	}

	exit(EXIT_SUCCESS);
}

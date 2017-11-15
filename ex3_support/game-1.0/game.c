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
#define GAME_WIDTH (WIDTH / TILE_SIZE / 2)
#define GAME_HEIGHT (HEIGHT / TILE_SIZE)

uint16_t player_x = 5;
uint16_t player_y = 0;
uint8_t current_shape[4][4];

#define SHAPE_WIDTH 4
#define SHAPE_HEIGHT 4

uint8_t moved = 0;

uint8_t shape_I[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{0, 0, 0, 0},
	{1, 1, 1, 1},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_J[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{1, 0, 0, 0},
	{1, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_L[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{0, 0, 1, 0},
	{1, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_O[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{0, 1, 1, 0},
	{0, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_S[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{0, 1, 1, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_T[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{0, 1, 0, 0},
	{1, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

uint8_t shape_Z[SHAPE_WIDTH][SHAPE_HEIGHT] = {
	{1, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

void memcpy_shape(uint8_t dst[SHAPE_WIDTH][SHAPE_HEIGHT], uint8_t shape[SHAPE_WIDTH][SHAPE_HEIGHT])
{
	int i;
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		memcpy(dst[i], shape[i], SHAPE_WIDTH);
	}
}

void rotate_shape(uint8_t shape[SHAPE_WIDTH][SHAPE_HEIGHT])
{
	// with malloc we could change where uint8_t **shape points
	// and free the old memory, but without it we need the temp
	// variable to go out of scope when this enclosure is gone
	uint8_t tmp_rshape[SHAPE_WIDTH][SHAPE_HEIGHT];
	uint8_t i, j;

	// it is also possible to do this in place for N*N matrices
	// for 90 degree rotation, but it is a bit more complex
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			tmp_rshape[(SHAPE_HEIGHT - 1) - j][(SHAPE_WIDTH - 1) - i] = shape[i][j];
		}
	}

	// copy the computed values to the shape we were given
	for (i = 0; i < SHAPE_WIDTH; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			shape[i][j] = tmp_rshape[i][j];
		}
	}

	for (i = 0; i < SHAPE_WIDTH; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			printf(" %d", shape[i][j]);
		}
		printf("\n");
	}
}

void paint_tile(uint16_t color, uint16_t x, uint16_t y)
{
	uint16_t tile_x = x * TILE_SIZE;
	uint16_t tile_y = y * TILE_SIZE;

	paint_region(color, tile_x, tile_y, TILE_SIZE, TILE_SIZE);
}

void paint_tetris_shape(uint16_t color, uint16_t x, uint16_t y, uint8_t shape[SHAPE_WIDTH][SHAPE_HEIGHT])
{
	uint16_t i, j;

	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			if (shape[i][j]) {
				paint_tile(color, x + j, y + i);
			} else {
				paint_tile(black, x + j, y + i);
			}
		}
	}

	// also paint tiles to the immediate right, left and above
	for (i = 0; i < (SHAPE_HEIGHT | SHAPE_WIDTH); ++i) {
		paint_tile(black, x - 1, y + i);
		paint_tile(black, x + SHAPE_WIDTH, y + i);
		paint_tile(black, x + i, y - 1);
	}

	update_region(x * TILE_SIZE - TILE_SIZE, y * TILE_SIZE - TILE_SIZE,
		      (SHAPE_WIDTH + 2) * TILE_SIZE, (SHAPE_HEIGHT * 2) * TILE_SIZE);
}

void gp_handler(int sig)
{
	printf("Inside gp_handler.\n");

	if (read(gpfd, &gp_state, GPBUF_SIZE) < 0) {
		printf("read() for gpfd failed with error [%s].\n", strerror(errno));
		return;
	}

	if (gp_state == 1 || gp_state == 2 || gp_state == 4)
		moved = gp_state;
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

	paint_screen(black);

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

	memcpy_shape(current_shape, shape_I);

	for (;;) {
		if (moved) {
			if (moved == 1) {
				--player_x;
			} else if (moved == 2) {
				rotate_shape(current_shape);
			} else if (moved == 4) {
				++player_x;
			}

			paint_tetris_shape(green, player_x, player_y, current_shape);

			moved = 0;
		}

		if (++ticker == 60) {
			ticker = 0;
			printf("A second has passed.\n");

			player_y = ++player_y % GAME_HEIGHT;
			paint_tetris_shape(green, player_x, player_y, current_shape);
		}

		// 16000000 = 16 000 000
		nanosleep((const struct timespec[]){{0, 16000000L}}, NULL);
	}

	if (teardown_screen() == -1) {
		printf("teardown_screen() failed.\n");
		return -1;
	}

	exit(EXIT_SUCCESS);
}

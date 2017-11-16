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
#include <stdbool.h>
#include <time.h>

#include "framebuffer.h"
#include "signal.h"

#define GPBUF_SIZE 1

#define TILE_SIZE 10
#define BORDER_WIDTH 1

#define GAME_WIDTH 10
#define GAME_HEIGHT (HEIGHT / TILE_SIZE)

#define UNIQ_SHAPES 7
#define SHAPE_WIDTH 4
#define SHAPE_HEIGHT 4

#define PLAYER_INIT_X 3
#define PLAYER_INIT_Y 0

#define BLACK   (0x0000)
#define WHITE   (0xFFFF)

#define RED     (0xF000)
#define GREEN   (0x0FF0)
#define BLUE    (0x000F)

// shape colors
static uint16_t color_I;
static uint16_t color_J;
static uint16_t color_L;
static uint16_t color_O;
static uint16_t color_S;
static uint16_t color_T;
static uint16_t color_Z;

struct player {
	uint16_t x;
	uint16_t y;
	uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH];
	uint16_t color;
};

// almost the same as a player, except that it just points to the
// player's shape instead
struct shape_projection {
	int16_t x;
	int16_t y;
	uint8_t (*shape)[SHAPE_WIDTH];
};

static uint8_t gp_state;
static int gpfd;
static bool tetris_tick_mutex;
static bool sigio_exec_deferred;
static uint8_t gp_deferred_state;
static uint16_t board[GAME_HEIGHT][GAME_WIDTH] = {{ 0 }};
static struct shape_projection projection;
static struct player player;
static uint8_t shapes[UNIQ_SHAPES][SHAPE_HEIGHT][SHAPE_WIDTH] = {
	{{0, 0, 0, 0},
	 {1, 1, 1, 1},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{0, 0, 1, 0},
	 {1, 1, 1, 0},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{0, 1, 1, 0},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{0, 1, 1, 0},
	 {1, 1, 0, 0},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{0, 1, 0, 0},
	 {1, 1, 1, 0},
	 {0, 0, 0, 0},
	 {0, 0, 0, 0}},

	{{1, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}}
};

// prototypes for now
void memcpy_tetris_shape(uint8_t dst[SHAPE_HEIGHT][SHAPE_WIDTH], uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
bool illegal_shape_position(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			    uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			    int16_t x, int16_t y);
void rotate_shape(uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
void paint_tetris_tile(uint16_t color, int16_t x, int16_t y);
void blit_tetris_shape(uint16_t color, int16_t x, int16_t y,
			uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
void blit_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH]);
void shift_occupied_above_row(int row);
void transfer_shape_to_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			     uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			     int16_t x, int16_t y);
uint16_t get_shape_color(int shape_index);
void new_player_shape();
void restart_tetris();
bool tick_tetris();
void update_projection(struct shape_projection *projection);
uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
void gp_handler(int sig);
void __nanosleep(const struct timespec *req, struct timespec *rem);

void memcpy_tetris_shape(uint8_t dst[SHAPE_HEIGHT][SHAPE_WIDTH], uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH])
{
	int i;
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		memcpy(dst[i], shape[i], SHAPE_WIDTH);
	}
}

bool illegal_shape_position(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			    uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			    int16_t x, int16_t y)
{
	int i, j;

	// if part of the shape would be outside the game area. we are
	// only concerned with whether an actual occupied tile is
	// outside - there is no problem with the container being
	// outside
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			// outside in X-axis
			if ((x + j < 0 || x + j >= GAME_WIDTH) && shape[i][j])
				return true;

			// outside in Y-axis
			if ((y + i < 0 || y + i >= GAME_HEIGHT) && shape[i][j])
				return true;
		}
	}

	// if part of the shape will be where the board is occupied,
	// it is illegal
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j)  {
			if (board[y + i][x + j] && shape[i][j]) {
				return true;
			}
		}
	}

	return false;
}

void rotate_shape(uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH])
{
	// with malloc we could change where uint8_t **shape points
	// and free the old memory, but without it we need the temp
	// variable to go out of scope when this enclosure is gone
	uint8_t tmp_rshape[SHAPE_HEIGHT][SHAPE_WIDTH];
	int i, j;

	do {
		// it is also possible to do this in place for N*N matrices
		// for 90 degree rotation, but it is a bit more complex
		for (i = SHAPE_HEIGHT - 1; i >= 0; --i) {
			for (j = SHAPE_WIDTH - 1; j >= 0; --j) {
				tmp_rshape[j][i] = shape[SHAPE_HEIGHT - 1 - i][j];
			}
		}

		// copy the computed values to the shape we were given
		for (i = 0; i < SHAPE_WIDTH; ++i) {
			for (j = 0; j < SHAPE_WIDTH; ++j) {
				shape[i][j] = tmp_rshape[i][j];
			}
		}
	} while (illegal_shape_position(board, shape, player.x, player.y));
}

void paint_tetris_tile(uint16_t color, int16_t x, int16_t y)
{
	uint16_t tile_x = x * TILE_SIZE;
	uint16_t tile_y = y * TILE_SIZE;

	if (x < 0 || y < 0) {
		printf("Could not paint tile x: %d, y: %d.\n", x, y);
		return;
	}

	paint_region(color, tile_x + BORDER_WIDTH, tile_y + BORDER_WIDTH,
		     TILE_SIZE - BORDER_WIDTH * 2, TILE_SIZE - BORDER_WIDTH * 2);
}

void blit_tetris_shape(uint16_t color, int16_t x, int16_t y,
		       uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH])
{
	int16_t i, j;

	// draw the actual tetris shape
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			if (shape[i][j]) {
				paint_tetris_tile(color, x + j, y + i);
			}
		}
	}

	// don't update regions outside the screen
	x = x < 0 ? 0 : x;

	update_region(x * TILE_SIZE, y * TILE_SIZE,
		      SHAPE_WIDTH * TILE_SIZE, SHAPE_HEIGHT * TILE_SIZE);
}

void blit_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH])
{
	int i, j;

	// draw only tiles that are red, as other functions should
	// take care of drawing back to black themselves when removing
	// from the board
	for (i = 0; i < GAME_HEIGHT; ++i) {
		for (j = 0; j < GAME_WIDTH; ++j) {
			if (board[i][j]) {
				paint_tetris_tile(board[i][j], j, i);
			}
		}
	}

	update_screen();
}

void shift_occupied_above_row(int row)
{
	int i, j;

	for (i = row; i >= 0; --i) {
		for (j = 0; j < GAME_WIDTH; ++j) {
			// if row 0, we should just zero it
			if (i) {
				board[i][j] = board[i - 1][j];
			} else {
				board[i][j] = 0;
			}
		}
	}

	// we take care of redrawing black for tiles that are now
	// empty, so others don't have to think about it (we do not
	// blit the screen, though, we should assume that the caller
	// does that - just update the fb map)
	for (i = 0; i <= row ; ++i) {
		for (j = 0; j < GAME_WIDTH; ++j) {
			if (!board[i][j]) {
				paint_tetris_tile(BLACK, j, i);
			}
		}
	}
}

void transfer_shape_to_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			     uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			     int16_t x, int16_t y)
{
	int i, j;

	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		for (j = 0; j < SHAPE_WIDTH; ++j) {
			if (shape[i][j]) {
				board[y + i][x + j] = player.color;
			}
		}
	}

	// now check if we need to trickle something down after
	// scoring points
	bool all_occupied;
	for (i = y; i < y + SHAPE_HEIGHT; ++i) {
		if (i >= GAME_HEIGHT)
			continue;

		all_occupied = 1;
		for (j = 0; j < GAME_WIDTH; ++j) {
			if (!board[i][j]) {
				all_occupied = 0;
			}
		}

		if (all_occupied) {
			shift_occupied_above_row(i);
		}
	}

	blit_board(board);
}

uint16_t get_shape_color(int shape_index)
{
	// one color for each shape
	switch (shape_index) {
	case 0:
		return color_I;
		break;
	case 1:
		return color_J;
		break;
	case 2:
		return color_L;
		break;
	case 3:
		return color_O;
		break;
	case 4:
		return color_S;
		break;
	case 5:
		return color_T;
		break;
	case 6:
		return color_Z;
		break;
	default:
		break;
	}

	return color_I;
}

void new_player_shape()
{
	player.x = PLAYER_INIT_X;
	player.y = PLAYER_INIT_Y;
	int random_shape = rand() % UNIQ_SHAPES;
	memcpy_tetris_shape(player.shape, shapes[random_shape]);
	player.color = get_shape_color(random_shape);
	update_projection(&projection);

	// even though restart_tetris might call new_player_shape again,
	// after we have reset, it is not possible for the new shape
	// to be in an illegal position
	if (illegal_shape_position(board, player.shape, player.x, player.y)) {
		restart_tetris();
	}
}

void restart_tetris()
{
	// to avoid having to draw all black in blit_board,
	// paint all tiles black here on a reset
	int i, j;
	memset(board, 0, sizeof(board));
	for (i = 0; i < GAME_HEIGHT; ++i) {
		for (j = 0; j < GAME_WIDTH; ++j) {
			paint_tetris_tile(BLACK, j, i);
		}
	}

	new_player_shape();

	// also draw the border again when we reset the game
	for (i = 0; i < GAME_HEIGHT; ++i) {
		paint_tetris_tile(WHITE, GAME_WIDTH, i);
	}

	update_screen(board);
}

bool tick_tetris()
{
	if (!illegal_shape_position(board, player.shape, player.x, player.y + 1)) {
		++player.y;
		return true;
	} else {
		transfer_shape_to_board(board, player.shape, player.x, player.y);
		new_player_shape();
		return false;
	}
}

void update_projection(struct shape_projection *projection)
{
	projection->shape = &(player.shape)[0];
	projection->x = player.x;
	projection->y = player.y;

	// set projection to the position that it would land
	for (;;) {
		if (!illegal_shape_position(board, projection->shape, projection->x, projection->y + 1)) {
			++projection->y;
		} else {
			break;
		}
	}
}

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	return (uint16_t) ((r << 11) | (g << 5) | b);
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

	// as we might be moving, draw were we currently are
	// completely black to avoid having to backtrack
	blit_tetris_shape(BLACK, projection.x, projection.y, player.shape);
	blit_tetris_shape(BLACK, player.x, player.y, player.shape);

	switch (gp_state) {
	case 1:
		if (!illegal_shape_position(board, player.shape, player.x - 1, player.y))
			--player.x;
		break;
	case 2:
		rotate_shape(player.shape);
		break;
	case 4:
		if (!illegal_shape_position(board, player.shape, player.x + 1, player.y))
			++player.x;
		break;
	case 8:
		while (tick_tetris());
		blit_board(board);
		break;
	case 32:
		restart_tetris();
		break;
	case 128:
		tick_tetris();
		break;
	default:
		break;
	}

	// we probably changed where the player is positioned, and
	// thus the projection as well - so do a catch-all reposition
	update_projection(&projection);

	// redraw ourselves after interrupt has handled action
	blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
	blit_tetris_shape(player.color, player.x, player.y, player.shape);
}

void __nanosleep(const struct timespec *req, struct timespec *rem)
{
	struct timespec _rem;
	if (nanosleep(req, rem) == -1)
		__nanosleep(rem, &_rem);
}

int main(int argc, char *argv[])
{
	// reseed every run
	srand(time(NULL));

	// setup framebuffer
	if (setup_screen() == -1) {
		printf("setup_screen() failed.\n");
		return -1;
	}

	// get rid of poor tux (i.e. just init to empty screen)
	paint_screen(BLACK);

	// we also want to draw a game border to the right of
	// GAME_WIDTH only once
	int i;
	for (i = 0; i < GAME_HEIGHT; ++i) {
		paint_tetris_tile(WHITE, GAME_WIDTH, i);
	}

	// setup to read gamepad
	gpfd = open("/dev/gamepad", O_RDWR);
	if (gpfd == -1) {
		printf("open() for gpfd failed with error [%s].\n", strerror(errno));
		return -1;
	}

	// register async notification on SIGIO with /dev/gamepad
	register_SIGIO(gpfd, gp_handler);

	// read in colors
	color_I = rgb888_to_rgb565(0, 255, 255);
	color_J = rgb888_to_rgb565(0, 0, 255);
	color_L = rgb888_to_rgb565(255, 172, 0);
	color_O = rgb888_to_rgb565(255, 255, 0);
	color_S = rgb888_to_rgb565(0, 255, 0);
	color_T = rgb888_to_rgb565(154, 0, 255);
	color_Z = rgb888_to_rgb565(255, 0, 0);

	// setup done, let's start playing
	new_player_shape();

	// setup a projection to use
	update_projection(&projection);

	// initial paint after setting up values like border
	blit_board(board);
	blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
	blit_tetris_shape(player.color, player.x, player.y, player.shape);

	// timespecs used for custom nanosleep that keeps sleeping
	// after signal
	struct timespec req = {0}, rem = {0};
	req.tv_sec = 1;
	req.tv_nsec = 0;

	for (;;) {
		// wrap this code in a mutex to deferr signal handling
		// (we know we are not spawning any more threads), so
		// this is a very simple way to do it without
		// p_thread. if we don't do this, getting a signal
		// during a tick would lead to some weird hiccups in
		// the movement at seemingly random times
		tetris_tick_mutex = 1;

		blit_tetris_shape(BLACK, projection.x, projection.y, player.shape);
		blit_tetris_shape(BLACK, player.x, player.y, player.shape);

		tick_tetris();

		blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
		blit_tetris_shape(player.color, player.x, player.y, player.shape);

		tetris_tick_mutex = 0;

		if (sigio_exec_deferred) {
			sigio_exec_deferred = 0;
			gp_handler(SIGIO);
		}

		// yes - we do need the rem pointer here as well, as
		// nanosleep expects a pointer to a const struct, and
		// a non-const struct (it writes the remaining time
		// back into &rem)
		__nanosleep(&req, &rem);
	}

	if (teardown_screen() == -1) {
		printf("teardown_screen() failed.\n");
		return -1;
	}

	exit(EXIT_SUCCESS);
}

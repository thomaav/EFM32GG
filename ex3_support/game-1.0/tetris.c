#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <time.h>

#include "framebuffer.h"
#include "tetris.h"
#include "util.h"

// create struct for queue to store the next shapes inbound, and
// initialize a head with STAILQ_HEAD macro
struct shape_node {
	uint8_t (*shape)[SHAPE_WIDTH];
	int color_index;
	STAILQ_ENTRY(shape_node) nodes;
}; STAILQ_HEAD(head_s, shape_node) shape_queue_head;

uint16_t color_I;
uint16_t color_J;
uint16_t color_L;
uint16_t color_O;
uint16_t color_S;
uint16_t color_T;
uint16_t color_Z;

struct shape_projection projection;
struct player player;
uint16_t board[GAME_HEIGHT][GAME_WIDTH] = {{0}};
uint8_t shapes[UNIQ_SHAPES][SHAPE_HEIGHT][SHAPE_WIDTH] = {
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
uint8_t score_text[5][LETTER_HEIGHT][LETTER_WIDTH] = {
	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 0, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0},
	 {1, 1, 0, 0},
	 {1, 0, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}}
};
uint8_t digit_text[10][LETTER_HEIGHT][LETTER_WIDTH] = {
	{{1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0}},

	{{1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 1, 0},
	 {1, 1, 1, 0},
	 {0, 0, 1, 0},
	 {1, 1, 1, 0}}
};

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

void paint_text_tile(uint16_t color, int16_t x, int16_t y)
{
	if (x < 0 || y < 0) {
		printf("Could not paint tile x: %d, y: %d.\n", x, y);
		return;
	}

	paint_region(color, x, y, LETTER_TILE_SIZE, LETTER_TILE_SIZE);
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

void paint_queue(uint8_t game_height, uint8_t game_width)
{
	int i, j;
	int shape_idx = 0;
	int offset_top = 3;
	uint16_t color;
	struct shape_node *current_shape;

	STAILQ_FOREACH(current_shape, &shape_queue_head, nodes) {
		color = get_shape_color(current_shape->color_index);
		for (i = 0; i < SHAPE_HEIGHT; ++i) {
			for (j = 0; j < SHAPE_WIDTH; ++j) {
				if ((current_shape->shape)[i][j])
					paint_tetris_tile(color, game_width + 4 + j,
							  shape_idx * (SHAPE_HEIGHT + 1) + i + offset_top);
				else
					paint_tetris_tile(BLACK, game_width + 4 + j,
							  shape_idx * (SHAPE_HEIGHT + 1) + i + offset_top);
			}
		}

		++shape_idx;
	}
}

void paint_glyph(uint8_t (*glyph)[LETTER_WIDTH], uint16_t x, uint16_t  y, uint16_t color)
{
	int i, j;

	for (i = 0; i < LETTER_HEIGHT; ++i) {
		for (j = 0; j < LETTER_WIDTH; ++j) {
			if (glyph[i][j])
				paint_text_tile(color, x + j * LETTER_TILE_SIZE,
						y + i * LETTER_TILE_SIZE);
			else
				paint_text_tile(BLACK, x + j * LETTER_TILE_SIZE,
						y + i * LETTER_TILE_SIZE);
		}
	}
}

void paint_text(uint8_t (*text)[LETTER_HEIGHT][LETTER_WIDTH], uint8_t num_letters,
		uint16_t x, uint16_t y, uint16_t color)
{
	uint8_t i;
	uint16_t letter_x;

	for (i = 0; i < num_letters; ++i) {
		letter_x = x + LETTER_WIDTH * LETTER_TILE_SIZE * i;
		paint_glyph(text[i], letter_x, y, color);
	}
}

void paint_score(uint32_t score, uint16_t x, uint16_t y, uint16_t color)
{
	struct decimal_string dstring = number_to_dstring(score);

	int i;
	uint16_t letter_x;

	for (i = 0; i < dstring.length; ++i) {
		letter_x = x + LETTER_WIDTH * LETTER_TILE_SIZE * i;
		paint_glyph(digit_text[dstring.digits[i]], letter_x, y, color);
	}
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
	int lines_scored = 0;
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
			++lines_scored;
			shift_occupied_above_row(i);
		}
	}

	switch (lines_scored) {
	case 1:
		player.score += 40;
		break;
	case 2:
		player.score += 100;
		break;
	case 3:
		player.score += 300;
		break;
	case 4:
		player.score += 1200;
		break;
	default:
		break;
	}
}

void new_player_shape()
{
	player.x = PLAYER_INIT_X;
	player.y = PLAYER_INIT_Y;

	// score
	paint_score(player.score, 190, 70, WHITE);

	// fetch new shape from the queue of shapes; remember to free
	// the memory
	struct shape_node *next_shape = STAILQ_FIRST(&shape_queue_head);
	STAILQ_REMOVE_HEAD(&shape_queue_head, nodes);
	memcpy_tetris_shape(player.shape, next_shape->shape);
	player.color = get_shape_color(next_shape->color_index);
	free(next_shape);

	update_projection(&projection);

	// insert new shape to the queue of coming shapes
	struct shape_node *new_shape = malloc(sizeof(struct shape_node));
	int random_shape = rand() % UNIQ_SHAPES;
	new_shape->shape = shapes[random_shape];
	new_shape->color_index = random_shape;
	STAILQ_INSERT_TAIL(&shape_queue_head, new_shape, nodes);

	// even though restart_tetris might call new_player_shape again,
	// after we have reset, it is not possible for the new shape
	// to be in an illegal position
	if (illegal_shape_position(board, player.shape, player.x, player.y)) {
		restart_tetris();
	}

	paint_queue(GAME_HEIGHT, GAME_WIDTH);
	paint_text(score_text, 5, 190, 30, WHITE);
	blit_board(board);
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

	// empty shape queue in case we pressed reset
	struct shape_node *current_shape;
	while (!STAILQ_EMPTY(&shape_queue_head)) {
		current_shape = STAILQ_FIRST(&shape_queue_head);
		STAILQ_REMOVE_HEAD(&shape_queue_head, nodes);
		free(current_shape);
	}

	// setup queue of shapes and fetch the first one
	struct shape_node *new_shape;
	int random_shape;
	for (i = 0; i < 4; ++i) {
		new_shape = malloc(sizeof(struct shape_node));
		random_shape = rand() % UNIQ_SHAPES;
		new_shape->shape = shapes[random_shape];
		new_shape->color_index = random_shape;
		STAILQ_INSERT_HEAD(&shape_queue_head, new_shape, nodes);
	}

	player.score = 0;
	new_player_shape();

	// also draw the border again when we reset the game
	for (i = 0; i < GAME_HEIGHT; ++i) {
		paint_tetris_tile(WHITE, GAME_WIDTH, i);
	}

	update_screen();
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

void tick_tetris_and_blit()
{
	blit_tetris_shape(BLACK, projection.x, projection.y, player.shape);
	blit_tetris_shape(BLACK, player.x, player.y, player.shape);

	tick_tetris();

	blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
	blit_tetris_shape(player.color, player.x, player.y, player.shape);
}

void handle_tetris_gp(uint8_t gp_state)
{
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
	case 16:
		paint_screen(BLACK);
		printf("Exiting tetris. Goodbye!\n");
		exit(EXIT_SUCCESS);
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

void initiate_tetris()
{
	// set the gamepad to send its state to our tetris handler
	extern void (*gp_state_handler)(uint8_t);
	gp_state_handler = &handle_tetris_gp;

	// get rid of poor tux (i.e. just init to empty screen)
	paint_screen(BLACK);

	// we also want to draw a game border to the right of
	// GAME_WIDTH only once
	int i;
	for (i = 0; i < GAME_HEIGHT; ++i) {
		paint_tetris_tile(WHITE, GAME_WIDTH, i);
	}

	// read in colors
	color_I = rgb888_to_rgb565(0, 255, 255);
	color_J = rgb888_to_rgb565(0, 0, 255);
	color_L = rgb888_to_rgb565(255, 172, 0);
	color_O = rgb888_to_rgb565(255, 255, 0);
	color_S = rgb888_to_rgb565(0, 255, 0);
	color_T = rgb888_to_rgb565(154, 0, 255);
	color_Z = rgb888_to_rgb565(255, 0, 0);

	// initalize queue for shapes
	STAILQ_INIT(&shape_queue_head);

	// setup done, let's start playing
	restart_tetris();

	// setup a projection to use
	update_projection(&projection);

	// initial paint after setting up values like border
	blit_board(board);
	blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
	blit_tetris_shape(player.color, player.x, player.y, player.shape);
}

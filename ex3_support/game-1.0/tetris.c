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
// initialize a head with STAILQ_HEAD macro, this is used to keep
// track of the incoming tetrominoes
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
uint8_t level_text[5][LETTER_HEIGHT][LETTER_WIDTH] = {
	{{1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {1, 0, 1, 0},
	 {0, 1, 0, 0},
	 {0, 1, 0, 0}},

	{{1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0},
	 {1, 0, 0, 0},
	 {1, 1, 1, 0}},

	{{1, 0, 0, 0},
	 {1, 0, 0, 0},
	 {1, 0, 0, 0},
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
	 {0, 0, 1, 0},
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

/*
  Copy the contents of a given shape into a buffer destination. This
  is to be able to rotate the current shape of the player without
  rotating the base shape.
 */
void memcpy_tetris_shape(uint8_t dst[SHAPE_HEIGHT][SHAPE_WIDTH], uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH])
{
	int i;
	for (i = 0; i < SHAPE_HEIGHT; ++i) {
		memcpy(dst[i], shape[i], SHAPE_WIDTH);
	}
}

/*
  Take an arbitraty x and y value and check whether placing the set
  tetris shape on the board at that position would result in an
  illegal state (e.g. overwriting the board or being outside).
 */
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

/*
  Rotate a shape (seemingly in place to the caller). The function will
  try rotating another 90 degrees until it eventually finds a position
  that is legal. Should it find none, it will end up where it started.
 */
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

/*
  Function to fetch the color of a shape (tetromino), should have just
  been implemented with a macro.
 */
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

/*
  After a player has changed their position, update the projection of
  said position to reflect the current state of the game correctly.
 */
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

/*
  Paint the framebuffer region given by the tetris tile at x, y. Does
  _NOT_ blit.
 */
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

/*
  Paint the tile of a text tile, that is half as wide as a tetris one
  and also takes absolute positions on the LCD grid.
 */
void paint_text_tile(uint16_t color, int16_t x, int16_t y)
{
	if (x < 0 || y < 0) {
		printf("Could not paint tile x: %d, y: %d.\n", x, y);
		return;
	}

	paint_region(color, x, y, LETTER_TILE_SIZE, LETTER_TILE_SIZE);
}

/*
  Paint the tetris tiles of a given shape, and blit it to the screen
  (often by first erasing a tetris shape before moving, to avoid
  backtracking).
 */
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

/*
  Paint all taken tiles of the board, and then blit the entire screen
  for consistency.
 */
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

/*
  Paint the queue of incoming tetrominoes by drawing all of the shapes
  normally.
 */
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

/*
  Paint a single glyph, e.g. "1" or "S".
 */
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

/*
  Paint all the glyphs in a text.
 */
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

/*
  Like paint text, but has to fetch the decimal string before drawing
  the text.
 */
void paint_digits(uint32_t score, uint16_t x, uint16_t y, uint16_t color)
{
	struct decimal_string dstring = number_to_dstring(score);

	int i;
	uint16_t letter_x;

	for (i = 0; i < dstring.length; ++i) {
		letter_x = x + LETTER_WIDTH * LETTER_TILE_SIZE * i;
		paint_glyph(digit_text[dstring.digits[i]], letter_x, y, color);
	}
}

/*
  Shift all rows above the given row down one, this is used to trickle
  down the state of the game when a line is cleared.
 */
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

/*
  When a tetromino hits the board and can go no further, it should be
  transferred to the board before a player receives a new
  tetromino. It is then no longer an actual shape, but a part of the
  board. We also take care of scoring and leveling here.
 */
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

	player.lines_cleared += lines_scored;

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

	// max score fitting into the score column that we write to
	if (player.score > 999999) {
		player.score = 999999;
	}

	if (player.lines_cleared <= 0) {
		player.level = 0;
	} else if (player.lines_cleared >= 1 && player.lines_cleared < 90) {
		player.level = player.lines_cleared / 10;
	} else {
		player.level = 9;
	}
}

/*
  Reset the player position and projection, and give the next
  tetromino in the queue. Also insert a new shape when popping from
  the queue.
 */
void new_player_shape()
{
	player.x = PLAYER_INIT_X;
	player.y = PLAYER_INIT_Y;

	// score and level (+ lines left to next level)
	paint_digits(player.score, 200, 70, WHITE);
	paint_digits(player.level, 200, 170, WHITE);

	// pad with 0s for lines left here, since we want float: right
	if (10 - (player.lines_cleared % 10) == 10) {
		paint_digits(10 - (player.lines_cleared % 10), 260, 170, WHITE);
	} else {
		paint_digits(0, 260, 170, BLACK);
		paint_digits(10 - (player.lines_cleared % 10), 280, 170, WHITE);
	}

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
	paint_text(score_text, 5, 200, 30, WHITE);
	paint_text(level_text, 5, 200, 130, WHITE);
	blit_board(board);
}

/*
  Restart the tetris game entirely.
 */
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

	// reset score and level and draw over it
	player.score = 0;
	player.level = 0;
	player.lines_cleared = 0;

	paint_digits(player.score, 200, 70, WHITE);
	paint_region(BLACK, 200, 70, 120, 30);
	paint_digits(player.level, 200, 170, WHITE);
	paint_region(BLACK, 200, 170, 120, 30);

	new_player_shape();

	// also draw the border again when we reset the game
	for (i = 0; i < GAME_HEIGHT; ++i) {
		paint_tetris_tile(WHITE, GAME_WIDTH, i);
	}

	update_screen();
}

/*
  Tick the tetris game, which usually means moving a tetromino further
  down one step, or adding it to the board if it has no place to go.
 */
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

/*
  Wrapper around ticking tetris that also blits, this is needed so
  that a fall through can be simulated with ticking the tetris until
  the piece no longer can go anywhere. This is called during the main
  loop.
 */
void tick_tetris_and_blit()
{
	blit_tetris_shape(BLACK, projection.x, projection.y, player.shape);
	blit_tetris_shape(BLACK, player.x, player.y, player.shape);

	tick_tetris();

	blit_tetris_shape(BLUE, projection.x, projection.y, player.shape);
	blit_tetris_shape(player.color, player.x, player.y, player.shape);
}

/*
  A pointer to this function is passed to the function that handles
  SIGIO signals, and can pass the gamepad value to the game's own
  handler. Handle what happens next according to which button the user
  has pressed.
 */
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

/*
  Initiate tetris, i.e. set up the handler of SIGIO signals, paint the
  screen an initial black, setup some colors and a queue of
  tetrominoes and start the game.
 */
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

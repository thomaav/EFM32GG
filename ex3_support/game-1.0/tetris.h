#pragma once

#define TILE_SIZE 10
#define BORDER_WIDTH 1

#define GAME_WIDTH 10
#define GAME_HEIGHT (HEIGHT / TILE_SIZE)

#define UNIQ_SHAPES 7
#define SHAPE_WIDTH 4
#define SHAPE_HEIGHT 4

#define PLAYER_INIT_X 3
#define PLAYER_INIT_Y 0

#define LETTER_TILE_SIZE 5
#define LETTER_HEIGHT 5
#define LETTER_WIDTH 4

struct player {
	uint16_t x;
	uint16_t y;
	uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH];
	uint16_t color;
	uint32_t score;
};

// almost the same as a player, except that it just points to the
// player's shape instead
struct shape_projection {
	int16_t x;
	int16_t y;
	uint8_t (*shape)[SHAPE_WIDTH];
};

// shape colors
extern uint16_t color_I;
extern uint16_t color_J;
extern uint16_t color_L;
extern uint16_t color_O;
extern uint16_t color_S;
extern uint16_t color_T;
extern uint16_t color_Z;

extern struct shape_projection projection;
extern struct player player;
extern uint16_t board[GAME_HEIGHT][GAME_WIDTH];
extern uint8_t shapes[UNIQ_SHAPES][SHAPE_HEIGHT][SHAPE_WIDTH];
extern uint8_t score_text[5][LETTER_HEIGHT][LETTER_WIDTH];
extern uint8_t digit_text[10][LETTER_HEIGHT][LETTER_WIDTH];

// NOTE: it should not be considered safe to be passing something like
// board[GAME_HEIGHT][GAME_WIDTH] to the function (even though the
// compiler will warn you about mismatching types, though), as all you
// actually get is a pointer, and you will have no idea what the size
// of the data is. however, as all of these sizes are set by global
// definitions, we decided to not pass these explicitly along with the
// pointer, and instead use them directly in the function.

// shape manipulation
void memcpy_tetris_shape(uint8_t dst[SHAPE_HEIGHT][SHAPE_WIDTH],
			 uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
bool illegal_shape_position(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			    uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			    int16_t x, int16_t y);
void rotate_shape(uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
uint16_t get_shape_color(int shape_index);
void update_projection(struct shape_projection *projection);

// blitting and painting to framebuffer/screen
void paint_tetris_tile(uint16_t color, int16_t x, int16_t y);
void paint_text_tile(uint16_t color, int16_t x, int16_t y);
void blit_tetris_shape(uint16_t color, int16_t x, int16_t y,
			uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
void blit_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH]);
void paint_queue(uint8_t game_height, uint8_t game_width);
void paint_glyph(uint8_t (*glyph)[LETTER_WIDTH], uint16_t x, uint16_t  y, uint16_t color);
void paint_text(uint8_t (*text)[LETTER_HEIGHT][LETTER_WIDTH], uint8_t num_letters,
		uint16_t x, uint16_t y, uint16_t color);
void paint_score(uint32_t score, uint16_t x, uint16_t y, uint16_t color);

// board manipulation
void shift_occupied_above_row(int row);
void transfer_shape_to_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH],
			     uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH],
			     int16_t x, int16_t y);

// game related tetris calls
void new_player_shape();
void restart_tetris();
bool tick_tetris();
void tick_tetris_and_blit();
void handle_tetris_gp(uint8_t gp_state);
void initiate_tetris();

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
void blit_tetris_shape(uint16_t color, int16_t x, int16_t y,
			uint8_t shape[SHAPE_HEIGHT][SHAPE_WIDTH]);
void blit_board(uint16_t board[GAME_HEIGHT][GAME_WIDTH]);

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

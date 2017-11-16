#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GPBUF_SIZE 1

int gpfd;
uint8_t gp_state;

bool tetris_tick_mutex;
bool sigio_exec_deferred;
uint8_t gp_deferred_state;

// declaration for the pointer that gp_handler will send gp_state to
// for handling input (in our case tetris), but it is easy to change
// for other games
void (*gp_state_handler)(uint8_t);

int gp_init();
void gp_handler(int sig);

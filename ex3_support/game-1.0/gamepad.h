#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GPBUF_SIZE 1

extern int gpfd;
extern uint8_t gp_state;

extern bool tetris_tick_mutex;
extern bool sigio_exec_deferred;
extern uint8_t gp_deferred_state;

// declaration for the pointer that gp_handler will send gp_state to
// for handling input (in our case tetris), but it is easy to change
// for other games
extern void (*gp_state_handler)(uint8_t);

int gp_init();
void gp_handler(int sig);

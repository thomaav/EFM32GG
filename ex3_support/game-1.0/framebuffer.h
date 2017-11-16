#pragma once

#include <linux/fb.h>

#define WIDTH 320
#define HEIGHT 240
#define FBSIZE ((size_t) (WIDTH * HEIGHT * 2))

// setup which parts of framebuffer that is to be refreshed for
// performance reasons, a small as rectangle as possible, and create
// an fd to fb0 and the pointer we will map to it
extern struct fb_copyarea rect;
extern int fbfd;
extern uint16_t *fb_map;

int mmap_fb(uint16_t **map, int fbfd);
int unmap_fb(uint16_t **map);

int setup_screen();
int teardown_screen();

void update_screen();
void update_region(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void clear_screen();
void paint_screen(uint16_t color);
void paint_region(uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "framebuffer.h"

int mmap_fb(uint16_t **map, int fbfd)
{
	*map = mmap(0, FBSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	if (*map == MAP_FAILED)
		return -1;

	return 0;
}

int unmap_fb(uint16_t **map)
{
	if (munmap(*map, FBSIZE) == -1)
		return -1;

	return 0;
}

int setup_screen()
{
	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		printf("open() failed with error [%s].\n", strerror(errno));
		return -1;
	}

	// actually map /dev/fb0 to some memory
	printf("[before] fb_map points to: %p.\n", fb_map);
	if (mmap_fb(&fb_map, fbfd) == -1) {
		printf("mmap() failed with error [%s].\n", strerror(errno));
		return -1;
	} else {
		printf("[after] fb_map points to: %p.\n", fb_map);
	}

	// if we successfully mapped memory for the framebuffer, we
	// also want to make sure that the copyarea rectangle is
	// initialized, here to the width and height of the entire
	// screen
	rect.dx = 0;
	rect.dy = 0;
	rect.width = WIDTH;
	rect.height = HEIGHT;

	return 0;
}

int teardown_screen()
{
	// unmap the /dev/fb0 memory again
	if (unmap_fb(&fb_map) == -1) {
		printf("munmap() failed with error [%s].\n", strerror(errno));
		return -1;
	}

	if (close(fbfd) == -1) {
		printf("close() failed with error [%s].\n", strerror(errno));
		return -1;
	}

	return 0;
}

void update_screen()
{
	rect.dx = 0;
	rect.dy = 0;
	rect.width = WIDTH;
	rect.height = HEIGHT;

	ioctl(fbfd, 0x4680, &rect);
}

void update_region(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	rect.dx = x;
	rect.dy = y;
	rect.width = width;
	rect.height = height;

	ioctl(fbfd, 0x4680, &rect);
}

void clear_screen()
{
	memset(fb_map, 0x0, FBSIZE);
}

void paint_screen(uint16_t color)
{
	int x, y;

	for (x = 0; x < WIDTH; ++x) {
		for (y = 0; y < HEIGHT; ++y) {
			fb_map[WIDTH * y + x] = color;
		}
	}

	update_screen();
}

void paint_region(uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	uint16_t i, j;

	if (x + width > WIDTH || y + height > HEIGHT) {
		printf("Could not paint region x: %d, y: %d.\n", x, y);
		return;
	}

	for (i = x; i < x + width; ++i) {
		for (j = y; j < y + height; ++j) {
			fb_map[WIDTH * j + i] = color;
		}
	}
}

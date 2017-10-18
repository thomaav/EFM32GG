#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define WIDTH 320
#define HEIGHT 240
#define FBSIZE ((size_t) (WIDTH * HEIGHT * 2))

int mmap_fb(uint16_t **map, int fbfd)
{
	*map = mmap(0, FBSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	if (*map == MAP_FAILED) {
		return -1;
	}

	return 0;
}

int unmap_fb(uint16_t **map)
{
	if (munmap(*map, FBSIZE) == -1) {
		return -1;
	}

	return 0;
}

void update_screen(int fbfd, struct fb_copyarea *rect)
{
	ioctl(fbfd, 0x4680, rect);
}

void clear_screen(uint16_t *fb_map)
{
	memset(fb_map, 0x0, FBSIZE);
}

void paint_screen(uint16_t *fb_map, uint16_t *color)
{
	int x, y;
	for (x = 0; x < WIDTH; ++x) {
		for (y = 0; y < HEIGHT; ++y) {
			fb_map[WIDTH * y + x] = *color;
		}
	}
}

int main(int argc, char *argv[])
{
	int fbfd;

	fbfd = open("/dev/fb0", O_RDWR);
	if(fbfd == -1) {
		printf("open() failed with error [%s].\n", strerror(errno));
		return -1;
	}

	// setup which parts of framebuffer that is to be refreshed
	// for performance reasons, a small as rectangle as possible
	struct fb_copyarea rect;
	rect.dx = 0;
	rect.dy = 0;
	rect.width = WIDTH;
	rect.height = HEIGHT;

	// actually map /dev/fb0 to some memory
	uint16_t *fb_map;
	printf("[before] fb_map points to: %p.\n", fb_map);
	if (mmap_fb(&fb_map, fbfd) == -1) {
		printf("mmap() failed with error [%s].\n", strerror(errno));
		return -1;
	} else {
		printf("[after] fb_map points to: %p.\n", fb_map);
	}

	uint16_t blue = 0x000F;
	paint_screen(fb_map, &blue);

	update_screen(fbfd, &rect);

	// unmap the /dev/fb0 memory again
	if (unmap_fb(&fb_map) == -1) {
		printf("munmap() failed with error [%s].\n", strerror(errno));
		return -1;
	}

	if (close(fbfd) == -1) {
		return -1;
	}

	printf("Hello World, I'm game! Now we are opening files..\n");

	exit(EXIT_SUCCESS);
}

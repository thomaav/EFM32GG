#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "framebuffer.h"

int main(int argc, char *argv[])
{
	if (setup_screen() == -1) {
		printf("setup_screen() failed.\n");
		return -1;
	}

	uint16_t blue = 0x000F;
	paint_screen(&blue);
	update_screen();

	if (teardown_screen() == -1) {
		printf("setup_screen() failed.\n");
		return -1;
	}

	printf("Hello World, I'm game! Now we are opening files..\n");

	exit(EXIT_SUCCESS);
}

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "signal.h"

void register_SIGIO(int fd, void *act)
{
	// register async notification on SIGIO with /dev/gamepad
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = act;
	action.sa_flags = 0;

	sigaction(SIGIO, &action, NULL);

	fcntl(fd, F_SETOWN, getpid());
	int oflags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oflags | FASYNC);
}

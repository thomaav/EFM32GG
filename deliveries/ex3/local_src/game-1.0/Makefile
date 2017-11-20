#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#
CFLAGS += -O2 -g -Wall #-Wsign-compare -Wfloat-equal -Wformat-security #-Werror
CPPFLAGS +=
LDFLAGS +=
OBJS = framebuffer.o signal.o util.o gamepad.o tetris.o


%.o: %.c
	${CC} ${CFLAGS} $< -c -o $@

game: game.c ${OBJS}
	${CC} ${CFLAGS} $^ -o $@

clean:
	-rm -f game *.o

install:


.PHONY: install clean

.PHONY : pretty
pretty :
	-indent *.c *.h


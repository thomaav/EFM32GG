# Example Makefile
#
# Exercise 2, TDT4258

# differences from original:
# added: link with audio.o, button.o, led.o, melody.h

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS=-mcpu=cortex-m3 -mthumb -g -O0 -std=c99 -Wall
LDFLAGS=-mcpu=cortex-m3 -mthumb -g -lgcc -lc -lefm32gg -lcs3 -lcs3unhosted -Llib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g
LINKERSCRIPT=lib/efm32gg.ld

ex2.bin : ex2.elf
	${OBJCOPY} -O binary $< $@

ex2.elf : audio.o button.o dac.o dma.o ex2.o gpio.o interrupt_handlers.o led.o timer.o nvic.o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

%.o : %.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY : pretty
pretty :
	-indent *.c *.h

.PHONY : upload
upload : ex2.bin
	-eACommander.sh -r --address 0x00000000 -f "ex2.bin" -r

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex

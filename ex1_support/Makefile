# Example Makefile
#
# Exercise 1, TDT4258
#
#
# Usage:
# make TARGET=ex1_baseline upload
# make TARGET=ex1_improved upload
# make TARGET=ex1_baseline ex1.elf


LD=arm-none-eabi-gcc
AS=arm-none-eabi-as
OBJCOPY=arm-none-eabi-objcopy

LDFLAGS=-nostdlib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g

LINKERSCRIPT=core/efm32gg.ld

ex1.bin : ex1.elf
	${OBJCOPY} -j .text -O binary ${TARGET}/$< $@

ex1.elf : ex1.o
	${LD} -T ${LINKERSCRIPT} ${TARGET}/$^ -o ${TARGET}/$@ ${LDFLAGS}

ex1.o : ${TARGET}/ex1.s
	${AS} ${ASFLAGS} $< -o ${TARGET}/$@

.PHONY : upload
upload : ex1.bin
	-eACommander.sh -r --address 0x00000000 -f ${TARGET}/$< -r

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex

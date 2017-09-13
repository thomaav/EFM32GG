# Example Makefile
#
# Exercise 1, TDT4258

LD=arm-none-eabi-gcc
AS=arm-none-eabi-as
OBJCOPY=arm-none-eabi-objcopy

LDFLAGS=-nostdlib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g

LINKERSCRIPT=efm32gg.ld

ex1.bin : ex1.elf
	${OBJCOPY} -j .text -O binary $< $@

ex1base.bin : ex1base.elf
	${OBJCOPY} -j .text -O binary $< $@

ex1improved.bin : ex1improved.elf
	${OBJCOPY} -j .text -O binary $< $@

ex1.elf : ex1.o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

ex1base.elf : ex1base.o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

ex1improved.elf : ex1improved.o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

ex1.o : ex1.s
	${AS} ${ASFLAGS} $< -o $@

ex1base.o : ex1base.s
	${AS} ${ASFLAGS} $< -o $@

ex1improved.o : ex1improved.s
	${AS} ${ASFLAGS} $< -o $@

.PHONY : upload
upload :
	-eACommander.sh -r --address 0x00000000 -f "ex1.bin" -r

.PHONY : uploadbase
uploadbase:
	-eACommander.sh -r --address 0x00000000 -f "ex1base.bin" -r

.PHONY : uploadimproved
uploadimproved:
	-eACommander.sh -r --address 0x00000000 -f "ex1improved.bin" -r

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex

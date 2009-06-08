# makfile configuration
NAME            = rtos
CPU             = msp430x449
FRTOS=../FreeRTOSV3.2.4
IFLAGS		= -I. -I${FRTOS}/Source/include -I${FRTOS}/Demo/Common/include 

CFLAGS          = -mmcu=${CPU} -O2 -Wall -g ${IFLAGS} -DGCC_MSP430 	


#switch the compiler (for the internal make rules)
CC              = msp430-gcc

.PHONY: all FORCE clean download download-jtag download-bsl dist

#all should be the first target. it's built when make is run without args
all: ${NAME}.elf 
debug : ${NAME}.a43 ${NAME}.lst ${NAME}.pp.c ${NAME}.s

#confgigure the next line if you want to use the serial download
download: download-jtag
#download: download-bsl

# Setup paths to source code
SOURCE_PATH = ${FRTOS}/Source
PORT_PATH = ${FRTOS}/Source/portable/GCC/MSP430F449
DEMO_PATH = ${FRTOS}/test

#
# Source files that can be built to THUMB mode.
#
SRC = \
main.c \
$(SOURCE_PATH)/tasks.c \
$(SOURCE_PATH)/list.c \
$(SOURCE_PATH)/queue.c \
$(SOURCE_PATH)/portable/MemMang/heap_1.c \
$(PORT_PATH)/port.c



#additional rules for files


OBJ = $(SRC:.c=.o)

${NAME}.elf : $(OBJ) Makefile
	$(CC) $(OBJ) $(CFLAGS) -o $@

$(OBJ) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

${NAME}.a43: ${NAME}.elf
	msp430-objcopy -O ihex $^ $@

${NAME}.lst: ${NAME}.elf
	msp430-objdump -dSt $^ >$@

${NAME}.dmp: ${NAME}.elf
	msp430-objdump -d $^ >$@

${NAME}.pp.c: main.c
	${CC} -E $(CFLAGS) $^ >$@

${NAME}.s:  main.c
	$(CC) -S $(CFLAGS) $^ >$@

download-jtag: all
	msp430-jtag -e ${NAME}.elf

download-bsl: all
	msp430-bsl -e ${NAME}.elf

clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${OBJ}

#backup archive
dist:
	tar cf dist.tar *.c *.h *.txt Makefile
	gzip dist.tar
#dummy target as dependecy if something has to be build everytime
FORCE:



# makfile configuration
NAME     = wsn430-serial
OBJECTS  = main.o clock.o uart1.o timer.o
CPU      = msp430x1611
INCLUDES = -I../wsn430-leds -I../wsn430-clock -I../wsn430-timer -I../wsn430-serial
CFLAGS   = -mmcu=${CPU} -O2 -Wall ${INCLUDES} -g

#switch the compiler (for the internal make rules)
CC              = msp430-gcc

.PHONY: all FORCE clean download download-jtag download-bsl dist

#all should be the first target. it's built when make is run without args
all: ${NAME}.elf ${NAME}.a43 ${NAME}.lst

#confgigure the next line if you want to use the serial download
download: download-jtag
#download: download-bsl

#additional rules for files
${NAME}.elf: ${OBJECTS}
	${CC} -mmcu=${CPU} -o $@ ${OBJECTS}

${NAME}.a43: ${NAME}.elf
	msp430-objcopy -O ihex $^ $@

${NAME}.lst: ${NAME}.elf
	msp430-objdump -dSt $^ >$@

${NAME}.dmp: ${NAME}.elf
	msp430-objdump -d $^ >$@

main.pp.c: main.c
	${CC} -E $(CFLAGS) $^ >$@

main.s: main.c
	${CC} -S $(CFLAGS) $^ >$@

download-jtag: all
	msp430-jtag -e ${NAME}.elf

download-bsl: all
	msp430-bsl -e ${NAME}.elf

clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${OBJECTS} *.log *.vcd *.trc

#backup archive
dist:
	tar czf dist.tgz *.c *.h *.txt makefile

#dummy target as dependecy if something has to be build everytime
FORCE:

#project dependencies
main.o: main.c

clock.o: ../wsn430-clock/clock.c ../wsn430-clock/clock.h
	${CC} -c ${CFLAGS} $<

uart1.o: ../wsn430-serial/uart1.c ../wsn430-serial/uart1.h
	${CC} -c ${CFLAGS} $<

timer.o: ../wsn430-timer/timer.c ../wsn430-timer/timer.h
	${CC} -c ${CFLAGS} $<

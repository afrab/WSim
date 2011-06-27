# makfile configuration
NAME     = test_spi_master
OBJECTS  = main.o uart1.o clock.o
CPU      = msp430x1611
INCLUDES = -I.
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

uart1.o: ./uart1.c ./uart1.h
	${CC} -c ${CFLAGS} $<

clock.o: ./clock.c ./clock.h
	${CC} -c ${CFLAGS} $<

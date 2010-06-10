# makfile configuration
NAME            = micaz-leds
OBJECTS         = main.o
CPU             = atmega128 

CFLAGS          = -mmcu=${CPU} -O2 -Wall -g

#switch the compiler (for the internal make rules)
CC              = avr-gcc 

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
	avr-objcopy -O ihex $^ $@

${NAME}.lst: ${NAME}.elf
	avr-objdump -dSt $^ >$@

${NAME}.dmp: ${NAME}.elf
	avr-objdump -d $^ >$@

main.pp.c: main.c
	${CC} -E $(CFLAGS) $^ >$@

main.s: main.c
	${CC} -S $(CFLAGS) $^ >$@

clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${OBJECTS} *.log *.vcd *.trc

#backup archive
dist:
	tar czf dist.tgz *.c *.h *.txt makefile

#dummy target as dependecy if something has to be build everytime
FORCE:

#project dependencies
main.o: main.c


# makfile configuration
NAME-26  = cc1100-26MHz
NAME-27  = cc1100-27MHz

OBJECTS  = main.o uart1.o clock.o timer.o cc1100_wsn430.o ds2411.o
CC1100-26= cc1100-26.o
CC1100-27= cc1100-27.o

CPU      = msp430x1611
INCLUDES = -I../wsn430-leds -I../wsn430-serial -I../wsn430-ds2411 -I../wsn430-timer -I../wsn430-clock
CFLAGS   = -mmcu=${CPU} -Wall ${INCLUDES} -g -DCC1100_POLLING_RX

CC       = msp430-gcc

.PHONY: all FORCE clean download download-jtag download-bsl dist

#all should be the first target. it's built when make is run without args
all:    ${NAME-26}.elf ${NAME-26}.a43 ${NAME-26}.lst \
	${NAME-27}.elf ${NAME-27}.a43 ${NAME-27}.lst

#confgigure the next line if you want to use the serial download
download: download-jtag
#download: download-bsl

#additional rules for files
${NAME-26}.elf: ${OBJECTS} ${CC1100-26}
	${CC} -mmcu=${CPU} -o $@ ${OBJECTS} ${CC1100-26}

${NAME-27}.elf: ${OBJECTS} ${CC1100-27}
	${CC} -mmcu=${CPU} -o $@ ${OBJECTS} ${CC1100-27}


${CC1100-26}: cc1100.c
	${CC} -c ${CFLAGS} -DFREQ_BASE_26MHz $< -o ${CC1100-26}

${CC1100-27}: cc1100.c
	${CC} -c ${CFLAGS} -DFREQ_BASE_27MHz $< -o ${CC1100-27}

# ##################################################

%.o: %.c
	${CC} -c ${CFLAGS} $<

%.s: %.c
	${CC} -S $(CFLAGS) $^ >$@

# ##################################################

%.a43: %.elf
	msp430-objcopy -O ihex $^ $@

%.lst: %.elf
	msp430-objdump -dSt $^ >$@

%.dmp: %.elf
	msp430-objdump -d $^ >$@



clean:
	rm -f   ${NAME-26}.elf ${NAME-26}.a43 ${NAME-26}.lst ${CC1100-26} ${OBJECTS} \
		${NAME-27}.elf ${NAME-27}.a43 ${NAME-27}.lst ${CC1100-27} \
		*.log *.vcd *.trc

#dummy target as dependecy if something has to be build everytime
FORCE:

#project dependencies
main.o: main.c 

uart1.o: ../wsn430-serial/uart1.c ../wsn430-serial/uart1.h
	${CC} -c ${CFLAGS} $<

ds2411.o: ../wsn430-ds2411/ds2411.c ../wsn430-ds2411/ds2411.h
	${CC} -c ${CFLAGS} -O2 $< 

clock.o: ../wsn430-clock/clock.c ../wsn430-clock/clock.h
	${CC} -c ${CFLAGS} $<

timer.o: ../wsn430-timer/timer.c ../wsn430-timer/timer.h
	${CC} -c ${CFLAGS} $<


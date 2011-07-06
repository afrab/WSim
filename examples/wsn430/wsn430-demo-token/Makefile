NAME     = wsn430-demo
CPU      = msp430x1611
CC       = msp430-gcc

OBJECTS  = main.o uart1.o ds2411.o clock.o timer.o cc1100_27MHz.o cc1100_wsn430.o
INCLUDES = -I../wsn430-leds -I../wsn430-serial -I../wsn430-cc1100 -I../wsn430-ds2411 -I../wsn430-clock -I../wsn430-timer
CFLAGS   = -mmcu=${CPU} -Wall ${INCLUDES} -g 


all: ${NAME}.elf  ${NAME}.lst

${NAME}.elf: ${OBJECTS}
	${CC} -mmcu=${CPU} -o $@ ${OBJECTS}

${NAME}.lst: ${NAME}.elf
	msp430-objdump -dSt $^ >$@

clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${OBJECTS}	\
		*.eps *.log *.trc *.pkt *.etr *.gp *.err *.vcd 


#project dependencies
main.o: main.c 
	${CC} -c ${CFLAGS} $<

uart1.o: ../wsn430-serial/uart1.c ../wsn430-serial/uart1.h
	${CC} -c ${CFLAGS} $<

cc1100_27MHz.o: ../wsn430-cc1100/cc1100.c ../wsn430-cc1100/cc1100.h 
	${CC} -c ${CFLAGS} -DFREQ_BASE_27MHz $< -o $@

cc1100_wsn430.o: ../wsn430-cc1100/cc1100_wsn430.c ../wsn430-cc1100/cc1100_wsn430.h 
	${CC} -c ${CFLAGS} $<

ds2411.o: ../wsn430-ds2411/ds2411.c ../wsn430-ds2411/ds2411.h
	${CC} -c ${CFLAGS} -O2 $< 

clock.o: ../wsn430-clock/clock.c ../wsn430-clock/clock.h
	${CC} -c ${CFLAGS} $<

timer.o: ../wsn430-timer/timer.c ../wsn430-timer/timer.h
	${CC} -c ${CFLAGS} $<

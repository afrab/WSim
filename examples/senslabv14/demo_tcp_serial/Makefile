LISTNOD  = 0 1 2 3
MAXNODES = 4

NAME     = senslabv14-demo-serial
CPU      = msp430x1611
CC       = msp430-gcc

OBJECTS  = uart0.o ds2411.o clock.o timerA.o cc2420.o
INCLUDES = -I../drivers
CFLAGS   = -mmcu=${CPU} -Wall ${INCLUDES} -g


all: ${NAME}.elf  ${NAME}.lst

${NAME}.elf: ${OBJECTS} main
	${CC} -mmcu=${CPU} -o ${NAME}.elf ${OBJECTS} main.o

${NAME}.lst: ${NAME}.elf
	msp430-objdump -dSt ${NAME}.elf > ${NAME}.lst

clean:
	rm -f ${NAME}*.elf ${NAME}*.a43 ${NAME}*.lst main*.o ${OBJECTS}	\
		*.eps *.log *.trc *.etr *.gp *.err *.vcd 


#project dependencies
main: main.c 
	${CC} -c ${CFLAGS} $< -o main.o

uart0.o: ../drivers/uart0.c ../drivers/uart0.h
	${CC} -c ${CFLAGS} $<

cc2420.o: ../drivers/cc2420.c ../drivers/cc2420.h 
	${CC} -c ${CFLAGS} -DCC2420_ENABLE_AUTOACK $<


ds2411.o: ../drivers/ds2411.c ../drivers/ds2411.h
	${CC} -c ${CFLAGS} -O2 $< 

clock.o: ../drivers/clock.c ../drivers/clock.h
	${CC} -c ${CFLAGS} $<

timerA.o: ../drivers/timerA.c ../drivers/timerA.h
	${CC} -c ${CFLAGS} $<

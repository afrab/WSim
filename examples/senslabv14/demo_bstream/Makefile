LISTNOD  = 0 1 2
MAXNODES = 3

NAME     = senslabv14-demo-bstream
CPU      = msp430x1611
CC       = msp430-gcc

OBJECTS  = uart0.o ds2411.o clock.o timerA.o cc2420.o
INCLUDES = -I../drivers
CFLAGS   = -mmcu=${CPU} -Wall ${INCLUDES} -g


all: ${NAME}.elf  ${NAME}.lst

${NAME}.elf: ${OBJECTS} main
	@for i in ${LISTNOD}; do ${CC} -mmcu=${CPU} -o ${NAME}$$i.elf ${OBJECTS} main$$i.o; done

${NAME}.lst: ${NAME}.elf
	@for i in ${LISTNOD}; do msp430-objdump -dSt ${NAME}$$i.elf >${NAME}$$i.lst; done

clean:
	rm -f ${NAME}*.elf ${NAME}*.a43 ${NAME}*.lst main*.o ${OBJECTS}	\
		*.eps *.log *.trc *.etr *.gp *.err *.vcd 


#project dependencies
main: main.c 
	@for i in ${LISTNOD}; do ${CC} -DNODEID=$$i -DMAX_NODES=${MAXNODES} -c ${CFLAGS} $< -o main$$i.o; done

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

## -*-makefile-*-
NAME    = test2-spi
SRC     = main.c

CC=msp430-gcc
CPU=msp430x1611
CFLAGS  = -mmcu=${CPU} -Wall
CFLAGS += -mdisable-hwmul
CFLAGS += -O2 -g

OBJ_BASE= leds.o clock.o timer.o uart.o spi.o

OBJECTS = ${OBJ_BASE} 

CCMDLINE = ${CFLAGS} ${INCLUDES}
OBJS=$(SRC:.c=.o)
OBJECTS+=${OBJS}

# ##################################################
# ##################################################

${NAME}.elf: ${OBJECTS} 
	@${CC} -mmcu=${CPU} ${LDFLAGS} ${OBJECTS} ${LDLIBS} -o $@ 
	@echo LD $@
	@echo "=================================================="
	@echo "`pwd` ::" $@
	@msp430-size $@
	@echo "=================================================="

%.lst: %.elf
	@msp430-objdump -dSt $^ >$@

# ##################################################
# ##################################################

%.o:%.c
	@echo CC ${CFLAGS} $< 
	@${CC} -c ${CCMDLINE} $< -o $@

clean:
	rm *.o *.elf

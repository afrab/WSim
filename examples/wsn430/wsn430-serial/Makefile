
# define one name for each executable to be built
NAMES      = wsn430-serial
CPU        = -mmcu=msp430x1611
UTILS_PATH = ../../utils

# sources files specific to 'first_target'
SRC_wsn430-serial = ../wsn430-clock/clock.c ../wsn430-timer/timer.c uart1.c
SRC              = main.c
INCLUDES         = -I. -I../wsn430-leds -I../wsn430-clock -I../wsn430-timer

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile


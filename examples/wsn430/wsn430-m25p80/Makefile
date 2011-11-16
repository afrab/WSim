
# define one name for each executable to be built
NAMES      = wsn430-m25p80
CPU        = -mmcu=msp430x1611
UTILS_PATH = ../../utils

# sources files specific to 'first_target'
SRC_wsn430-m25p80 = main.c m25p80.c
SRC               = ../wsn430-clock/clock.c 
SRC              += ../wsn430-timer/timer.c 
SRC              += ../wsn430-serial/uart1.c
INCLUDES          = -I. -I../wsn430-leds -I../wsn430-clock -I../wsn430-timer -I../wsn430-serial

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile



# define one name for each executable to be built
NAMES      = wsn430-token
CPU        = -mmcu=msp430x1611
OPT        = -O2

UTILS_PATH = ../../utils
CFLAGS     = -DFREQ_BASE_27MHz

# sources files specific to 'first_target'
SRC_wsn430-token = main.c

SRC               = ../wsn430-clock/clock.c 
SRC              += ../wsn430-timer/timer.c 
SRC              += ../wsn430-serial/uart1.c
SRC              += ../wsn430-m25p80/m25p80.c
SRC              += ../wsn430-ds2411/ds2411.c
SRC              += ../wsn430-cc1100/cc1100.c
SRC              += ../wsn430-cc1100/cc1100_wsn430.c

INCLUDES          = -I. 
INCLUDES         += -I../wsn430-leds 
INCLUDES         += -I../wsn430-clock 
INCLUDES         += -I../wsn430-timer
INCLUDES         += -I../wsn430-serial
INCLUDES         += -I../wsn430-m25p80
INCLUDES         += -I../wsn430-ds2411
INCLUDES         += -I../wsn430-cc1100

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile


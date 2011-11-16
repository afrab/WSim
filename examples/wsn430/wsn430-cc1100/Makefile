
# define one name for each executable to be built
NAMES      = wsn430-cc1100
CPU        = -mmcu=msp430x1611
OPT        = -O2

UTILS_PATH = ../../utils
CFLAGS     = -DCC1100_POLLING_RX -DFREQ_BASE_27MHz
#CFLAGS     = -DCC1100_POLLING_RX -DFREQ_BASE_26MHz

# sources files specific to 'first_target'
SRC_wsn430-cc1100 = main.c cc1100.c cc1100_wsn430.c

SRC               = ../wsn430-clock/clock.c 
SRC              += ../wsn430-timer/timer.c 
SRC              += ../wsn430-serial/uart1.c
SRC              += ../wsn430-m25p80/m25p80.c
SRC              += ../wsn430-ds2411/ds2411.c

INCLUDES          = -I. 
INCLUDES         += -I../wsn430-leds 
INCLUDES         += -I../wsn430-clock 
INCLUDES         += -I../wsn430-timer
INCLUDES         += -I../wsn430-serial
INCLUDES         += -I../wsn430-m25p80
INCLUDES         += -I../wsn430-ds2411

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile


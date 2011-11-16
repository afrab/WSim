
# define one name for each executable to be built
NAMES      = spi
CPU        = -mmcu=msp430x1611
UTILS_PATH = ../../utils

# sources files specific to 'first_target'
SRC              = main.c spi.c timer.c uart.c clock.c leds.c
INCLUDES         = -I. 

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile



# define one name for each executable to be built
NAMES      = uart
CPU        = -mmcu=msp430x135
UTILS_PATH = ../../utils

# sources files specific to 'first_target'
SRC              = main.c serialComm.c
ASM              = taskhandler.S asmlib.S
INCLUDES         = -I. 

# the makefile common will generate NAMES.hex and NAMES.elf files, and define 'all' and 'clean' directives
-include ${UTILS_PATH}/mspgcc.makefile


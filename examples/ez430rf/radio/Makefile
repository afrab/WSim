NAME		= radio
LIBS		= -lez430
SRC		= main.c
SRC_DIR		= src
INC_DIR		= -I../../inc
OUT_DIR		= bin
LIB_DIR		= ../../lib
OBJ_DIR		= .obj
DOC_DIR		= doc
DEP_DIR 	= .deps
OBJ		= $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
OBJTX           = $(OBJ_DIR)/main-tx.o
OBJRX           = $(OBJ_DIR)/main-rx.o
DEPS		= $(patsubst %.c,$(DEP_DIR)/%.d,$(SRC))
# Platform EZ430
CPU		= msp430x2274
CFLAGS		= -g -Wall -mmcu=${CPU} ${INC_DIR}
LDFLAGS		= -static -L${LIB_DIR} ${LIBS}
CC		= msp430-gcc
#MAKEDEPEND	= ${CC} ${CFLAGS} -MM -MP -MT $@ -MF ${DEP_DIR}/$*.d

all: ${OUT_DIR}/${NAME}-tx.elf ${OUT_DIR}/${NAME}-rx.elf 

download: all
#	mspdebug -R "prog ${OUT_DIR}/${NAME}.elf"
	@echo "Pour flasher, utiliser une des deux commandes:"
	@echo "    mspdebug uif -d /dev/ttyUSB0 'prog bin/radio-tx.elf'"
	@echo "    mspdebug uif -d /dev/ttyUSB0 'prog bin/radio-rx.elf'"

${OUT_DIR}/${NAME}-tx.elf: ${OBJTX}
	@mkdir -p ${OUT_DIR}
	${CC} -mmcu=${CPU} ${OBJTX} ${LDFLAGS} -o $@

${OUT_DIR}/${NAME}-rx.elf: ${OBJRX}
	@mkdir -p ${OUT_DIR}
	${CC} -mmcu=${CPU} ${OBJRX} ${LDFLAGS} -o $@

${OBJ_DIR}/main-tx.o: ${SRC_DIR}/main.c
	@mkdir -p ${OBJ_DIR} ${DEP_DIR}
	#${MAKEDEPEND} $<
	${CC} ${CFLAGS} -DTX -MM -MP -MT $@ -MF ${DEP_DIR}/main-tx.d $<
	${CC} ${CFLAGS} -DTX -c $< -o $@

${OBJ_DIR}/main-rx.o: ${SRC_DIR}/main.c
	@mkdir -p ${OBJ_DIR} ${DEP_DIR}
	#${MAKEDEPEND} $<
	${CC} ${CFLAGS} -DRX -MM -MP -MT $@ -MF ${DEP_DIR}/main-rx.d $<
	${CC} ${CFLAGS} -DRX -c $< -o $@

-include ${DEPS}

.PHONY: clean
clean:
	@rm -Rf ${OUT_DIR} ${OBJ_DIR} ${DEP_DIR} ${DOC_DIR}

.PHONY: rebuild
rebuild: clean all

.PHONY: doc
doc:
	doxygen


#! /bin/sh


INCLUDE="-I. -Iarch -Iarch/common -Iarch/msp430 -Idevice -Igdb -Ilogger -Imachine -Isrc -Itracer"
PREPROC="-DLINUX -DMSP430f1611"
#OUTPUT="-showscan -showalluses -stats"
MODE="-strict -isoreserved -exportfcn"
CMD="splint ${INCLUDE} ${PREPROC} ${OUTPUT} ${MODE} $*"

echo ${CMD}
${CMD}


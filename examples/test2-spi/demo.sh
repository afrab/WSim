#! /bin/sh

rm -f trace*.eps trace*.gp

WSIM=../../../build/wsim-debug/platforms/tests/wsim-msp1611-2
WTRC=../../../build/wtracer/src/wtracer

PSER="--serial1_io=/dev/null"
MODE="--mode=time --modearg=10s"
#MODE="--mode=gdb"
#UI="--ui"

${WSIM} ${UI} ${MODE} ${PSER} --trace=wsim.trc --logfile=stdout --verbose=4 test2-spi.elf
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd

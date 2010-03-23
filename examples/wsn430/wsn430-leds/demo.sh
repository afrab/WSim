#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer

LOG="--logfile=wsim.log --verbose=2"
TRC="--trace=wsim.trc"
MODE="--mode=time --modearg=30s"
#MODE="--mode=gdb"
UI="--ui"
## ======================================


## =============WSIM=====================
WS1="${WSIM} ${UI} ${MODE} ${LOG} ${TRC} ./wsn430-leds.elf"
xterm -T wsim-1 -e "${WS1}" &
echo "${WS1}"
## ======================================


## =============Wait=====================
read dummyval
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
## ======================================


## =============Traces===================
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd
## ======================================

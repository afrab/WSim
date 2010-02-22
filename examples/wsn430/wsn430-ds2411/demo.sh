#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer

LOG="--logfile=wsim.log --verbose=2"
TRC="--trace=wsim.trc"
SERIAL="--serial1_io=stdout"
MODE="--mode=time --modearg=10s"
DS="--ds2411=0a:00:00:00:00:00:01:01"
#MODE="--mode=gdb"
UI=""
## ======================================


## =============WSIM=====================
WS1="${WSIM} ${MODE} ${LOG} ${TRC} ${DS} ${SERIAL} ./wsn430-ds2411.elf"
xterm -T wsim-1 -e "${WS1}" &
echo "${WS1}"
## ======================================


## =============Wait=====================
read dummyval
## ======================================


## =============Traces===================
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
## ======================================


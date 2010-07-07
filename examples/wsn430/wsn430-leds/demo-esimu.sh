#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430-esimu
WTRC=wtracer

LOG="--logfile=wsim.log --verbose=2"
TRC="--trace=wsim.trc"
UI="--ui"
MODE="--mode=time --modearg=30s"

ESIMU=esimu-ng
CALIB=wsn430.conf
EOPT="--esimu-start --esimu=wsim.etr"
## ======================================


## ============ Simulation ==============
WS1="${WSIM} ${UI} ${MODE} ${LOG} ${TRC} ${EOPT} ./wsn430-leds.elf"
echo "${WS1}"
#xterm -T wsim-1 -e "${WS1}" 
## ======================================


## ============ Traces ==================
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd
## ======================================


## ============ eSimu ===================
$ESIMU --in=wsim.etr --conf=$CALIB --out=n0.dat --mode=tlin --exec=wsn430-leds.elf
$ESIMU --in=wsim.etr --conf=$CALIB --out=n0.prf --mode=prof --exec=wsn430-leds.elf
## ======================================


#! /bin/sh

WSIM=wsim-msp135
WTRACE=wtracer
#ESIMU=${BASE}/os_basse_conso/tools/skyeye/eSimu/src/esimu

TIME=5s

UI=""
MODE="--mode=time --modearg=${TIME}"
LOGS="--verbose=1 --logfile=wsim.log"
TRACE="--trace=wsim.trc"
SERIAL="--serial0_io=stdout"

W="${WSIM} ${UI} ${MODE} ${SERIAL} ${TRACE} ${LOGS} insn.elf"
echo $W
time $W

${WTRACE} --in=wsim.trc --out=wsim.vcd --format=vcd

# ${ESIMU} --bin=wsim.etr --out=wsim.prof --exec=insn.elf

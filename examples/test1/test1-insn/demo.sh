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
${WTRACE} --in=wsim.trc --out=wsim.gp  --format=gplot
gnuplot < wsim.gp

# ${ESIMU} --bin=wsim.etr --out=wsim.prof --exec=insn.elf



# ascci 
# 45 Mo
# real    0m3.194s
# user    0m2.824s
# sys     0m0.324s

# binaire:
# 24515062 ~ 24Mo
# 15s

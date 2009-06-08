#! /bin/sh

BASE=${HOME}/projets

WBASE=${BASE}/worldsens/build

WSIM=${WBASE}/wsim-etrace/platforms/tests/wsim-test1
WCONSOLE=${WBASE}/wconsole/src/wconsole
WTRACE=${WBASE}/wtracer/src/wtracer

ESIMU=${BASE}/os_basse_conso/tools/skyeye/eSimu/src/esimu

TIME=$(( 5 * 1000000000 ))

time ${WSIM} --ui --serial0=stdout --mode=time --modearg=${TIME} --trace=wsim.trc --esimu=wsim.etr --logfile=wsim.log insn.elf
${WTRACE} --in=wsim.trc --out=wsim.gp --format=gplot
gnuplot < wsim.gp

${ESIMU} --bin=wsim.etr --out=wsim.prof --exec=insn.elf



# ascci 
# 45 Mo
# real    0m3.194s
# user    0m2.824s
# sys     0m0.324s

# binaire:
# 24515062 ~ 24Mo
# 15s

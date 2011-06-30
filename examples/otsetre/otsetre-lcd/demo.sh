#! /bin/sh

WSIM=wsim-otsetre
WTRC=wtracer

${WSIM} --msp430_trc_sp --ui --trace=wsim.trc --logfile=stdout --verbose=4 display.elf 
${WTRC} --debug --in=wsim.trc --out=wsim.vcd --format=vcd


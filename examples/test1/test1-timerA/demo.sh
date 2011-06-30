#! /bin/sh

WSIM=wsim-msp1611

${WSIM} --mode=run --modearg=10s --verbose=1 --ui --serial0_io=stdout --trace=wsim.trc timerA.elf 

wtracer --in=wsim.trc --out=wsim.vcd --format=vcd

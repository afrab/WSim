#! /bin/sh

wsim-msp135 --verbose=1 --ui --serial0_io=stdout --trace=wsim.trc timerA.elf 

wtracer --in=wsim.trc --out=wsim.vcd --format=vcd

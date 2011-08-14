#! /bin/sh

WSIM=wsim-telosb
WTRC=wtracer

WS=""
WS="${WS} --verbose=2 --ui"
WS="${WS} --serial1_io=stdout"
WS="${WS} --mode=time --modearg=10s"
WS="${WS} --trace=wsim.trc" 

${WSIM}  ${WS}  telosb-leds.elf 
${WTRC}  --in=wsim.trc --out=wsim.vcd --format=vcd

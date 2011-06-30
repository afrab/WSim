#! /bin/sh

WSIM=wsim-msp1611
MODE=" --mode=run --modearg=1s "

if [ "$1" = "gdb" ] ; then
    MODE=" --mode=gdb "
fi

${WSIM} ${MODE} --verbose=6 --logfile=stdout --serial0_io=stdout --trace=wsim.trc timings.elf 

wtracer --in=wsim.trc --out=wsim.vcd --format=vcd

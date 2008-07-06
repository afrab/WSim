#! /bin/sh

rm -f trace*.eps trace*.gp trace*.jpg

CALLGRINDOPTS="--dump-line=yes --dump-instr=yes --trace-jump=yes --collect-systime=yes --collect-jumps=yes --combine-dumps=yes"
DEBUG="callgrind ${CALLGRINDOPTS}"
DEBUG=""

${DEBUG} ./wsim --ui --serial1=$1 --trace=trace --tracemode=gplot  uart.elf 

GP=`echo *.gp`
if [ "$GP" != "*.gp" ] ; then 
    for i in ${GP} ; do 
	echo "============= $i"
        gnuplot < $i 
    done
fi

EPS=`echo *.eps`
if [ "$EPS" != "*.eps" ] ; then 
    for i in ${EPS} ; do 
	gv $i & 
    done
fi



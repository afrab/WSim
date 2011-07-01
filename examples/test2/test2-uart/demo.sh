#! /bin/sh

rm -f trace*.eps trace*.gp

gnuplot_build()
{    
    GP=`echo *.gp`
    if [ "$GP" != "*.gp" ] ; then 
	for i in ${GP} ; do 
	    gnuplot < $i 
	done
    fi
}

gnuplot_show()
{
    EPS=`echo *.eps`
    if [ "$EPS" != "*.eps" ] ; then 
	for i in ${EPS} ; do 
	    gv $i & 
	done
    fi
}

CALLGRINDOPTS="--dump-line=yes --dump-instr=yes --trace-jump=yes --collect-systime=yes --collect-jumps=yes --combine-dumps=yes"
DEBUG="callgrind ${CALLGRINDOPTS}"
DEBUG=""

if [ "$1" != "" ] ; then 
    ${DEBUG}  ./wsim --ui --serial0=$1 --trace=trace --tracemode=gplot  uart.elf 
else
    ${DEBUG}  ./wsim --ui --serial0=stdout --trace=trace --tracemode=gplot  uart.elf 
fi 
    

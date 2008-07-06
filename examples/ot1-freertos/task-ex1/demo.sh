#! /bin/sh


traces()
{
    GP=`echo trace$2*.gp`
    if [ "$GP" != "trace$2*.gp" ] ; then 
	for i in ${GP} ; do 
	    gnuplot < $i 
	done
    fi

    EPS=`echo trace$2.*.eps`
    if [ "$EPS" != "trace$2*.eps" ] ; then 
	for i in ${EPS} ; do 
	    gv $i & 
	done
    fi
}

rm -f trace$2*.eps trace$2*.gp trace$2*.jpg

CALLGRINDOPTS="--dump-line=yes --dump-instr=yes --trace-jump=yes --collect-systime=yes --collect-jumps=yes --combine-dumps=yes"
DEBUG="callgrind $CALLGRINDOPTS"

DEBUG=""
# --tracemode=gplot

${DEBUG} ./wsim --ui --trace=trace$2 rtos.elf

gnuplot < trace$2.2.gp
gv trace$2.Interrupt.eps

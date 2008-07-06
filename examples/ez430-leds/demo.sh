#! /bin/sh

gnuplot_data()
{
    GP=`echo trace$1*.gp`
    if [ "$GP" != "trace$1*.gp" ] ; then 
	for i in ${GP} ; do 
	    echo "===== $i"
	    gnuplot < $i 
	done
    fi
    
    EPS=`echo trace$1*.eps`
    if [ "$EPS" != "trace$1*.eps" ] ; then 
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

${DEBUG} ./wsim --verbose=1 --ui  ez430-leds.elf 


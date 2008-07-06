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
WSIM=../../../build/wsim-debug/platforms/wsn430/wsim-wsn430
# --tracemode=gplot

# ${DEBUG} ${WSIM} --verbose=2 --ui --serial=stdout --mode=time --modearg=$(( 5 * 1000000000 )) wsn430-leds.elf 
${DEBUG} ${WSIM} --verbose=2 --ui --serial=stdout --mode=gdb wsn430-leds.elf 


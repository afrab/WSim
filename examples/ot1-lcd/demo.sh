#! /bin/sh

rm -f trace*.eps trace*.gp trace*.jpg

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
WSIM=/home/antoine/projets/worldsens/build/wsim-debug/platforms/ot2007/wsim-ot2007

if [ "$1" != "" ] ; then 
    ${DEBUG} ${WSIM} --ui --serial=$1     --trace=trace --mode=run --ui --logfile=stdout lcd.elf
else
    ${DEBUG} ${WSIM} --ui --serial=stdout --trace=trace --mode=run --ui --logfile=stdout lcd.elf
fi 



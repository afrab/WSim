#! /bin/sh

## set -x
source ../config.soft

XGEOMETRY="-geometry 70x12"
XTERM="xterm ${XGEOMETRY}"
## ==================================

#C1=`run_console -l c1.log`
C1=stdout

sync
echo "consoles $C1"
echo "== Press the any key (enter) =============="
read 

## ==================================
export SETUI=true
TIME=$(( 20 * $FACTOR ))
EXTRA=echo

export SETUI="yes"
export SETESIMU="yes"

if [ "$1" == "debug" ] ; then 
    WS1="`run_wsim_gdb $DS1 wsn430-timer.elf $TIME $C1`" 
    EXTRA="msp430-insight wsn430-timer.elf"
else
    WS1="`run_wsim $DS1 wsn430-timer.elf $TIME $C1`" 
fi 

echo "${WS1}"
${XTERM} -T wsim-1 -e "$WS1" &
${EXTRA} 

read

## ==================================
## ==================================

kill_demo


#! /bin/bash

## set -x
. ../config.soft

## ==================================

xterm -T wsnet -e "${WSNET} -F config -l wsnet.log"   &

C1=`run_console -l c1.log`
C2=`run_console -l c2.log`
C3=`run_console -l c3.log`

#C1=stdout
#C2=stdout
#C3=stdout

sync
echo "consoles $C1 $C2 $C3"

## ==================================

TIME=$(( 5 * $FACTOR ))

WS1="`run_wsimnet 1 $DS1 cc1100-26MHz.elf $TIME $C1`" 
WS2="`run_wsimnet 2 $DS2 cc1100-26MHz.elf $TIME $C2`"
WS3="`run_wsimnet 3 $DS3 cc1100-26MHz.elf $TIME $C3`"

echo "${WS1}"
echo "${WS2}"
echo "${WS3}"
xterm -T wsim-1 -e "$WS1" &
xterm -T wsim-2 -e "$WS2" &
xterm -T wsim-3 -e "$WS3" &

read

## ==================================
## ==================================

kill_demo

#! /bin/sh

## set -x
source ../config.soft

## ==================================

C1=`run_console -l c1.log`
C1=`tmp_console $C1`
sync
echo "consoles $C1"
echo "== Press the any key (enter) =============="
read 

## ==================================
export SETUI=true
TIME=$(( 7 * $FACTOR ))

WS1="`run_wsim $DS1 wsn430-serial.elf $TIME $C1`" 

echo "${WS1}"
xterm -T wsim-1 -e "$WS1" &

read

## ==================================
## ==================================

kill_demo


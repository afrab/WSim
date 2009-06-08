#! /bin/bash

## set -x
. ../config.soft

## ==================================
export SETUI=true
#C1=`run_console -l c1.log`
C1=stdout

sync
echo "consoles $C1"

## ==================================

TIME=$(( 7 * $FACTOR ))

WS1="`run_wsim $DS1 wsn430-m25p80.elf $TIME $C1`" 

echo "${WS1}"
xterm -T wsim-1 -e "$WS1" &

read

## ==================================
## ==================================

kill_demo


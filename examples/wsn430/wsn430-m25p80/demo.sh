#! /bin/sh

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE=""
export WSNET2_CONF_PATH=""

## ==================================

. ../config.soft

## ================================

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

read dummyval

## ==================================
## ==================================

kill_demo


#! /bin/sh

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE=""
export WSNET2_CONF_PATH=""

## ==================================

. ../config.soft

## ==================================

C1=`run_console -l c1.log`

sync
echo "consoles $C1"

## ==================================

export SETUI=true
TIME=$(( 7 * $FACTOR ))

WS1="`run_wsim $DS1 wsn430-serial.elf $TIME $C1`" 

echo "${WS1}"
xterm -T wsim-1 -e "$WS1" &

read dummyval

## ==================================
## ==================================

kill_demo


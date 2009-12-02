#! /bin/sh

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE=""
export WSNET2_CONF_PATH=""

## ==================================

. ../config.soft

## ==================================

export SETUI=true

TIME=$(( 30 * $FACTOR ))
SERIAL_IO="no-serial"

WS1="`run_wsim $DS1 wsn430-clock.elf $TIME $SERIAL_IO`" 

echo "${WS1}"
xterm -T wsim-1 -e "$WS1" &

read dummyval

## ==================================
## ==================================

kill_demo

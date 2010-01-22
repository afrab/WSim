#! /bin/bash

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE="wsnet1"
export WSNET2_CONF_PATH="worldsens.xml"

NBNODES=4

## ==================================

. ../config.soft

## ==================================

xterm -T wsnet -e "${WSNET} ${WSNET_CONF}"   &
echo "${WSNET} ${WSNET_CONF}"

## ==================================

for ((i=0 ; i<NBNODES; i++))
  do 
    C[$i]=`run_console -l c$i.log`
    echo "Console: ${C[$i]}"
  done

sync

## ==================================

TIME=$(( 10 * $FACTOR ))
SETUI="OK"

for ((i=0 ; i<NBNODES; i++))
  do
    DSi=`eval echo \"$\DS$i\"`
    WS[$i]="`run_wsim $DSi ./senslabv14-demo-token$i.elf $TIME ${C[i]} $i`"
    echo "${WS[i]}"
  done

for ((i=0 ; i<NBNODES; i++))
  do
    xterm -T wsim-$i -e "${WS[i]}" &
    sleep 0.5
  done

## ==================================

read

kill_demo

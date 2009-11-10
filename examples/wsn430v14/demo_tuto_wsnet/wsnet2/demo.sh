#! /bin/bash

. ./config.soft

NBNODES=4

xterm -T wsnet -e "${WSNET} -c worldsens.xml"   &

for ((i=0 ; i<NBNODES; i++))
  do C[$i]=`run_console -l c$i.log`
done

sync

TIME=$(( 10 * $FACTOR ))

SETUI="OK"

for ((i=0 ; i<NBNODES; i++))
  do
    WS[$i]="`run_wsimnet $i $DS$i ../senslabv14-demo-token$i.elf $TIME ${C[i]}`"
    echo "${WS[i]}"
  done

for ((i=0 ; i<NBNODES; i++))
  do
    xterm -T wsim-$i -e "${WS[i]}" &
    sleep 0.5
  done

read

kill_demo

#! /bin/sh

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE="wsnet2"
export WSNET2_CONF_PATH="worldsens.xml"

## ==================================

. ../config.soft

## ==================================

xterm -T wsnet -e "${WSNET} ${WSNET_CONF}"   &

echo "${WSNET} ${WSNET_CONF}"

C1=`run_console -l c1.log`
C2=`run_console -l c2.log`
C3=`run_console -l c3.log`

echo "consoles $C1 $C2 $C3"

## ==================================

TIME=$(( 5 * $FACTOR ))

WS1="`run_wsim $DS1 cc1100-26MHz.elf $TIME $C1 1`" 
WS2="`run_wsim $DS2 cc1100-26MHz.elf $TIME $C2 2`"
WS3="`run_wsim $DS3 cc1100-26MHz.elf $TIME $C3 3`"

echo "${WS1}"
echo "${WS2}"
echo "${WS3}"
xterm -T wsim-1 -e "$WS1" &
xterm -T wsim-2 -e "$WS2" &
xterm -T wsim-3 -e "$WS3" &

read dummy_val

dump_trace n1
dump_trace n2
dump_trace n3

#gnuplot < n1.gp
#${WTRC} --in=n1.trc --out=n1.vcd --format=vcd
#dump_esimu_trace n1 -p wsn430-demo.elf > esimu.log 2>&1

## ==================================
## ==================================

kill_demo

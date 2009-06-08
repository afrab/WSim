#! /bin/bash

## set -x
. ../config.soft

## ==================================

xterm -T wsnet -e "${WSNET} -F config -l wsnet.log"   &

C1=`run_console -l c1.log`
C2=`run_console -l c2.log`
C3=`run_console -l c3.log`

echo "consoles $C1 $C2 $C3"

## ==================================

export SETUI=yes
export SETESIMU=yes

TIME=$(( 20 * $FACTOR ))

if [ "$1" != "dbg" ] ; then
	WS1="`run_wsimnet 1 $DS1 wsn430-demo.elf $TIME $C1`" 
	WS2="`run_wsimnet 2 $DS2 wsn430-demo.elf $TIME $C2`"
	WS3="`run_wsimnet 3 $DS3 wsn430-demo.elf $TIME $C3`"
else
	case $2 in
		1)
		WS1="`run_wsimnet_gdb 1 $DS1 wsn430-demo.elf $TIME $C1`"
		WS2="`run_wsimnet 2 $DS2 wsn430-demo.elf $TIME $C2`"
		WS3="`run_wsimnet 3 $DS3 wsn430-demo.elf $TIME $C3`"
		;;
		2)
		WS2="`run_wsimnet_gdb 2 $DS2 wsn430-demo.elf $TIME $C2`"
		WS1="`run_wsimnet 1 $DS1 wsn430-demo.elf $TIME $C1`"
		WS3="`run_wsimnet 3 $DS3 wsn430-demo.elf $TIME $C3`"
		;;
		3)
		WS3="`run_wsimnet_gdb 3 $DS3 wsn430-demo.elf $TIME $C3`"
		WS1="`run_wsimnet 1 $DS1 wsn430-demo.elf $TIME $C1`"
		WS2="`run_wsimnet 2 $DS2 wsn430-demo.elf $TIME $C2`"
		;;
	esac
fi

echo "${WS1}"
echo "${WS2}"
echo "${WS3}"
xterm -T wsim-1 -e "$WS1" &
xterm -T wsim-2 -e "$WS2" &
xterm -T wsim-3 -e "$WS3" &

read

dump_trace n1
dump_trace n2
dump_trace n3

gnuplot < n1.gp
${WTRC} --in=n1.trc --out=n1.vcd --format=vcd
dump_esimu_trace n1 -p wsn430-demo.elf > esimu.log 2>&1

## ==================================
## ==================================

kill_demo

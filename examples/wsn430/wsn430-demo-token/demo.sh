#! /bin/sh

## ==================================

# set WSNET_MODE to "wsnet1", "wsnet2", or "" if you are using wsim alone
export WSNET_MODE="wsnet1"
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

export SETUI=yes
export SETESIMU=yes

TIME=$(( 20 * $FACTOR ))

if [ "$1" != "dbg" ] ; then
	WS1="`run_wsim $DS1 wsn430-demo.elf $TIME $C1 1`" 
	WS2="`run_wsim $DS2 wsn430-demo.elf $TIME $C2 2`"
	WS3="`run_wsim $DS3 wsn430-demo.elf $TIME $C3 3`"
else
	case $2 in
		1)
		WS1="`run_wsim_gdb $DS1 wsn430-demo.elf $TIME $C1 1`"
		WS2="`run_wsim $DS2 wsn430-demo.elf $TIME $C2 2`"
		WS3="`run_wsim $DS3 wsn430-demo.elf $TIME $C3 3`"
		;;
		2)
		WS2="`run_wsim_gdb $DS2 wsn430-demo.elf $TIME $C2 2`"
		WS1="`run_wsim $DS1 wsn430-demo.elf $TIME $C1 1`"
		WS3="`run_wsim $DS3 wsn430-demo.elf $TIME $C3 3`"
		;;
		3)
		WS3="`run_wsim_gdb $DS3 wsn430-demo.elf $TIME $C3 3`"
		WS1="`run_wsim $DS1 wsn430-demo.elf $TIME $C1 1`"
		WS2="`run_wsim $DS2 wsn430-demo.elf $TIME $C2 2`"
		;;
	esac
fi

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

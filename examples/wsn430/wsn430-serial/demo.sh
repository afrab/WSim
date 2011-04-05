#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer
NB_NODE=1

# set WSNET to "wsnet1", "wsnet2", or "" if you are using wsim alone
WSNET=""
WSNET2_CONF="./worldsens.xml"

UI="--ui"
LOG="--logfile=stdout --verbose=5"
TRC="--trace=wsim.trc"

# Mode = time | gdb
MODE="--mode=time --modearg=10s"
#MODE="--mode=gdb"

# Serial terminal emulation
#  stdout | UDP | TCP
SERMODE="UDP"

## ======================================

if [ "x`which nc.traditional`" = "x" ]
then
    NETCAT=nc
else
    NETCAT=nc.traditional
fi

case $SERMODE in
    "stdout")
	SERIAL="--serial1_io=stdout"
	NCCMD=""
	;;
    "UDP")
	NCCMD="${NETCAT} -u -p 7000 localhost 6000"
	SERIAL="--serial1_io=udp:localhost:6000:localhost:7000"
	;;
    "TCP")
	NCCMD="${NETCAT} localhost 6000"
	SERIAL="--serial1_io=bk:tcp:s:localhost:6000"
	;;
esac

## =============WSNET=====================
if [ "$WSNET" = "wsnet1" ]
then
    xterm -T ${WSNET} -e "${WSNET}"
    echo "${WSNET}"
else
    if [ "$WSNET_MODE" = "wsnet2" ]
    then
        xterm -T ${WSNET} -e "${WSNET} -c ${WSNET2_CONF}"
        echo "${WSNET} -c ${WSNET2_CONF}"
    fi
fi
## ======================================


## =============WSIM=====================
WS1="${WSIM} ${MODE} ${LOG} ${TRC} ${SERIAL} ./wsn430-serial.elf"
xterm -T wsim-1 -e "${WS1}" &
echo "${WS1}"
sleep 0.5
## ======================================


## =============NETCAT / SERIAL ==========
if [ "${NCCMD}" != "" ] ; then
    xterm -T netcat-1 -e "${NCCMD}" &
    echo "${NCCMD}"
fi
## ======================================

## =============Wait=====================
read dummyval
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${WSNET}  > /dev/null 2>&1
killall -SIGQUIT ${NETCAT} > /dev/null 2>&1
## ======================================


## =============Traces===================
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd
## ======================================

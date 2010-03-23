#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer

# set WSNET to "wsnet1", "wsnet2", or "" if you are using wsim alone
WSNET=
WSNET2_CONF="./worldsens.xml"
NB_NODE=1

LOG="--logfile=wsim.log --verbose=2"
TRC="--trace=wsim.trc"
SERIAL="--serial1_io=udp:localhost:6000:localhost:7000"
MODE="--mode=time --modearg=10s"
#MODE="--mode=gdb"
UI="--ui"
## ======================================


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


## =============NETCAT====================
xterm -T netcat-1 -e "nc -u -p 7000 localhost 6000" &
echo "nc -u -p 7000 localhost 6000"
## ======================================


## =============WSIM=====================
WS1="${WSIM} ${MODE} ${LOG} ${TRC} ${SERIAL} ./wsn430-serial.elf"
xterm -T wsim-1 -e "${WS1}" &
echo "${WS1}"
## ======================================


## =============Wait=====================
read dummyval
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${WSNET}  > /dev/null 2>&1
killall -SIGQUIT nc        > /dev/null 2>&1
## ======================================


## =============Traces===================
${WTRC} --in=wsim.trc --out=wsim.vcd --format=vcd
## ======================================

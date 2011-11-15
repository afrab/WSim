#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer
WSNET1=wsnet1
WSNET2=wsnet

if [ "x`which nc.traditional`" = "x" ]
then
    NETCAT=nc
else
    NETCAT=nc.traditional
fi

# set WSNET to "--wsnet1", "--wsnet2", or "" if you are using wsim alone
WSNET_MODE=--wsnet1
WSNET2_CONF="./worldsens.xml"
NB_NODE=3

LOG="--verbose=2"
MODE="--mode=time --modearg=10s"
#MODE="--mode=gdb"
UI="--ui"

DS0=0a:00:00:00:00:00:00:01
DS1=0a:00:00:00:00:00:01:01
DS2=53:00:00:00:00:00:02:01
DS3=64:00:00:00:00:00:03:01
DS4=e1:00:00:00:00:00:04:01
DS5=d6:00:00:00:00:00:05:01
DS6=8f:00:00:00:00:00:06:01
DS7=b8:00:00:00:00:00:07:01
DS8=9c:00:00:00:00:00:08:01
DS9=ab:00:00:00:00:00:09:01
DS10=f2:00:00:00:00:00:0a:01
## ======================================


## =============WSNET=====================
if [ "$WSNET_MODE" = "--wsnet1" ]
then
    xterm -T ${WSNET1} -e "${WSNET1}" &
    echo "${WSNET1}"
else
    if [ "$WSNET_MODE" = "--wsnet2" ]
    then
        xterm -T ${WSNET2} -e "${WSNET2} -c ${WSNET2_CONF}" &
        echo "${WSNET2} -c ${WSNET2_CONF}"
    fi
fi
## ======================================

sleep 0.5

## =============NETCAT====================
iter=0
while [ ${iter} -lt ${NB_NODE} ]
do
    NC="${NETCAT} -u -p 700${iter} localhost 600${iter}"
    xterm -T netcat-${iter} -e "${NC}" &
    echo "${NC}"
    iter=`expr ${iter} + 1`
done
## ======================================


## =============WSIM=====================
iter=0
while [ ${iter} -lt ${NB_NODE} ]
do 
    eval DS="\$DS${iter}"
    WS="${WSIM} ${MODE} ${WSNET_MODE} ${LOG} ${TRC} --logfile=n${iter}.log --trace=n${iter}.trc --serial1_io=bk:udp:localhost:600${iter}:localhost:700${iter} --node-id=${iter} --ds2411=${DS} ./wsn430-cc1100.elf"
    xterm -T wsim-${iter} -e "${WS}" &
    echo "${WS}"
    iter=`expr ${iter} + 1`
done
## ======================================


## =============Wait=====================
read dummyval
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${WSNET1} > /dev/null 2>&1
killall -SIGQUIT ${WSNET2} > /dev/null 2>&1
killall -SIGUSR1 ${NETCAT} > /dev/null 2>&1
## ======================================


## =============Traces===================
iter=0
while [ ${iter} -lt ${NB_NODE} ]
do
    ${WTRC} --in=n${iter}.trc --out=n${iter}.vcd --format=vcd
    echo "${WTRC} --in=n${iter}.trc --out=n${iter}.vcd --format=vcd"
    iter=`expr ${iter} + 1` 
done
## ======================================

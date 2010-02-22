#! /bin/sh

## =============Conf=====================
WSIM=wsim-wsn430
WTRC=wtracer
WSNET1=wsnet1
WSNET2=wsnet

# set WSNET to "--wsnet1", "--wsnet2", or "" if you are using wsim alone
WSNET_MODE=--wsnet1
WSNET2_CONF="./worldsens.xml"
NB_NODE=3

LOG="--verbose=2"
MODE="--mode=time --modearg=10s"
#MODE="--mode=gdb"
UI="--ui"
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


## =============NETCAT====================
iter=0
while [ ${iter} -lt ${NB_NODE} ]
do
    NC="nc -u -p 700${iter} localhost 600${iter}"
    xterm -T netcat-${iter} -e "${NC}" &
    echo "${NC}"
    iter=`expr ${iter} + 1`
done
sync
## ======================================


## =============WSIM=====================
iter=0
while [ ${iter} -lt ${NB_NODE} ]
do
    WS="${WSIM} ${MODE} ${WSNET_MODE} ${LOG} ${TRC} --logfile=n${iter}.log --trace=n${iter}.trc --serial1_io=bk:udp:localhost:600${iter}:localhost:700${iter} --node-id=${iter} --ds2411=0a:00:00:00:0${iter}:0${iter}:0${iter}:01 ./cc1100-26MHz.elf"
    xterm -T wsim-${iter} -e "${WS}" &
    echo "${WS}"
    iter=`expr ${iter} + 1`
done
## ======================================


## =============Wait=====================
read dummyval
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


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${WSNET}  > /dev/null 2>&1
killall -SIGUSR1 nc        > /dev/null 2>&1
## ======================================


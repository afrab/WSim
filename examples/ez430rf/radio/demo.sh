#! /bin/sh

## =============Conf=====================
WSIM=wsim-ez430rf
WSNET1=wsnet1
WSNET_MODE="--wsnet1"
NB_NODE=2
LOG="--verbose=6 --logfile=stdout"
MODE="--mode=time --modearg=10s"
UI="--ui"
#UI=""



## =============WSNET=====================
xterm -T ${WSNET1} -e "${WSNET1}" &
echo "${WSNET1}"

## ======================================

sleep 0.5


## =============WSIM=====================
iter=1
while [ ${iter} -le ${NB_NODE} ]
do

    WS="${WSIM} ${MODE} ${WSNET_MODE} ${LOG} ${UI}"
    WS="${WS} --serial0_io=stdout"
    WS="${WS} --node-id=${iter}"
    if [ "${iter}" = "1" ] ; then 
	WS="${WS} bin/radio-tx.elf"
    else
	WS="${WS} bin/radio-rx.elf"
    fi	

    xterm -T wsim-${iter} -e "${WS}" &
    echo "${WS}"
    iter=`expr ${iter} + 1`
    sleep 0.5
done
## ======================================


## =============Wait=====================
read dummyval
## ======================================


## =============End======================
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${WSNET1} > /dev/null 2>&1

## ======================================



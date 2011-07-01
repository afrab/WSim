#! /bin/sh

# Serial terminal emulation
#  stdout | UDP | TCP
SERMODE="UDP"

if [ "x`which nc.traditional`" = "x" ]
then
    NETCAT=nc
else
    NETCAT=nc.traditional
fi

case $SERMODE in
    "stdout")
        NCCMD=""
        SERIAL="--serial0_io=stdout"
        ;;
    "UDP")
        NCCMD="${NETCAT} -u -p 7000 localhost 6010"
        SERIAL="--serial0_io=udp:localhost:6010:localhost:7000"
        ;;
    "TCP")
        NCCMD="${NETCAT} localhost 6000"
        SERIAL="--serial0_io=bk:tcp:s:localhost:6000"
        ;;
esac

## ##################################################
## ##################################################

WSIM=wsim-msp1611

CMD="${WSIM} --ui ${SERIAL} --trace=wsim.trc nn4.elf"
echo $CMD
$CMD &

## ##################################################
## ##################################################

if [ "${NCCMD}" != "" ] ; then
    xterm -T netcat-1 -e "${NCCMD}" &
    echo "${NCCMD}"
fi

## ##################################################
## ##################################################

read dummyval
killall -SIGUSR1 ${WSIM}   > /dev/null 2>&1
killall -SIGQUIT ${NETCAT} > /dev/null 2>&1

wtracer --in=wsim.trc --out=wsim.vcd --format=vcd


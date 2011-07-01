#! /bin/sh

# Serial terminal emulation
#  stdout | UDP | TCP | wconsole
SERMODE="wconsole"

if [ "x`which nc.traditional`" = "x" ]
then
    NETCAT=nc
else
    NETCAT=nc.traditional
fi

create_fifo()
{
    if [ ! -e $1 ] ; then 
        mkfifo $1
    fi
}

case $SERMODE in
    "stdout")
        NCCMD=""
        SERIAL="--serial1_io=stdout"
        ;;
    "UDP")
        NCCMD="${NETCAT} -u -p 7000 localhost 6010"
        SERIAL="--serial1_io=udp:localhost:6010:localhost:7000"
        ;;
    "TCP")
        NCCMD="${NETCAT} localhost 6000"
        SERIAL="--serial1_io=bk:tcp:s:localhost:6000"
        ;;
    "wconsole")
        create_fifo towsim
        create_fifo fromwsim
        NCCMD=""
        wconsole -m o -f fromwsim & 
        wconsole -m i -f towsim   &
        SERIAL="--serial1_io=bk:fromwsim,towsim"
        ;;
esac

## ##################################################
## ##################################################

WSIM=wsim-msp1611

CMD="${WSIM} --ui ${SERIAL} --trace=wsim.trc uart.elf"
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
if [ "${SERMODE}" = "wconsole" ] ; then 
    killall -9 wconsole > /dev/null 2>&1 
    rm -f fromwsim towsim
fi

wtracer --in=wsim.trc --out=wsim.vcd --format=vcd



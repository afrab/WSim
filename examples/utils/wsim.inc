#! /bin/false         ## -*-shell-script-*-

## Variables
##
## 
## ELF       = 
## PLATFORM  = 
## 
## VERBOSE   = 2
## LOGFILE   = wsim.log
## TRACEFILE = wsim.trc
## GUI       = yes
## 
## MODE      = time
## TIME      = 30s

WTRC=wtracer
WSNET=wsnet1.undefined

#NETCAT=nc.undefined
if [ "x`which nc.traditional`" = "x" ] ; then
    NETCAT=nc
else
    NETCAT=nc.traditional
fi

# ESIMU=esimu-ng
# EOPT="--esimu-start --esimu=wsim.etr"
# CALIB=wsn430.conf
# $ESIMU --in=wsim.etr --conf=$CALIB --out=n0.dat --mode=tlin --exec=wsn430-leds.elf
# $ESIMU --in=wsim.etr --conf=$CALIB --out=n0.prf --mode=prof --exec=wsn430-leds.elf

declare -a SERIAL
declare -a SERIAL_TERM
declare -a DS24

## ########################################
## ## DS2411 serial ID ####################
## ########################################

DS24[0]=0a:00:00:00:00:00:00:01
DS24[1]=0a:00:00:00:00:00:01:01
DS24[2]=53:00:00:00:00:00:02:01
DS24[3]=64:00:00:00:00:00:03:01
DS24[4]=e1:00:00:00:00:00:04:01
DS24[5]=d6:00:00:00:00:00:05:01
DS24[6]=8f:00:00:00:00:00:06:01
DS24[7]=b8:00:00:00:00:00:07:01
DS24[8]=9c:00:00:00:00:00:08:01
DS24[9]=ab:00:00:00:00:00:09:01
DS24[10]=f2:00:00:00:00:00:0a:01

## ########################################
## ########################################
## ########################################

do_trace_if_any()
{
    if [ "x${TRACEFILE}" != "x" ] ; then
	${WTRC} --in=${TRACEFILE} --out=${TRACEFILE}.vcd --format=vcd
    fi
}

## ########################################
## ########################################
## ########################################

run_wsim_xterm()
{
    WSIM=wsim-${PLATFORM}
    _OPT=""

    if [ "x${VERBOSE}"   != "x" ] ; then _OPT="${_OPT} --verbose=${VERBOSE}" ; fi
    if [ "x${LOGFILE}"   != "x" ] ; then _OPT="${_OPT} --logfile=${LOGFILE}" ; fi
    if [ "x${TRACEFILE}" != "x" ] ; then _OPT="${_OPT} --trace=${TRACEFILE}" ; fi
    if [ "x${DS2411}"    != "x" ] ; then _OPT="${_OPT} --ds2411=${DS2411}"   ; fi
    if [ "x${SERIAL[0]}" != "x" ] ; then _OPT="${_OPT} --serial0_io=${SERIAL[0]}" ; fi
    if [ "x${SERIAL[1]}" != "x" ] ; then _OPT="${_OPT} --serial1_io=${SERIAL[1]}" ; fi

    case "${MODE}" in
	"time")
	    _OPT="${_OPT} --mode=time"
	    if [ "x${TIME}" != "x" ] ; then _OPT="${_OPT} --modearg=${TIME}" ; fi
	    ;;
	"insn")
	    _OPT="${_OPT} --mode=insn"
	    if [ "x${INSN}" != "x" ] ; then _OPT="${_OPT} --modearg=${INSN}" ; fi
	    ;;
	"run")
	    _OPT="${_OPT} --mode=run"
	    ;;
	"gdb")
	    _OPT="${_OPT} --mode=gdb"
	    if [ "x${GDBPORT}" != "x" ] ; then _OPT="${_OPT} --modearg=${GDBPORT}" ; fi
	    ;;
	*)
	    ;;
    esac 

    case "${GUI}" in
	"yes")
	    _OPT="${_OPT} --ui"
	    ;;
	*)
	    ;;
    esac

    _CMD="${WSIM} ${_OPT} ${ELF}"
    echo "${_CMD}"
    xterm -T wsim-1 -e "${_CMD}" 
}

## ########################################
## ########################################
## ########################################

run_serial_term()
{
    if [ "x${SERIAL_TERM[$1]}" != "x" ] ; then
	_CMD="xterm -T serial$1 -e ${SERIAL_TERM[$1]}"
	echo ${_CMD}
	${_CMD}
    fi
}

## ########################################
## ########################################
## ########################################

run_wsim()
{
    if [ ! -f ${ELF} ] ; then
	echo "Cannot find Elf file ${ELF}"
	exit 1
    fi

    run_wsim_xterm &

    # wait for wsim to start and listen on serial port if needed
    sleep 0.5

    run_serial_term 0 &
    run_serial_term 1 &

    # wait for everything to start until printing the message
    sleep 0.5

    echo "Press any key to end the simulation from the command line"
    read input

    killall -SIGUSR1 wsim-${PLATFORM} > /dev/null 2>&1
    killall -SIGQUIT ${WSNET}         > /dev/null 2>&1
    killall -SIGQUIT ${NETCAT}        > /dev/null 2>&1

    do_trace_if_any
}

## ########################################
## ########################################
## ########################################



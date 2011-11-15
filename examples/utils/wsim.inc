#! /bin/false

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

# ESIMU=esimu-ng
# EOPT="--esimu-start --esimu=wsim.etr"
# CALIB=wsn430.conf
# $ESIMU --in=wsim.etr --conf=$CALIB --out=n0.dat --mode=tlin --exec=wsn430-leds.elf
# $ESIMU --in=wsim.etr --conf=$CALIB --out=n0.prf --mode=prof --exec=wsn430-leds.elf

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

    case "${MODE}" in
	"time")
	    _OPT="${_OPT} --mode=time"
	    if [ "x${TIME}" != "x" ] ; then _OPT="${_OPT} --modearg=${TIME}" ; fi
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

    if [ ! -f ${ELF} ] ; then
	echo "Cannot find Elf file ${ELF}"
	exit 1
    fi

    _CMD="${WSIM} ${_OPT} ${ELF}"
    echo "${WS1}"
    xterm -T wsim-1 -e "${_CMD}" 
}

## ########################################
## ########################################
## ########################################

run_wsim()
{
    run_wsim_xterm &

    echo "Press any key to end the simulation"
    read input
    killall -SIGUSR1 ${WSIM} > /dev/null 2>&1

    do_trace_if_any
}

## ########################################
## ########################################
## ########################################



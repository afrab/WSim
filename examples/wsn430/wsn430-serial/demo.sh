#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=wsn430-serial.elf
PLATFORM=wsn430

VERBOSE=2
LOGFILE=stdout
TRACEFILE=wsim.trc
GUI=yes

MODE=time
TIME=60s

# Serial 0
SERIAL[0]=""
SERIAL_TERM[0]=""

# Serial 1
SERMODE=TCP
case "$SERMODE" in
    "stdout")
	SERIAL[1]="stdout"
	SERIAL_TERM[1]=""
	;;
    "UDP")
	SERIAL[1]="udp:localhost:6000:localhost:7000"
	SERIAL_TERM[1]="${NETCAT} -u localhost 6000"
	;;
    "TCP")
	SERIAL[1]="tcp:s:localhost:6000"
	SERIAL_TERM[1]="${NETCAT} localhost 6000"
	;;
    *)
	SERIAL[1]=""
	SERIAL_TERM[1]=""
	;;
esac

## ============= Run =====================

run_wsim

## =======================================


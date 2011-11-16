#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=reed.elf
PLATFORM=msp1611

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc

MODE=run
TIME=60s

# Serial 0
SERIAL[0]="tcp:s:localhost:6000"
SERIAL_TERM[0]="${NETCAT} localhost 6000"

## ============= Run =====================

run_wsim

## =======================================


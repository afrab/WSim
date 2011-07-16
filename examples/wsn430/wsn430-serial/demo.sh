#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=wsn430-serial.elf
PLATFORM=wsn430

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=yes

MODE=run
TIME=60s

SERIAL[1]="tcp:netcat"

## ============= Run =====================

run_wsim

## =======================================


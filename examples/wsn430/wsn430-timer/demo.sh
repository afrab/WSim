#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=wsn430-timer.elf
PLATFORM=wsn430

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=yes

MODE=time
TIME=60s

SERIAL[0]=""
SERIAL[1]="stdout"

## ============= Run =====================

run_wsim

## =======================================


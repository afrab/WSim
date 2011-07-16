#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=spi.elf
PLATFORM=msp1611-2

VERBOSE=4
LOGFILE=wsim.log
TRACEFILE=wsim.trc

MODE=run
TIME=60s

# Serial 0
SERIAL[1]="stdout"

## ============= Run =====================

run_wsim

## =======================================


#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=hwmul.elf
PLATFORM=msp1611

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=no

MODE=time
TIME=30s

SERIAL[0]="stdout"

## ============= Run =====================

run_wsim

## =======================================


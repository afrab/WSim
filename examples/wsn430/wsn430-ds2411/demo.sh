#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=wsn430-ds2411.elf
PLATFORM=wsn430
OPT=-O2

VERBOSE=6
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=yes

MODE=time
TIME=60s

SERIAL[0]=""
SERIAL[1]="stdout"

DS2411=${DS24[1]}

## ============= Run =====================

run_wsim

## =======================================


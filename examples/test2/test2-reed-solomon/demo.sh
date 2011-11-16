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

SERIAL[0]="tcp:netcat"

## ============= Run =====================

run_wsim

## =======================================


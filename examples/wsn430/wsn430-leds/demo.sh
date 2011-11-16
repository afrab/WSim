#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=wsn430-leds.elf
PLATFORM=wsn430

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=yes

MODE=time
TIME=30s

## ============= Run =====================

run_wsim

## =======================================


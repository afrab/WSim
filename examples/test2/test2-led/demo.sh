#! /bin/bash

source ../../utils/wsim.inc

## ============= Config===================

ELF=leds.elf
PLATFORM=msp1611

VERBOSE=2
LOGFILE=wsim.log
TRACEFILE=wsim.trc
GUI=yes

MODE=run
TIME=30s

## ============= Run =====================

run_wsim

## =======================================


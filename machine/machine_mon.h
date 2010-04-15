
/**
 *  \file   machine_mon.h
 *  \brief  Machine memory monitor
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef MACHINE_MON_H
#define MACHINE_MON_H

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "libelf/libelf.h"

void machine_monitor_init      (elf32_t elf);
void machine_monitor_start     (void);
void machine_monitor_stop      (void);

void machine_monitor_set       (char* args, elf32_t elf);
void machine_modify_set        (char* args, elf32_t elf);
void machine_monitor_add_trace (int access_type);

#endif

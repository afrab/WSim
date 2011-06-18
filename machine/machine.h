
/**
 *  \file   machine.h
 *  \brief  Platform Machine definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MACHINE_H
#define MACHINE_H

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "libelf/libelf.h"


struct machine_opt_t {
  uint64_t   insn;
  wsimtime_t time;
  int        realtime;
};


int machine_options_add();

/**
 * machine_create
 * create all devices and plug them to the mcu memory map
 **/
int  machine_create();


/** 
 * machine_dump
 **/
int machine_dump(const char *filename);

/**
 * machine_delete
 * free machine memory
 **/
int machine_delete();


/**
 * machine_reset
 **/
int machine_reset();


/**
 * machine_load_elf
 **/
int machine_load_elf(const char* filename, int verbose_level);

/**
 * machine_monitor
 **/
void machine_monitor(char* monitor, char *modify);

/**
 * machine_print_description
 **/
void machine_print_description(void);

/**
 * 
 **/
int machine_get_node_id(void);

/**
 * machine_nanotime
 */
wsimtime_t machine_get_nanotime(void);

/**
 * machine_run
 **/
void       machine_exit      (int arg);
void       machine_run       (struct machine_opt_t *m);

/**
 * machine state allocate
 **/
uint8_t*   machine_state_allocate(int devsize);
void       machine_state_free(void);

/**
 * save machine state in a backup place
 **/
void machine_state_save(void);


/**
 * restore previously stored machine state
 **/
void machine_state_restore(void);

/**
 * dump statistics at end of simulation
 */
void machine_dump_stats(uint64_t user_nanotime);

#endif

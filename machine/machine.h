
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
uint64_t machine_get_nanotime(void);

/**
 * machine_run
 **/
void     machine_run_free(void);
uint64_t machine_run_insn(uint64_t insn);
uint64_t machine_run_time(uint64_t nanotime);

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

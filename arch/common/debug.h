
/**
 *  \file   debug.h
 *  \brief  WSim internals debug functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_DEBUG_H
#define HW_DEBUG_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#include <stdint.h>

#define UNUSED __attribute__((unused))  

void     debug_write_binary  (int val, int size);
void     debug_dump_section  (uint8_t* data, uint32_t addr, uint32_t size, int maxlines);

char*    host_signal_str     (int sig);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

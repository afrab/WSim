
/**
 *  \file   cc2420_ram.h
 *  \brief  CC2420 Ram 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_ram.h
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_RAM_H
#define _CC2420_RAM_H

#include "cc2420_internals.h"

/*
 * RAM banks definitions
 */

#define CC2420_RAM_TXFIFO_BANK        0
#define CC2420_RAM_RXFIFO_BANK        1
#define CC2420_RAM_SECURITY_BANK      2
#define CC2420_RAM_NO_BANK            3

#define CC2420_RAM_TXFIFO_START       0x0000
#define CC2420_RAM_RXFIFO_START       0x0080
#define CC2420_RAM_SECURITY_START     0x0100

#define CC2420_RAM_TXFIFO_LEN         (CC2420_RAM_RXFIFO_START - CC2420_RAM_TXFIFO_START)
#define CC2420_RAM_RXFIFO_LEN         (CC2420_RAM_SECURITY_START - CC2420_RAM_RXFIFO_START)


/*
 * RAM memory space definitions (cf [1] p.31)
 */

#define CC2420_RAM_IEEEADR            0x0160
#define CC2420_RAM_PANID              0x0168
#define CC2420_RAM_SHORTADR           0x016A


/*
 * macros to extract bits from 2nd RAM access byte
 */

#define CC2420_RAM_BANK(x)               (((x) & 0xC0) >> 6)
#define CC2420_RAM_READ_ACCESS(x)        (((x) & 0x20) >> 5)


/*
 * read function
 */

uint8_t cc2420_ram_read_byte(struct _cc2420_t * cc2420, uint8_t bank, uint8_t addr);

/*
 * write function, returns the old value
 */

uint8_t cc2420_ram_write_byte(struct _cc2420_t * cc2420, uint8_t bank, uint8_t addr, uint8_t val);

#endif

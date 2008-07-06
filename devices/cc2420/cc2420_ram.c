
/**
 *  \file   cc2420_ram.c
 *  \brief  CC2420 Ram 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_ram.c
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc2420_internals.h"
#include "cc2420_ram.h"
#include "cc2420_macros.h"
#include "cc2420_debug.h"


/**
 * calculate the offset for a bank
 */

uint16_t cc2420_ram_bank_offset(uint8_t bank) {

    switch (bank) {

    case CC2420_RAM_TXFIFO_BANK :
	return CC2420_RAM_TXFIFO_START;
	
    case CC2420_RAM_RXFIFO_BANK :
	return CC2420_RAM_RXFIFO_START;
	
    case CC2420_RAM_SECURITY_BANK :
	return CC2420_RAM_SECURITY_START;

    case CC2420_RAM_NO_BANK :
        return 0;
	
    default :
	CC2420_DEBUG("cc2420_ram_bank_offset : bad RAM bank ID\n");
	return 0;
    }
}


/**
 * check if RAM is available
 * returns 0 if OK, -1 otherwise
 */

int cc2420_check_ram_access(struct _cc2420_t * cc2420) {
    if (cc2420->mem_access != CC2420_ACCESS_ALL) {
	CC2420_DEBUG("cc2420_check_ram_access : RAM is not available is this state\n");
	return -1;
    }

    return 0;
}


/**
 * read a byte from RAM at 'addr'
 * failure here is considered as a bug from driver
 */

uint8_t cc2420_ram_read_byte(struct _cc2420_t * cc2420, uint8_t bank, uint8_t addr) {

    uint16_t full_addr;

    if (cc2420_check_ram_access(cc2420)) {
	CC2420_DEBUG("cc2420_ram_read_byte : access not allowed in this state, BUG !!\n");
	return 0;
    }

    /*
     * combine bank and address to get the full 9 bits address
     */

    full_addr = addr + cc2420_ram_bank_offset(bank);

    if (full_addr > CC2420_RAM_SIZE) {
	CC2420_DEBUG("cc2420_ram_read_byte : out of RAM space, BUG !!\n");
	return 0;
    }
    
    return cc2420->ram[full_addr];
}


/**
 * write a byte to ram
 * returns the previous value
 */

uint8_t cc2420_ram_write_byte(struct _cc2420_t * cc2420, uint8_t bank, uint8_t addr, uint8_t val) {

    uint16_t full_addr;
    uint8_t old_val;

    if (cc2420_check_ram_access(cc2420)) {
	return 0;
    }

    /* combine bank and address to get the full 9 bits address */

    full_addr = addr + cc2420_ram_bank_offset(bank);

    if (full_addr > CC2420_RAM_SIZE) {
	CC2420_DEBUG("cc2420_ram_read_byte : out of RAM space, BUG !!\n");
	return 0;
    }

    old_val = cc2420->ram[full_addr];
    cc2420->ram[full_addr] = val;

    return old_val;
}


/**
 *  \file   cc2420_registers.c
 *  \brief  CC2420 registers
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_registers.c
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc2420_internals.h"
#include "cc2420_dev.h"
#include "cc2420_registers.h"
#include "cc2420_tx.h"
#include "cc2420_debug.h"

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_reset_registers(struct _cc2420_t * cc2420) 
{
  cc2420->registers[CC2420_REG_MAIN]     = CC2420_REG_MAIN_DEFAULT;
  cc2420->registers[CC2420_REG_MDMCTRL0] = CC2420_REG_MDMCTRL0_DEFAULT;
  cc2420->registers[CC2420_REG_MDMCTRL1] = CC2420_REG_MDMCTRL1_DEFAULT;
  cc2420->registers[CC2420_REG_RSSI]     = CC2420_REG_RSSI_DEFAULT;
  cc2420->registers[CC2420_REG_SYNCWORD] = CC2420_REG_SYNCWORD_DEFAULT;
  cc2420->registers[CC2420_REG_TXCTRL]   = CC2420_REG_TXCTRL_DEFAULT;
  cc2420->registers[CC2420_REG_RXCTRL0]  = CC2420_REG_RXCTRL0_DEFAULT;
  cc2420->registers[CC2420_REG_RXCTRL1]  = CC2420_REG_RXCTRL1_DEFAULT;
  cc2420->registers[CC2420_REG_FSCTRL]   = CC2420_REG_FSCTRL_DEFAULT;
  cc2420->registers[CC2420_REG_SECCTRL0] = CC2420_REG_SECCTRL0_DEFAULT;
  cc2420->registers[CC2420_REG_SECCTRL1] = CC2420_REG_SECCTRL1_DEFAULT;
  cc2420->registers[CC2420_REG_BATTMON]  = CC2420_REG_BATTMON_DEFAULT;
  cc2420->registers[CC2420_REG_IOCFG0]   = CC2420_REG_IOCFG0_DEFAULT;
  cc2420->registers[CC2420_REG_IOCFG1]   = CC2420_REG_IOCFG1_DEFAULT;
  cc2420->registers[CC2420_REG_MANFIDL]  = CC2420_REG_MANFIDL_DEFAULT;
  cc2420->registers[CC2420_REG_MANFIDH]  = CC2420_REG_MANFIDH_DEFAULT;
  cc2420->registers[CC2420_REG_FSMTC]    = CC2420_REG_FSMTC_DEFAULT;
  cc2420->registers[CC2420_REG_MANAND]   = CC2420_REG_MANAND_DEFAULT;
  cc2420->registers[CC2420_REG_MANOR]    = CC2420_REG_MANOR_DEFAULT;
  cc2420->registers[CC2420_REG_AGCCTRL]  = CC2420_REG_AGCCTRL_DEFAULT;
  cc2420->registers[CC2420_REG_AGCTST0]  = CC2420_REG_AGCTST0_DEFAULT;
  cc2420->registers[CC2420_REG_AGCTST1]  = CC2420_REG_AGCTST1_DEFAULT;
  cc2420->registers[CC2420_REG_AGCTST2]  = CC2420_REG_AGCTST2_DEFAULT;
  cc2420->registers[CC2420_REG_FSTST0]   = CC2420_REG_FSTST0_DEFAULT;
  cc2420->registers[CC2420_REG_FSTST1]   = CC2420_REG_FSTST1_DEFAULT;
  cc2420->registers[CC2420_REG_FSTST2]   = CC2420_REG_FSTST2_DEFAULT;
  cc2420->registers[CC2420_REG_FSTST3]   = CC2420_REG_FSTST3_DEFAULT;
  cc2420->registers[CC2420_REG_RXBPFTST] = CC2420_REG_RXBPFTST_DEFAULT;
  cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_REG_FSMSTATE_DEFAULT;
  cc2420->registers[CC2420_REG_ADCTST]   = CC2420_REG_ADCTST_DEFAULT;
  cc2420->registers[CC2420_REG_DACTST]   = CC2420_REG_DACTST_DEFAULT;
  cc2420->registers[CC2420_REG_TOPTST]   = CC2420_REG_TOPTST_DEFAULT;
  cc2420->registers[CC2420_REG_RESERVED] = CC2420_REG_RESERVED_DEFAULT;
  cc2420->registers[CC2420_REG_TXFIFO]   = CC2420_REG_TXFIFO_DEFAULT;
  cc2420->registers[CC2420_REG_RXFIFO]   = CC2420_REG_RXFIFO_DEFAULT;
}

/***************************************************/
/***************************************************/
/***************************************************/


/**
 * check if a register is available.
 * depends on the machine state
 * return 0 is available, -1 else
 */

int cc2420_check_register_access(struct _cc2420_t * cc2420, uint8_t addr) {

    if (cc2420->mem_access == CC2420_ACCESS_ALL)
	return 0;

    if (cc2420->mem_access == CC2420_ACCESS_REGISTERS_ONLY)
	return 0;

    if ( (cc2420->mem_access == CC2420_ACCESS_MAIN_ONLY) && (addr == CC2420_REG_MAIN) )
	return 0;

    CC2420_DEBUG("cc2420_check_register_access : register is not available in this state\n");

    return -1;
}


/**
 * write register at given address
 * if address is not valid, nothing's done
 */

void cc2420_write_register(struct _cc2420_t * cc2420, uint8_t addr, uint16_t val) {

    /* register is not available */
    if (cc2420_check_register_access(cc2420, addr)) {
	return;
    }

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_write_register : bad address\n");
	return;
    }

    /* check RO buffers */
    if (addr == CC2420_REG_FSMSTATE) {
	CC2420_DEBUG("cc2420_write_register : can't write register CC2420_REG_FSMSTATE\n");
	return;
    }

    cc2420->registers[addr] = val;

    if (addr == CC2420_REG_MDMCTRL0) {
	/* recalcalculate TX preamble length */
	/* we do it there to avoid to calculate it again on each tx */
	cc2420->tx_preamble_symbols = cc2420_tx_preamble_symbols(cc2420);
    }
}


/**
 * write high byte of register
 * if address is not valid, nothing's done
 */

void cc2420_write_register_h(struct _cc2420_t * cc2420, uint8_t addr, uint8_t valh) {

    /* register is not available */
    if (cc2420_check_register_access(cc2420, addr)) {
	return;
    }

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_write_register : bad address\n");
	return;
    }

    /* check read only buffers */
    if (addr == CC2420_REG_FSMSTATE) {
	CC2420_DEBUG("cc2420_write_register : can't write register CC2420_REG_FSMSTATE\n");
	return;
    }

    /* clear and write high byte */
    cc2420->registers[addr] &= 0X00FF;
    cc2420->registers[addr] |= ((uint16_t) (valh)) << 8;

    return;
}


/**
 * write low byte of a register
 * if address is not valid, nothing's done
 */

void cc2420_write_register_l(struct _cc2420_t * cc2420, uint8_t addr, uint8_t vall) {

    /* register is not available */
    if (cc2420_check_register_access(cc2420, addr)) {
	return;
    }

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_write_register : bad address\n");
	return;
    }

    /* check read-only buffers */
    if (addr == CC2420_REG_FSMSTATE) {
	CC2420_DEBUG("cc2420_write_register : can't write register CC2420_REG_FSMSTATE\n");
	return;
    }

    /* clear and write low byte */
    cc2420->registers[addr] &= 0XFF00;
    cc2420->registers[addr] |= ((uint16_t) (vall));

    return;
}


/**
 * read register at given address
 * if address is not valid, return -1, 0 otherwise
 */

uint16_t cc2420_read_register(struct _cc2420_t * cc2420, uint8_t addr) {

    if (cc2420_check_register_access(cc2420, addr))
	return 0;

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_read_register : bad address\n");
	return 0;
    }
    
    /* check write-only buffers */
    if (addr == CC2420_REG_TXFIFO) {
	CC2420_DEBUG("cc2420_read_register : can't read register CC2420_REG_TXFIFO\n");
	return 0;
    }

    return cc2420->registers[addr];
}


/**
 * read high byte of register
 */

uint8_t  cc2420_read_register_h(struct _cc2420_t * cc2420, uint8_t addr) {

    if (cc2420_check_register_access(cc2420, addr))
	return 0;

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_read_register : bad address\n");
	return 0;
    }
    
    /* check write-only buffers */
    if (addr == CC2420_REG_TXFIFO) {
	CC2420_DEBUG("cc2420_read_register : can't read register CC2420_REG_TXFIFO\n");
	return 0;
    }

    return (uint8_t )(cc2420->registers[addr] >> 8);
}


/**
 * read low byte of register
 */

uint8_t  cc2420_read_register_l(struct _cc2420_t * cc2420, uint8_t addr) {

    if (cc2420_check_register_access(cc2420, addr))
	return 0;

    /* check address range */
    if ( (addr < 0x10) || (addr > 0x3F) ) {
	CC2420_DEBUG("cc2420_read_register : bad address\n");
	return 0;
    }
    
    /* check write-only buffers */
    if (addr == CC2420_REG_TXFIFO) {
	CC2420_DEBUG("cc2420_read_register : can't read register CC2420_REG_TXFIFO\n");
	return 0;
    }

    return (uint8_t )(cc2420->registers[addr]);
}

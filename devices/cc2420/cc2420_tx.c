
/**
 *  \file   cc2420_tx.c
 *  \brief  CC2420 Tx methods
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_tx.c
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 *  tx functions
 */

#include <math.h>

#include "cc2420_macros.h"
#include "cc2420_internals.h"
#include "cc2420_registers.h"
#include "cc2420_ram.h"
#include "cc2420_debug.h"
#include "cc2420_tx.h"
#include "cc2420_crc_ccitt.h"
#include "cc2420_macros.h"

/**
 * calculate the operating RF frequency
 * cf [1] p 49
 */

double cc2420_get_frequency_mhz(struct _cc2420_t * cc2420) {
    int reg;
    double freq;
    reg = cc2420->registers[CC2420_REG_FSCTRL];
    reg = CC2420_REG_FSCTRL_FREQ(reg); /* */
    freq = 2048.0 + (double)reg;
    return freq;
}



/**
 * calculate the output power
 * cf [1] p 52
 */

double cc2420_get_power_dbm(struct _cc2420_t * cc2420) {
    double dbm = 0;
    uint16_t reg = cc2420->registers[CC2420_REG_TXCTRL];

    switch (reg) {
    case 0xA0FF : dbm =   0.0; break;
    case 0xA0FB : dbm = - 1.0; break;
    case 0xA0F7 : dbm = - 3.0; break;
    case 0xA0F3 : dbm = - 5.0; break;
    case 0xA0EF : dbm = - 7.0; break;
    case 0xA0EB : dbm = -10.0; break;
    case 0xA0E7 : dbm = -15.0; break;
    case 0xA0E3 : dbm = -25.0; break;
    default :
	CC2420_DEBUG("cc2420_get_power : unknown config for TXCTRL\n");
	dbm = 0.0;
    }
    return dbm;
}


/**
 * get modulation
 * cf [1] p 64
 */

int cc2420_get_modulation(struct _cc2420_t * cc2420 UNUSED) {
  int mod = (cc2420->registers[CC2420_REG_MDMCTRL1] >> 4) & 0x1;
  return (mod == 0) ? WSNET_MODULATION_OQPSK_REV : WSNET_MODULATION_802_15_4;
}


/**
 * send a byte
 */

void cc2420_tx_byte(struct _cc2420_t *cc2420, uint8_t tx_byte) 
{
  struct wsnet_tx_info tx;
  tx.data        = tx_byte;
  tx.freq_mhz    = cc2420_get_frequency_mhz(cc2420);
  tx.modulation  = cc2420_get_modulation(cc2420);
  tx.power_dbm   = cc2420_get_power_dbm(cc2420);
  tx.duration    = 2 * CC2420_SYMBOL_PERIOD; // duration

  worldsens_c_tx(&tx);

  CC2420_DBG_TX("cc2420:tx: data %02x, freq: %lgMHz, modulation: %d, "
		"Power: %lgdBm, time: %" PRId64 " + %" PRId64 " = %" PRId64 " \n", 
		tx.data & 0xff, 
		tx.freq_mhz,
		tx.modulation,
		tx.power_dbm,
		MACHINE_TIME_GET_NANO(), 
		tx.duration,
		MACHINE_TIME_GET_NANO()  + tx.duration);

  cc2420->tx_timer  = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
}

/**
 * calculate the number of 0 symbols in preamble
 */

uint8_t cc2420_tx_preamble_symbols(struct _cc2420_t * cc2420) {
    
    /* register value */
    uint8_t reg_val;

    /* number of symbols in preamble */
    uint8_t symbols;

    /* get the number of sync 0s */
    reg_val = CC2420_REG_MDMCTRL0_PREAMBLE_LENGTH(cc2420->registers[CC2420_REG_MAIN]);

    symbols = 2 * (reg_val + 1);

    /* add the SFD (2 bytes) */
    symbols += 4;
    
    return symbols;
}


/**
 *  \file   cc2420_tx.h
 *  \brief  CC2420 Tx methods
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_tx.h
 *  
 *
 *  Created by Nicolas Boulicault on 31/05/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_TX_H_
#define _CC2420_TX_H_

#include "cc2420_macros.h"
#include "cc2420_internals.h"


#define CC2420_MAX_TX_LEN          127


/**
 * the most significant bit is reserved in tx frame length field (cf [1] p.37)
 */

#define CC2420_TX_LEN_FIELD(x)  (x & 0x7F)

void     cc2420_tx_byte            (struct _cc2420_t * cc2420, uint8_t tx_byte);
uint64_t cc2420_tx_preamble_time   (struct _cc2420_t * cc2420);
uint8_t  cc2420_tx_preamble_symbols(struct _cc2420_t * cc2420);
void     cc2420_tx                 (struct _cc2420_t * cc2420);

double   cc2420_get_frequency_mhz  (struct _cc2420_t * cc2420);
int      cc2420_get_modulation     (struct _cc2420_t * cc2420);


#endif

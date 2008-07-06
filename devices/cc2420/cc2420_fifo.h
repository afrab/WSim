
/**
 *  \file   cc2420_fifo.h
 *  \brief  CC2420 Data RX/TX fifo 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_fifo.h
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_FIFO_H
#define _CC2420_FIFO_H

#include "cc2420_internals.h"

void    cc2420_tx_fifo_write     (struct _cc2420_t * cc2420, uint8_t   val);

int     cc2420_rx_fifo_push      (struct _cc2420_t * cc2420, uint8_t   val);
int     cc2420_rx_fifo_read      (struct _cc2420_t * cc2420, uint8_t * val);
int     cc2420_rx_fifo_pop       (struct _cc2420_t * cc2420, uint8_t * val);
int     cc2420_rx_fifo_get_buffer(struct _cc2420_t * cc2420, uint8_t   start_address, uint8_t * buffer, uint8_t len);

#endif

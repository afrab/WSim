
/**
 *  \file   cc2420_spi.c
 *  \brief  CC2420 SPI port
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_spi.c
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc2420_debug.h"
#include "cc2420_internals.h"
#include "cc2420_spi.h"

/**
 * write data to SPI
 */

void cc2420_spi_output(struct _cc2420_t * cc2420, uint8_t val) {
    
    cc2420->SO_pin = val ;
    cc2420->SO_set = 0xFF;

    return;
}

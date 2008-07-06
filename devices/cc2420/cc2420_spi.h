
/**
 *  \file   cc2420_spi.h
 *  \brief  CC2420 SPI port
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_spi.h
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_SPI_H
#define _CC2420_SPI_H

#include "cc2420_internals.h"

void cc2420_spi_output(struct _cc2420_t * cc2420, uint8_t val);

#endif

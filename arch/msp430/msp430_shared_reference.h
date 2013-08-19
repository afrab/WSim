
/**
 *  \brief  MSP430 shared reference
 *  \author David Graeff
 *  \date   2013
 **/

#pragma once

void msp430_sharedreference_create();
void msp430_sharedreference_reset();
#define msp430_sharedreference_update() do { } while (0)

int16_t msp430_sharedreference_read(uint16_t addr);
void msp430_sharedreference_write(uint16_t addr, int16_t val);

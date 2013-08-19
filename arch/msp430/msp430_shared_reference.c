/**
 *  \file   msp430_sysrstiv.h
 *  \brief  MSP430 Reset reason
 *  \author David Graeff
 *  \date   2013
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"
#include "msp430_shared_reference.h"

#if !defined(SHAREDREFERENCE_BASE)
#define SHAREDREFERENCE_BASE  0x01B0 
#endif

void msp430_sharedreference_create()
{
  msp430_io_register_range16(SHAREDREFERENCE_BASE,SHAREDREFERENCE_BASE,msp430_sharedreference_read,msp430_sharedreference_write);
}

void msp430_sharedreference_reset() {}

int16_t msp430_sharedreference_read(uint16_t addr)
{
  switch (addr) {
    case  SHAREDREFERENCE_BASE:      
      return 0;
    default:
      break;
  }
  
  ERROR("msp430:shared_reference: read not implemented for 0x%04x\n",addr);
  return 0;
}

void msp430_sharedreference_write(uint16_t addr, int16_t val)
{
  static uint8_t already_message = 0;
  if (already_message)
    return;
  already_message = 1;
  ERROR("msp430:shared_reference: write not implemented 0x%04x, 0x%04x\n",addr,val);
}

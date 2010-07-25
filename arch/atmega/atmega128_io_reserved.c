/**
 *  \file   atmega128_io_reserved.c
 *  \brief  Atmega128 MCU Reserved IO ports 
 *  \author Joe R. Nassimian
 *  \date   2010
 **/
 
#include <stdio.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "atmega128.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define RESERVED_IO_REG_ADDR(X) 0 // TODO

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_io_reserved_init(void)
{
    int8_t idx;
    
    for(idx = 0 ; idx < 21 ; idx++)
    {
        atmega128_io_register(RESERVED_IO_REG_ADDR(idx), 
                              atmega128_io_reserved_mcu_read, 
                              atmega128_io_reserved_mcu_write,
                              "Reserved");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t atmega128_io_reserved_mcu_read(uint16_t addr)
{
    HW_DMSG_IO_RESERVED("atmega128:io:reserved: read byte [0x%04x] for pc 0x%04x\n",addr,mcu_get_pc());
    HW_DMSG_IO_RESERVED("atmega128:io:reserved:     -- source is reserved address\n");
    
    return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_io_reserved_mcu_write(uint16_t addr, int8_t val)
{
    mcu_signal_add(SIG_MCU | SIG_MCU_BUS);
    ERROR("atmega128:io:reserved: write byte [0x%04x] = 0x%02x at pc 0x%04x\n",addr,(uint8_t)val,mcu_get_pc());
    ERROR("atmega128:io:reserved:     -- target is reserved address\n");
}
/**
 *  \file   atmega128_io_reserved.h
 *  \brief  Atmega128 MCU Reserved IO ports 
 *  \author Joe R. Nassimian
 *  \date   2010
 **/

#ifndef ATMEGA128_IO_RESERVED_H
#define ATMEGA128_IO_RESERVED_H


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum reserved_regs_t {
    
    IO_REG_RESERVED1 = 96,
    IO_REG_RESERVED2 = 102,
    IO_REG_RESERVED3,
    
    IO_REG_RESERVED4 = 105,
    IO_REG_RESERVED5 = 107,
    IO_REG_RESERVED6 = 110,
    
    IO_REG_RESERVED7 = 117,
    IO_REG_RESERVED8,
    IO_REG_RESERVED9,
    
    IO_REG_RESERVED10 = 123,
    IO_REG_RESERVED11 = 126,
    IO_REG_RESERVED12,
    
    IO_REG_RESERVED13 = 141,
    IO_REG_RESERVED14,
    IO_REG_RESERVED15,
    
    IO_REG_RESERVED16 = 145,
    IO_REG_RESERVED17,
    IO_REG_RESERVED18,
    
    IO_REG_RESERVED19,
    IO_REG_RESERVED20 = 150,
    IO_REG_RESERVED21
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    atmega128_io_reserved_init              (void);

int8_t  atmega128_io_reserved_mcu_read          (uint16_t addr);
void    atmega128_io_reserved_mcu_write         (uint16_t addr, int8_t val);


#endif
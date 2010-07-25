/**
 *  \file   atmega128_io_reserved.h
 *  \brief  Atmega128 MCU Reserved IO ports 
 *  \author Joe R. Nassimian
 *  \date   2010
 **/


#ifndef ATMEGA128_IO_RESERVED_H
#define ATMEGA128_IO_RESERVED_H


void    atmega128_io_reserved_init              (void);

int8_t  atmega128_io_reserved_mcu_read          (uint16_t addr);
void    atmega128_io_reserved_mcu_write         (uint16_t addr, int8_t val);


#endif
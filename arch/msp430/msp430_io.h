
/**
 *  \file   msp430_io.h
 *  \brief  MSP430 Memory mapped IO selector 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_IO_H
#define MSP430_IO_H

void     msp430_io_init     (void);
int8_t   msp430_read_byte   (uint16_t loc);
int16_t  msp430_read_short  (uint16_t loc);
uint16_t msp430_fetch_short (uint16_t loc);
void     msp430_write_byte  (uint16_t loc, int8_t  val);
void     msp430_write_short (uint16_t loc, int16_t val);

void     msp430_set_flash_write_start_erase (uint16_t start, uint16_t end);
void     msp430_set_flash_write_normal      (uint16_t start, uint16_t end);
void     msp430_set_flash_read_jump_pc      (uint16_t start, uint16_t end);
void     msp430_set_flash_read_normal       (uint16_t start, uint16_t end); 

#endif

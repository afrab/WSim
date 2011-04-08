
/**
 *  \file   msp430_io.h
 *  \brief  MSP430 Memory mapped IO selector 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_IO_H
#define MSP430_IO_H

void     msp430_io_create   (void);

int8_t   msp430_read_byte   (uint16_t loc);
int16_t  msp430_read_short  (uint16_t loc);
uint16_t msp430_fetch_short (uint16_t loc);
void     msp430_write_byte  (uint16_t loc, int8_t  val);
void     msp430_write_short (uint16_t loc, int16_t val);

void     msp430_io_set_flash_write_start_erase (uint16_t start, uint16_t end);
void     msp430_io_set_flash_write_normal      (uint16_t start, uint16_t end);
void     msp430_io_set_flash_read_jump_pc      (uint16_t start, uint16_t end);
void     msp430_io_set_flash_read_normal       (uint16_t start, uint16_t end); 

/*
 * memory mapped peripheral access functions
 */

typedef int8_t  (*addr_map_read8_t  ) (uint16_t addr);
typedef void    (*addr_map_write8_t ) (uint16_t addr, int8_t val);
typedef int16_t (*addr_map_read16_t ) (uint16_t addr);
typedef void    (*addr_map_write16_t) (uint16_t addr, int16_t val);

void     msp430_io_register_addr8   (uint16_t addr,  addr_map_read8_t read8, addr_map_write8_t write8);
void     msp430_io_register_range8  (uint16_t start, uint16_t stop, addr_map_read8_t read8, addr_map_write8_t write8);
void     msp430_io_register_addr16  (uint16_t addr,  addr_map_read16_t read16, addr_map_write16_t write16);
void     msp430_io_register_range16 (uint16_t start, uint16_t stop, addr_map_read16_t read16, addr_map_write16_t write16);

#endif


/**
 *  \file   atmega128_digiIO.c
 *  \brief  Atmega128 MCU Digital IO ports
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "atmega128.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline uint8_t portx_idx_to_port(uint8_t idx)
{
    if (idx < 16)
    {
        return ((idx / 3) - 1);   
    }
    else
    {
        return (idx / 3);
    }
}

inline uint8_t address_to_digiio_IDX(uint16_t addr)
{
    int8_t index;
    
    index = addr % 81 % 45 % 33;
    
    // we map pinf to 15 in order to make everything else easier.
    if (index == 32){
        index = 15;
    }
    
    return index;
}

void atmega128_digiIO_init(void)
{
    atmega128_io_register(IO_REG_PINF,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PINE,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRE,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTE, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PIND,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRD,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTD, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PINC,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRC,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTC, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PINB,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRB,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTB, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PINA,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRA,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTA, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRF,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTF, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PING,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_DDRG,  &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    atmega128_io_register(IO_REG_PORTG, &atmega128_digiIO_mcu_read, &atmega128_digiIO_mcu_write);
    
    atmega128_digiIO_reset();
}

void atmega128_digiIO_reset(void)
{
    /* after a reset the pin IO are switched to input mode */
    int i;
    for(i = 20 ; i-- ; )
    {
        DIGIIO_REGS(i) = 0;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t atmega128_digiIO_mcu_read(uint16_t addr)
{
    uint8_t res = 0;
    uint8_t index = address_to_digiio_IDX(addr);
    
    HW_DMSG_DIGI_IO("atmega128:dio: reading from MCU [%s:0x%02x]\n",
                    atmega128_debug_portname(addr),addr);

    res = DIGIIO_REGS(index);
    
    HW_DMSG_DIGI_IO("atmega128:dio: read from MCU [%s:0x%02x] = 0x%02x\n",
                    atmega128_debug_portname(addr),addr,res);
    
    return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_digiIO_mcu_write(uint16_t addr, int8_t val)
{
    uint8_t index = address_to_digiio_IDX(addr);
    uint8_t oldval;
    
    HW_DMSG_DIGI_IO("atmega128:dio: write from MCU [%s:0x%02x] = 0x%02x\n",
                    atmega128_debug_portname(addr),addr,val & 0xff); 

    uint8_t reg_case = index % 3;
    
    switch (reg_case)
    {
        case DIGIIO_PIN:
                ERROR("atmega128:dio: write on %s input register (read only)\n",
                atmega128_debug_portname(addr));
                break;
        case DIGIIO_DDR:
                DIGIIO_REGS(index) = val;
                break;
        case DIGIIO_PORTX:
                oldval = DIGIIO_REGS(index); 
                DIGIIO_REGS(index) = val;  
                MCU.digiIO.out_updated = oldval ^ DIGIIO_REGS(index); 
                //TRACER_TRACE_PORT3(DIGIIO_REGS(index));
                break;
        default:
                ERROR("atmega128:dio: write [0x%02x] undefined\n",addr); 
                break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int atmega128_digiIO_dev_read (int UNUSED port_number, uint8_t UNUSED *val)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_digiIO_dev_write(int UNUSED port_number, uint8_t UNUSED val, uint8_t UNUSED bitmask)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int atmega128_digiIO_internal_dev_read (int UNUSED port_number, uint8_t UNUSED *val)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_digiIO_internal_dev_write(int UNUSED port_number, uint8_t UNUSED val, uint8_t UNUSED bitmask)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_digiIO_update_done(void)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int atmega128_digiIO_chkifg(void)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


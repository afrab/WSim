
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

struct digiIO_IDX_mapping_t {
    uint16_t IO_reg_addr;
    char *name;
};

// Map indexes back to IO register addresses and name
struct digiIO_IDX_mapping_t digiio_idx_mapping[] = 
{    
    {IO_REG_PINE, "PINE" },
    {IO_REG_DDRE, "DDRE" },
    {IO_REG_PORTE,"PORTE"},
    
    {IO_REG_PIND, "PIND" },
    {IO_REG_DDRD, "DDRD" },
    {IO_REG_PORTD,"PORTD"},
    
    {IO_REG_PINC, "PINC" },
    {IO_REG_DDRC, "DDRC" },
    {IO_REG_PORTC,"PORTC"},
    
    {IO_REG_PINB, "PINB" },
    {IO_REG_DDRB, "DDRB" },
    {IO_REG_PORTB,"PORTB"},
    
    {IO_REG_PINA, "PINA" },
    {IO_REG_DDRA, "DDRA" },
    {IO_REG_PORTA,"PORTA"},
 
    {IO_REG_PINF, "PINF" },
    {IO_REG_DDRF, "DDRF" },
    {IO_REG_PORTF,"PORTF"},
    
    {IO_REG_PING, "PING" },
    {IO_REG_DDRG, "DDRG" },
    {IO_REG_PORTG,"PORTG"}
};

#define DIGIIO_REG_ADDR(X) digiio_idx_mapping[X].IO_reg_addr
#define DIGIIO_REG_NAME(X) digiio_idx_mapping[X].name

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * There are 3 discontinuities and 1 irregularity in the IO address range
 * - 32 : In an ideal world, PINF would have an address of 60 or 96, but isn't
 * - 33-35 : PORTE's 3 registers
 * - 48-59 : PORTD, PORTC, PORTB & PORTA's 3 registers each
 * - 97-101 : PORTG and part of PORTF
 * 
 * PINF      0x20 |  32
 * 
 * PINE      0x21 |  33
 * DDRE      0x22 |  34
 * PORTE     0x23 |  35
 *
 * PIND      0x30 |  48
 * DDRD      0x31 |  49
 * PORTD     0x32 |  50
 *
 * PINC      0x33 |  51
 * DDRC      0x34 |  52
 * PORTC     0x35 |  53
 *
 * PINB      0x36 |  54
 * DDRB      0x37 |  55
 * PORTB     0x38 |  56
 *
 * PINA      0x39 |  57
 * DDRA      0x3A |  58
 * PORTA     0x3B |  59
 *
 * DDRF      0x61 |  97
 * PORTF     0x62 |  98
 *
 * PING      0x63 |  99
 * DDRG      0x64 | 100
 * PORTG     0x65 | 101
 * 
 * In order to have a regular addressing system :
 *  - We map 32 to 15
 *  - The arithmetic modulus 33 of 33-35 maps to 0-2
 *  - The arithmetic modulus 45 of 48-59 maps to 3-14
 *  - The arithmetic modulus 81 of 97-101 maps to 16-20
 * 
 * 
 * PINE      0x21 |  33 % 33 =  0
 * DDRE      0x22 |  34 % 33 =  1
 * PORTE     0x23 |  35 % 33 =  2
 *
 * PIND      0x30 |  48 % 45 =  3
 * DDRD      0x31 |  49 % 45 =  4
 * PORTD     0x32 |  50 % 45 =  5
 *
 * PINC      0x33 |  51 % 45 =  6
 * DDRC      0x34 |  52 % 45 =  7
 * PORTC     0x35 |  53 % 45 =  8
 *
 * PINB      0x36 |  54 % 45 =  9
 * DDRB      0x37 |  55 % 45 = 10
 * PORTB     0x38 |  56 % 45 = 11
 *
 * PINA      0x39 |  57 % 45 = 12
 * DDRA      0x3A |  58 % 45 = 13
 * PORTA     0x3B |  59 % 45 = 14
 *
 * PINF      0x20 |  32      = 15
 * DDRF      0x61 |  97 % 81 = 16
 * PORTF     0x62 |  98 % 81 = 17
 *
 * PING      0x63 |  99 % 81 = 18
 * DDRG      0x64 | 100 % 81 = 19
 * PORTG     0x65 | 101 % 81 = 20
 * 
 * In order to accomplish this, we can apply the 3 consecutive arithmetic 
 * modulus operations starting with the highest number.
 * 
 * - if in 33-35 : % 81 will have no effect, and neither will 45, as they're all 
 *                 greater then 33, same principle applies to 34 and 34
 * - if in 48-59 : % 81 will have no effect, but % 45 will reduce the range 
 *                 45-59 to 3-14, and so %33 will have no effect after the 
 *                 application of % 45
 * - if in 97-101 : % 81 will reduce the range 97-101 to 16-20 and so 
 *                  % 45 and % 33 will have no effect as they're both gonna 
 *                 greater then the result of the operation.
 * 
 * This allows us to do most of the translation in one line :
 * addr % 81 % 45 % 33
 * */
inline uint8_t address_to_digiio_IDX(uint16_t addr)
{
    int8_t index;
    
    // we map pinf to 15 in order to make everything else easier.
    if (addr == 32)
    {
        index = 15;
    }
    else
    {
        index = addr % 81 % 45 % 33;
    }
    
    return index;
}

void atmega128_digiIO_init(void)
{
    int8_t idx;
    
    for(idx = 0 ; idx < 21 ; idx++)
    {
        atmega128_io_register(DIGIIO_REG_ADDR(idx), 
                              atmega128_digiIO_mcu_read, 
                              atmega128_digiIO_mcu_write,
                              DIGIIO_REG_NAME(idx));
    }
    
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

/*
 * The quick address conversion in address_to_digiio_IDX has an added advantage 
 * of allowing us find the type of register we are operating on since all we 
 * need to do is look at the arithmetic modulus 3 of the address
 * 
 *  - 0 means it's a PINx
 *  - 1 means it's a DDRx
 *  - 2 means it's a PORTx
 * 
 * */

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


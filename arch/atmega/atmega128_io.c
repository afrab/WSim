
/**
 *  \file   atmega128_io.c
 *  \brief  Atmega128 MCU Memory mapped IO 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "atmega128.h"

#define HW_DMSG_RAM   HW_DMSG_IO
#define HW_DMSG_FLASH HW_DMSG_IO


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline uint16_t IO_space_to_RAM_space(uint16_t addr)
{
    uint16_t res = 0;
    
    if (addr < 64)
    {
        res = addr + 32;
    }
    else
    {
        ERROR("atmega128:io: IO to RAM address space conversion :- %d is out of range", addr);
    }
    return res;
}

static inline uint16_t RAM_space_to_IO_space(uint16_t addr)
{
    uint16_t res = 0;
    
    if ((addr > 31) && (addr < 96))
    {
        res = addr - 32;
    }
    else
    {
        ERROR("atmega128:io: RAM to IO address space conversion :- %d is out of range", addr);
    }
    return res;
}

static inline uint16_t RAM_space_to_IDX(uint16_t addr)
{
    uint16_t res = 0;
    
    if ((addr < 158) && (addr > 31))
    {
        res = addr - 32;
    }
    else
    {
        ERROR("atmega128:io: RAM to IDX address conversion :- %d is out of range", addr);
    }
    return res;
}

static int8_t atmega128_read8_sigbus(uint16_t addr)
{
  mcu_signal_add(SIG_MCU | SIG_MCU_BUS);
  ERROR("atmega128:io: read byte at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc());
  ERROR("atmega128:io:     -- target unknown or block not implemented\n");
  return 0;
}

static void atmega128_write8_sigbus(uint16_t addr, int8_t val)
{
  mcu_signal_add(SIG_MCU | SIG_MCU_BUS);
  ERROR("atmega128:io: write byte [0x%04x] = 0x%02x at pc 0x%04x\n",addr,(uint8_t)val,mcu_get_pc());
  ERROR("atmega128:io:     -- target unknown or block not implemented\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
static void atmega128_write16_sigbus(uint16_t addr, int16_t val)
{
  mcu_signal_add(SIG_MCU_BUS);
  ERROR("atmega128:io: write short [0x%04x] = 0x%04x at pc 0x%04x\n",addr,(uint16_t)val,mcu_get_pc_current());
  ERROR("atmega128:io:     -- target unknown or block not implemented\n"); 
}
*/
/*
static int16_t atmega128_read16_sigbus(uint16_t addr)
{
  mcu_signal_add(SIG_MCU_BUS);
  ERROR("atmega128:io: read short at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc_current());
  ERROR("atmega128:io:     -- target unknown or block not implemented\n");
  return 0;
}
*/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct atmega128_io_addr_fptr_t {
  io_read8_t     read;
  io_write8_t    write;
  char           *name;
};

/* #if defined(DEBUG_IO) */
/* addresses correspond to OUT opcode, do not use with ST */
/* static */
struct atmega128_io_addr_fptr_t atmega128_io_addr_fptr[] = {

  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINF"     }, /* 0x00 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINE"     }, /* 0x01 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRE"     }, /* 0x02 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTE"    }, /* 0x03 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCL"     }, /* 0x04 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCH"     }, /* 0x05 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCSRA"   }, /* 0x06 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADMUX"    }, /* 0x07 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ACSR"     }, /* 0x08 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR0L"   }, /* 0x09 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0B"   }, /* 0x0A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0A"   }, /* 0x0B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UDR0"     }, /* 0x0C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPCR"     }, /* 0x0D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPSR"     }, /* 0x0E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPDR"     }, /* 0x0F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PIND"     }, /* 0x10 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRD"     }, /* 0x11 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTD"    }, /* 0x12 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINC"     }, /* 0x13 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRC"     }, /* 0x14 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTC"    }, /* 0x15 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINB"     }, /* 0x16 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRB"     }, /* 0x17 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTB"    }, /* 0x18 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINA"     }, /* 0x19 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRA"     }, /* 0x1A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTA"    }, /* 0x1B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EECR"     }, /* 0x1C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEDR"     }, /* 0x1D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEARL"    }, /* 0x1E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEARH"    }, /* 0x1F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SFIOR"    }, /* 0x20 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "WDTCR"    }, /* 0x21 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCDR"     }, /* 0x22 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR2"     }, /* 0x23 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT2"    }, /* 0x24 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR2"    }, /* 0x25 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR1L"    }, /* 0x26 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR1H"    }, /* 0x27 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1BL"   }, /* 0x28 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1BH"   }, /* 0x29 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1AL"   }, /* 0x2A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1AH"   }, /* 0x2B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT1L"   }, /* 0x2C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT1H"   }, /* 0x2D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1B"   }, /* 0x2E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1A"   }, /* 0x2F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ASSR"     }, /* 0x30 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR0"     }, /* 0x31 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT0"    }, /* 0x32 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR0"    }, /* 0x33 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "MCUCSR"   }, /* 0x34 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "MCUCR"    }, /* 0x35 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TIFR"     }, /* 0x36 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TIMSK"    }, /* 0x37 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EIFR"     }, /* 0x38 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EIMSK"    }, /* 0x39 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EICRB"    }, /* 0x3A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "RAMPZ"    }, /* 0x3B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XDIV"     }, /* 0x3C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPL"      }, /* 0x3D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPH"      }, /* 0x3E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SREG"     },  /* 0x3F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x60 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRF"     }, /* 0x61 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTF"    }, /* 0x62 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PING"     }, /* 0x63 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRG"     }, /* 0x64 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTG"    }, /* 0x65 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x66 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x67 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPMCSR"   }, /* 0x68 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x69 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EICRA"    }, /* 0x6A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x6B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XMCRB"    }, /* 0x6C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XMCRA"    }, /* 0x6D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x6E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OSCCAL"   }, /* 0x6F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWBR"     }, /* 0x70 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWSR"     }, /* 0x71 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWAR"     }, /* 0x72 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWDR"     }, /* 0x73 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWCR"     }, /* 0x74 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x75 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x76 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x77 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1CL"   }, /* 0x78 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1CH"   }, /* 0x79 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1C"   }, /* 0x7A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x7B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ETIFR"    }, /* 0x7C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ETIMSK"   }, /* 0x7D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x7E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x7F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR3L"    }, /* 0x80 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR3H"    }, /* 0x81 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3CL"   }, /* 0x82 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3CH"   }, /* 0x83 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3BL"   }, /* 0x84 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3BH"   }, /* 0x85 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3AL"   }, /* 0x86 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3AH"   }, /* 0x87 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT3L"   }, /* 0x88 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT3H"   }, /* 0x89 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3B"   }, /* 0x8A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3A"   }, /* 0x8B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3C"   }, /* 0x8C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x8D */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x8E */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x8F */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR0H"   }, /* 0x90 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x91 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x92 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x93 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x94 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0C"   }, /* 0x95 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x96 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" }, /* 0x97 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR1H"   }, /* 0x98 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR1L"   }, /* 0x99 */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1B"   }, /* 0x9A */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1A"   }, /* 0x9B */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UDR1"     }, /* 0x9C */
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1C"   }, /* 0x9D */

};


/*
 * atmega103 0x00 to 0x59 
 * atmega128 0x60 to 0xff extension
 */

void atmega128_io_init(void)
{
    
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_io_register(uint8_t addr, io_read8_t reader, io_write8_t writer)
{
    uint16_t loc = RAM_space_to_IDX(addr);
    
    atmega128_io_addr_fptr[loc].read  = *reader;
    atmega128_io_addr_fptr[loc].write = *writer;
}

/* 
 * Atmega 128 data memory map
 *
 * Data Memory
 * 32 Registers     $0000 - $001F
 * 64 I/O Registers $0020 - $005F
 * 160 Ext I/O Reg. $0060 - $00FF
 *                  $0100
 * Internal SRAM
 *   (4096 x 8)
 *                  $10FF
 *                  $1100
 * External SRAM
 *   (0 - 64K x 8)
 *                  $FFFF
 * 
 * These two functions are written for I/O registers 0x20 -> 0x5F 
 * mapped to 0 -> 0x3F
 */

int8_t atmega128_io_read_byte(uint16_t loc)
{
  int8_t res;
  res = MCU_IOREGS[loc & 0xff];
  HW_DMSG_IO("atmega128:io:io:read byte [0x%04x] = 0x%02x (%s)\n",
	     loc,res & 0xff, atmega128_io_addr_fptr[loc].name);
  return res;
}

void atmega128_io_write_byte(uint16_t loc, int8_t val)
{
  HW_DMSG_IO("atmega128:io:io:write byte [0x%04x] = 0x%02x (%s)\n",
	     loc,val & 0xff, atmega128_io_addr_fptr[loc].name);
  MCU_IOREGS[loc & 0xff] = val;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t atmega128_ram_read_byte(uint16_t loc)
{
  int8_t res = MCU_RAM[loc];
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
  HW_DMSG_RAM("atmega128:io:ram read byte [0x%04x] = 0x%02x\n",loc,res & 0xff);
  return res;
}

void atmega128_ram_write_byte(uint16_t loc, int8_t val)
{
  HW_DMSG_RAM("atmega128:io:ram write byte [0x%04x] = 0x%02x\n",loc,val & 0xff);
  MCU_RAM[loc] = val;
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t atmega128_ram_read_short(uint16_t loc)
{
  // int16_t res = MCU_RAM[(loc & 0xffff)+1] << 8 | MCU_RAM[loc & 0xffff];
  int16_t res = MCU_RAM[loc+1] << 8 | MCU_RAM[loc];
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  HW_DMSG_RAM("atmega128:io:ram read short [0x%04x] = 0x%04x\n",loc,res & 0xffff);
  return res;
}

void atmega128_ram_write_short(uint16_t loc, int16_t val)
{
  HW_DMSG_IO("atmega128:io: write short [0x%04x] = 0x%04x\n",loc,val & 0xffff);
  MCU_RAM[loc++] =  val       & 0xff;
  MCU_RAM[loc  ] = (val >> 8) & 0xff;
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


int8_t atmega128_flash_read_byte(uint32_t loc)
{
  int8_t res = MCU_FLASH[loc];
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
  HW_DMSG_FLASH("atmega128:io:flash read byte [0x%04x] = 0x%02x\n",loc,res & 0xff);
  return res;
}

/*
  void atmega128_flash_write_byte(uint16_t loc, int8_t val)
  {
  HW_DMSG_FLASH("atmega128:io: write byte at adress 0x%04x, value = 0x%04x\n",loc,val);
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
  }
*/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t atmega128_flash_read_short(uint32_t loc)
{
  // int16_t res = MCU_FLASH[(loc & 0xffff)+1] << 8 | MCU_FLASH[loc & 0xffff];
  int16_t res = MCU_FLASH[loc+1] << 8 | MCU_FLASH[loc];
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  HW_DMSG_FLASH("atmega128:io:flash: read short [0x%04x] = 0x%04x\n",loc,res & 0xffff);
  return res;
}

/*
  void atmega128_flash_write_short(uint16_t loc, int16_t val)
  {
  HW_DMSG_IO("atmega128:io: read short at adress 0x%04x, value = 0x%04x\n",loc,val);
  // etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  }
*/


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

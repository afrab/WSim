
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
    
    if ((31 < addr) && (addr < 158))
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


/* #if defined(DEBUG_IO) */
/* addresses correspond to OUT opcode, do not use with ST */
/* static */
struct atmega128_io_addr_fptr_t atmega128_io_addr_fptr[MCU_IO_SIZE] = {

  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINF"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINE"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRE"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTE"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCL"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCH"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADCSRA"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ADMUX"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ACSR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR0L"   },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0B"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0A"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UDR0"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPCR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPSR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPDR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PIND"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRD"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTD"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINC"     },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRC"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTC"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINB"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRB"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTB"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PINA"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRA"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTA"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EECR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEDR"     },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEARL"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EEARH"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SFIOR"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "WDTCR"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCDR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR2"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT2"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR2"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR1L"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR1H"    },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1BL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1BH"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1AL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1AH"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT1L"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT1H"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1B"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1A"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ASSR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR0"     },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT0"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR0"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "MCUCSR"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "MCUCR"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TIFR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TIMSK"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EIFR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EIMSK"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EICRB"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "RAMPZ"    },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XDIV"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPL"      },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPH"      },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SREG"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRF"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTF"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PING"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "DDRG"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "PORTG"    },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "SPMCSR"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "EICRA"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XMCRB"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "XMCRA"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OSCCAL"   },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWBR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWSR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWAR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWDR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TWCR"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1CL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR1CH"   },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR1C"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ETIFR"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ETIMSK"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR3L"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "ICR3H"    },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3CL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3CH"   },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3BL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3BH"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3AL"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "OCR3AH"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT3L"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCNT3H"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3B"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3A"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "TCCR3C"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR0H"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR0C"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "Reserved" },
  
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR1H"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UBRR1L"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1B"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1A"   },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UDR1"     },
  { .read = atmega128_read8_sigbus, .write = atmega128_write8_sigbus, .name = "UCSR1C"   },

};


/*
 * atmega103 0x00 to 0x59 
 * atmega128 0x60 to 0xff extension
 */

void atmega128_io_init(void)
{
    atmega128_digiIO_init();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_io_register(uint8_t addr, io_read8_t reader, io_write8_t writer, char * name)
{
    uint16_t loc = RAM_space_to_IDX(addr);
    
    atmega128_io_addr_fptr[loc].read  = reader;
    atmega128_io_addr_fptr[loc].write = writer;
    atmega128_io_addr_fptr[loc].name  = name;
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

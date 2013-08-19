/**
 *  \file   msp430_lcdb.c
 *  \brief  MSP430 LCD_B definition based upon msp430_lcd.c
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_lcdb)
/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_create()
{
  msp430_io_register_range8(LCDB_IOMEM_BEGIN, LCDB_IOMEM_END, msp430_lcdb_read8, msp430_lcdb_write8);
  msp430_io_register_range16(LCDB_IOMEM_BEGIN, LCDB_IOMEM_END, msp430_lcdb_read, msp430_lcdb_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_lcdb_reset()
{
  MCU.lcdb.lcdbctl0.s = 0x00;
  MCU.lcdb.lcdbctl1.s = 0x00;
  MCU.lcdb.lcdbblkctl.s = 0x00;
  return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_update()
{
  //always set the LCD framing INT.
  MCU.lcdb.lcdbctl1.b.lcdfrmifg = 1;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t msp430_lcdb_read(uint16_t addr)
{
	switch(addr) {
		case LCDB_LCDBCTL0:
			return MCU.lcdb.lcdbctl0.s;
		case LCDB_LCDBCTL1:
			return MCU.lcdb.lcdbctl1.s;
		case LCDB_LCDBBLKCTL:
			return MCU.lcdb.lcdbblkctl.s;
		case LCDB_LCDBMEMCTL: /* LCD_B memory control register */
		case LCDB_LCDBPCTL0: /* LCD_B Port Control Registers */
		case LCDB_LCDBPCTL1:
		case LCDB_LCDBPCTL2:
		case LCDB_LCDBPCTL3:
		case LCDB_LCDBVCTL: /* LCD_B Voltage Control Register */
		case LCDB_LCDBCPCTL: /* LCD_B Charge Pump Control Register */
			return 0; /* return 0: not implemented */
		default: /* no control register. Maybe an address for the lcd segment buffer/blinking buffer? */
			if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
				return MCU.lcdb.mem[addr - LCDB_LCDM_START];
			} else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
				return MCU.lcdb.bmem[addr - LCDB_LCDBM_START];
			} else {
				/* Address unknown */
				ERROR("msp430:lcdb: bad read address 0x%04x\n", addr);
				return 0;
			}
	}
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_lcdb_read8(uint16_t addr)
{
	switch(addr) {
		case LCDB_LCDBCTL0:
			return MCU.lcdb.lcdbctl0.s;
		case LCDB_LCDBCTL1:
			return MCU.lcdb.lcdbctl1.s;
		case LCDB_LCDBBLKCTL:
			return MCU.lcdb.lcdbblkctl.s;
		case LCDB_LCDBMEMCTL: /* LCD_B memory control register */
		case LCDB_LCDBPCTL0: /* LCD_B Port Control Registers */
		case LCDB_LCDBPCTL1:
		case LCDB_LCDBPCTL2:
		case LCDB_LCDBPCTL3:
		case LCDB_LCDBVCTL: /* LCD_B Voltage Control Register */
		case LCDB_LCDBCPCTL: /* LCD_B Charge Pump Control Register */
			return 0; /* return 0: not implemented */
		default: /* no control register. Maybe an address for the lcd segment buffer/blinking buffer? */
			if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
				return MCU.lcdb.mem[addr - LCDB_LCDM_START];
			} else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
				return MCU.lcdb.bmem[addr - LCDB_LCDBM_START];
			} else {
				/* Address unknown */
				ERROR("msp430:lcdb: bad read8 address 0x%04x\n", addr);
				return 0;
			}
	}
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_write(uint16_t addr, int16_t val)
{
	switch(addr) {
		case LCDB_LCDBCTL0:
			MCU.lcdb.lcdbctl0.s = val;
			break;
		case LCDB_LCDBCTL1:
			MCU.lcdb.lcdbctl1.s = val;
			break;
		case LCDB_LCDBBLKCTL:
			MCU.lcdb.lcdbblkctl.s = val;
			break;
		case LCDB_LCDBMEMCTL:
			if (val & LCDCLRM) { /* clear display */
				memset(&MCU.lcdb.mem, 0, LCDB_LCDM_STOP-LCDB_LCDM_START);
			}
			if (val & LCDCLRBM) { /* LCD_B Clear LCD blinking memory */
				memset(&MCU.lcdb.bmem, 0, LCDB_LCDBM_STOP-LCDB_LCDBM_START);
			}
		case LCDB_LCDBPCTL0: /* ignore LCD_B Port Control Registers */
		case LCDB_LCDBPCTL1:
		case LCDB_LCDBPCTL2:
		case LCDB_LCDBPCTL3:
		case LCDB_LCDBVCTL: /* ignore LCD_B Voltage Control Register */
		case LCDB_LCDBCPCTL: /* ignore LCD_B Charge Pump Control Register */
			break;
		default: /* no control register. Maybe an address for the lcd segment buffer/blinking buffer? */
			if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
				MCU.lcdb.mem[addr - LCDB_LCDM_START] = val;
			} else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
				MCU.lcdb.bmem[addr - LCDB_LCDBM_START] = val;
			} else {
				/* Address unknown */
				ERROR("msp430:lcdb: bad write address 0x%04x = 0x%02x (Valid: 0x%04x,0x%04x,0x%04x,0x%04x-0x%04x)\n",
						addr, val & 0xff, LCDB_LCDBCTL0, LCDB_LCDBCTL1, LCDB_LCDBBLKCTL, LCDB_LCDM_START, LCDB_LCDM_STOP);
			}
	}
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_write8(uint16_t addr, int8_t val)
{
  if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
    MCU.lcdb.mem[addr - LCDB_LCDM_START] = val;
  } else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
    MCU.lcdb.bmem[addr - LCDB_LCDBM_START] = val;
  } else {
    ERROR("msp430:lcdb: bad write8 address 0x%04x = 0x%02x\n", addr, val & 0xff);
  }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // _have_lcdb

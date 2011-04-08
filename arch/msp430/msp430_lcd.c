
/**
 *  \file   msp430_lcd.c
 *  \brief  MSP430 LCD definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h> 

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_lcd)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_lcd_create()
{
  msp430_io_register_range8(LCD_IOMEM_BEGIN,LCD_IOMEM_END,msp430_lcd_read,msp430_lcd_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_lcd_reset()
{
  MCU.lcd.lcdctl.s = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_lcd_update()
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_lcd_read (uint16_t addr)
{
  int8_t res;
  HW_DMSG_LCD("msp430:lcd: read [0x%04x]\n",addr);
  if (addr == LCDCTL)
    {
      res = MCU.lcd.lcdctl.s;
    }
  else if ((addr >= LCD_MEM_START) && (addr <= LCD_MEM_STOP))
    {
      res = MCU.lcd.mem[addr - LCD_MEM_START];
    }
  else
    {
      ERROR("msp430:lcd: bad read address 0x%04x\n",addr);
      res = 0;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_lcd_write(uint16_t addr, int8_t val)
{
  HW_DMSG_LCD("msp430:lcd: write : [0x%04x] = 0x%02x\n",addr,val & 0xff); 
  if (addr == LCDCTL)
    {
      MCU.lcd.lcdctl.s = val;
    }
  else if ((addr >= LCD_MEM_START) && (addr <= LCD_MEM_STOP))
    {
      MCU.lcd.mem[addr - LCD_MEM_START] = val;
    }
  else
    {
      ERROR("msp430:lcd: bad write address 0x%04x = 0x%02x\n",addr,val & 0xff);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // _have_lcd


/**
 *  \file   msp430_sfr.c
 *  \brief  MSP430 Special functions registers definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_sfr_create()
{
  msp430_io_register_range8(SFR_START,SFR_END,msp430_sfr_read8,msp430_sfr_write8);
  msp430_io_register_range16(SFR_START,SFR_END,msp430_sfr_read,msp430_sfr_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_sfr_reset()
{
  memset(&MCU.sfr,0,sizeof(struct msp430_sfr_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_sfr_read8 (uint16_t addr)
{
  int8_t res = 0;
  switch(addr)
    {
    case SFR_IE1:
      res = MCU.sfr.ie1.s;
      HW_DMSG_SFR("msp430:sfr: read IE1 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFR_IE2:
      res = MCU.sfr.ie2.s;
      HW_DMSG_SFR("msp430:sfr: read IE2 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFR_IFG1:
      res = MCU.sfr.ifg1.s;
      HW_DMSG_SFR("msp430:sfr: read IFG1 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFR_IFG2:
      res = MCU.sfr.ifg2.s;
      HW_DMSG_SFR("msp430:sfr: read IFG2 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
#if defined(__msp430_have_new_sfr)
    case SFRRPCR:
      res = MCU.sfr.sfrrpcr.s & 0x00ff;
      HW_DMSG_SFR("msp430:sfr: read SFRRPCR_L [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFRRPCR + 1:
      res = (MCU.sfr.sfrrpcr.s & 0xff00) >> 8;
      HW_DMSG_SFR("msp430:sfr: read SFRRPCR_H [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
#else
    case SFR_MER1:
      res = MCU.sfr.me1.s;
      HW_DMSG_SFR("msp430:sfr: read ME1 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFR_MER2:
      res = MCU.sfr.me2.s;
      HW_DMSG_SFR("msp430:sfr: read ME2 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
#endif
    default:
      ERROR("msp430:sfr: read bad address 0x%04x\n",addr);
      break;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_sfr_read(uint16_t addr)
{
  int16_t res = 0;
  switch(addr)
    {
    case SFR_IE1:
      res = MCU.sfr.ie1.s | (MCU.sfr.ie2.s << 8);
      HW_DMSG_SFR("msp430:sfr: read IE [0x%04x] = 0x%04x \n",addr,res & 0xffff);
      break;
    case SFR_IFG1:
      res = MCU.sfr.ifg1.s | (MCU.sfr.ifg1.s << 8);
      HW_DMSG_SFR("msp430:sfr: read IFG [0x%04x] = 0x%04x \n",addr,res & 0xffff);
      break;
#if defined (__msp430_have_new_sfr)
    case SFRRPCR:
      res = MCU.sfr.sfrrpcr.s;
      HW_DMSG_SFR("msp430:sfr: read SFRRPCR [0x%04x] = 0x%04x \n",addr,res & 0xffff);
      break;
#endif
    default:
      ERROR("msp430:sfr: read bad address 0x%04x\n",addr);
      break;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_sfr_write8(uint16_t addr, int8_t val)
{
  switch(addr)
    {
    case SFR_IE1:
      MCU.sfr.ie1.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IE1 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

    case SFR_IE2:
      MCU.sfr.ie2.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IE2 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

#if defined(__msp430_have_new_sfr)
    case SFR_IFG1:
      MCU.sfr.ifg1.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IFG1 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

    case SFR_IFG2:
      MCU.sfr.ifg2.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IFG1 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;      

    case SFRRPCR:
      MCU.sfr.sfrrpcr.s  = (MCU.sfr.sfrrpcr.s & 0xff00) | val;
      HW_DMSG_SFR("msp430:sfr: write SFRRPCR_L [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;      

    case SFRRPCR + 1:
      MCU.sfr.sfrrpcr.s  = (MCU.sfr.sfrrpcr.s & 0x00ff) | (val << 8);
      HW_DMSG_SFR("msp430:sfr: write SFRRPCR_H [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;
#else
      /***********/
    case SFR_IFG1:
      /***********/
      {
	union {
	  struct ifg1_t    b;
	  uint8_t          s;
	} ifg1;

	ifg1.s = val;
#if defined(__msp430_have_usart0)
	if (ifg1.b.utxifg0 == 1 && MCU.sfr.ie1.b.utxie0 == 0)
	  {
	    DMA_SET_UTXIFG0();
	  }
	if (ifg1.b.urxifg0 == 1 && MCU.sfr.ie1.b.urxie0 == 0)
	  {
	    DMA_SET_URXIFG0();
	  }
#endif
	MCU.sfr.ifg1.s = ifg1.s;
      	HW_DMSG_SFR("msp430:sfr: write IFG1 [0x%04x] = 0x%02x\n",addr,ifg1.s & 0xff); 
      }
      break;

      /***********/
    case SFR_IFG2:
      /***********/
      {
	union {
	  struct ifg2_t    b;
	  uint8_t          s;
	} ifg2;

	ifg2.s = val;
#if defined(__msp430_have_uscia0)
	if (ifg2.b.uca0txifg == 1 && MCU.sfr.ie2.b.uca0txie == 0)
	  {
	    DMA_SET_UTXIFG0();
	  }
	if (ifg2.b.uca0rxifg == 1 && MCU.sfr.ie2.b.uca0rxie == 0)
	  {
	    DMA_SET_URXIFG0();
	  }
#endif
#if defined(__msp430_have_usart1)
	if (ifg2.b.utxifg1 == 1 && MCU.sfr.ie2.b.utxie1 == 0)
	  {
	    DMA_SET_UTXIFG1();
	  }
	if (ifg2.b.urxifg1 == 1 && MCU.sfr.ie2.b.urxie1 == 0)
	  {
	    DMA_SET_URXIFG1();
	  }
#endif
	MCU.sfr.ifg2.s = ifg2.s;
	HW_DMSG_SFR("msp430:sfr: write IFG2 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      }
      break;

      /***********/
    case SFR_MER1:
      /***********/
      MCU.sfr.me1.s  = val;
      HW_DMSG_SFR("msp430:sfr: write ME1 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

      /***********/
    case SFR_MER2:
      /***********/
      MCU.sfr.me2.s  = val;
      HW_DMSG_SFR("msp430:sfr: write ME2 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

#endif
    default:
      ERROR("msp430:sfr: write bad address [0x%04x]=0x%02x\n",addr,val & 0xff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_sfr_write(uint16_t addr, int16_t val)
{
  switch(addr)
    {
    case SFR_IE1:
      MCU.sfr.ie1.s = val & 0x00ff;
      MCU.sfr.ie2.s = (val & 0xff00) >> 8;
      HW_DMSG_SFR("msp430:sfr: write IE [0x%04x] = 0x%04x\n",addr,val & 0xffff);
      break;

#if defined(__msp430_have_new_sfr)
    case SFR_IFG1:
      MCU.sfr.ifg1.s = val & 0x00ff;
      MCU.sfr.ifg2.s = (val & 0xff00) >> 8;
      HW_DMSG_SFR("msp430:sfr: write IFG [0x%04x] = 0x%04x\n",addr,val & 0xffff);
      break;

    case SFRRPCR:
      MCU.sfr.sfrrpcr.s = val;
      HW_DMSG_SFR("msp430:sfr: write SFRRPCR [0x%04x] = 0x%04x\n",addr,val & 0xffff);
      break;    
#endif
    default:
      ERROR("msp430:sfr: write bad address [0x%04x]=0x%02x\n",addr,val & 0xff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */



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

void msp430_sfr_reset()
{
  memset(&MCU.sfr,0,sizeof(struct msp430_sfr_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_sfr_read (uint16_t addr)
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
    case SFR_MER1:
      res = MCU.sfr.me1.s;
      HW_DMSG_SFR("msp430:sfr: read ME1 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    case SFR_MER2:
      res = MCU.sfr.me2.s;
      HW_DMSG_SFR("msp430:sfr: read ME2 [0x%04x] = 0x%02x \n",addr,res & 0xff);
      break;
    default:
      ERROR("msp430:sfr: read bad address 0x%04x\n",addr);
      break;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_sfr_write(uint16_t addr, int8_t val)
{
  switch(addr)
    {
      /***********/
    case SFR_IE1:
      /***********/
      MCU.sfr.ie1.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IE1 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

      /***********/
    case SFR_IE2:
      /***********/
      MCU.sfr.ie2.s  = val;
      HW_DMSG_SFR("msp430:sfr: write IE2 [0x%04x] = 0x%02x\n",addr,val & 0xff); 
      break;

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

      /***********/
    default:
      /***********/
      ERROR("msp430:sfr: write bad address [0x%04x]=0x%02x\n",addr,val & 0xff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


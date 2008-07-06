
/**
 *  \file   msp430_hwmul.c
 *  \brief  MSP430 Hardware Multiplier 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

// FIXME: this file needs a functionnal verification

#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/* ****************************************
** [slau056e.pdf, page 162 section 7.1]
** description of the hardware multiplier
**
** this model does not verify timing, results
** are ready in 1 clock sycle. 
**
* *****************************************/ 

#ifdef __msp430_have_hwmul

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_hwmul_init()
{
  memset(&MCU_HWMUL,0,sizeof(struct msp430_hwmul_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_hwmul_reset()
{
  /* registers are unchanged or undefined on reset */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_hwmul_read  (uint16_t addr)
{
  int16_t res;
  switch (addr)
    {
    case HWMUL_MPY      : /* fall through */
    case HWMUL_MPYS     : /* fall through */
    case HWMUL_MAC      : /* fall through */
    case HWMUL_MACS     : res = MCU_HWMUL.op1;    break;
    case HWMUL_OP2      : res = MCU_HWMUL.op2;    break;
    case HWMUL_RESLO    : res = MCU_HWMUL.reslo;  break;
    case HWMUL_RESHI    : res = MCU_HWMUL.reshi;  break;
    case HWMUL_SUMEXT   : res = MCU_HWMUL.sumext; break;
    default:
      ERROR("msp430:hwmul: read address error 0x%04x\n",addr);
      res = 0;
      break;
    }
  HW_DMSG_HWMUL("msp430:hwmul: read [0x%04x] = 0x%04x\n",addr,res);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_hwmul_write (uint16_t addr, int16_t val)
{

  switch (addr)
    {
    case HWMUL_MPY      : /* fall through */
    case HWMUL_MPYS     : /* fall through */
    case HWMUL_MAC      : /* fall through */
    case HWMUL_MACS     : 
      MCU_HWMUL.op1  = val;
      MCU_HWMUL.mode = addr;
      HW_DMSG_HWMUL("msp430:hwmul: write op1 [0x%04x] = 0x%04x\n",addr,val);
      break;

    case HWMUL_OP2      : 
      MCU_HWMUL.op2  = val;
      HW_DMSG_HWMUL("msp430:hwmul: write op2 [0x%04x] = 0x%04x\n",addr,val);
      switch (MCU_HWMUL.mode)
	{
	  /**************************************/
	  /* Multiply (reshi,reslo) = op1 * op2 */
	  /**************************************/

	case HWMUL_MPY:  /* unsigned multiply */
	  MCU_HWMUL.mult_out.u = (unsigned)MCU_HWMUL.op2 * (unsigned)MCU_HWMUL.op1;
	  MCU_HWMUL.sumext     = 0;
	  MCU_HWMUL.reslo      =  MCU_HWMUL.mult_out.u        & 0xffff;
	  MCU_HWMUL.reshi      = (MCU_HWMUL.mult_out.u >> 16) & 0xffff;
	  HW_DMSG_HWMUL("msp430:hwmul: MPY\n");
	  break;

	case HWMUL_MPYS: /* signed multiply */
	  MCU_HWMUL.mult_out.s = MCU_HWMUL.op2 * MCU_HWMUL.op1;
	  MCU_HWMUL.sumext     = (MCU_HWMUL.mult_out.s < 0) ? 0xffff : 0x0000;
	  MCU_HWMUL.reslo      =  MCU_HWMUL.mult_out.s        & 0xffff;
	  MCU_HWMUL.reshi      = (MCU_HWMUL.mult_out.s >> 16) & 0xffff;
	  HW_DMSG_HWMUL("msp430:hwmul: MPYS \n");
	  break;

	  /***************************************************/
	  /* MAC : (reshi,reslo) = op1 * op2 + (reshi,reslo) */
	  /***************************************************/

	case HWMUL_MAC:  /* unsigned multiply accumulate */
	  {
	    uint64_t t;
	    t  = (unsigned)(MCU_HWMUL.reshi << 16) | (unsigned)(MCU_HWMUL.reslo);
	    t += (unsigned)MCU_HWMUL.op1 * (unsigned)MCU_HWMUL.op2;
	    MCU_HWMUL.sumext     =  (t >> 32) & 1;
	    MCU_HWMUL.reslo      =   t        & 0xffff;
	    MCU_HWMUL.reshi      =  (t >> 16) & 0xffff;
	    HW_DMSG_HWMUL("msp430:hwmul: MAC\n");
	  }
	  break;

	case HWMUL_MACS: /* signed multiply accumulate */
	  {
	    int64_t t;
	    t  = (MCU_HWMUL.reshi << 16) | (MCU_HWMUL.reslo);
	    t +=  MCU_HWMUL.op1 * MCU_HWMUL.op2;
	    MCU_HWMUL.sumext     =  (t < 0) ? 0xffff : 0x0000;
	    MCU_HWMUL.reslo      =   t        & 0xffff;
	    MCU_HWMUL.reshi      =  (t >> 16) & 0xffff;
	    HW_DMSG_HWMUL("msp430:hwmul: MACS\n");
	  }
	  break;

	default:
	  ERROR("msp430:hwmul: wrong operation type, mode has not been initialized\n");
	  break;
	}
      break;

    case HWMUL_RESLO    : 
      MCU_HWMUL.reslo = val;
      HW_DMSG_HWMUL("msp430:hwmul: write reslo [0x%04x] = 0x%04x\n",addr,val);
      break;
    case HWMUL_RESHI    : 
      MCU_HWMUL.reshi = val;
      HW_DMSG_HWMUL("msp430:hwmul: write reshi [0x%04x] = 0x%04x\n",addr,val);
      break;
    case HWMUL_SUMEXT   : 
      ERROR("msp430:hwmul: write on SUMEXT which is a read-only register\n");
      break;
    default:
      ERROR("msp430:hwmul: write address error [0x%04x] = 0x%04x\n",addr,val);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // have_hwmul


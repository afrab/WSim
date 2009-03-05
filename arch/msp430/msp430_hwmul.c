
/**
 *  \file   msp430_hwmul.c
 *  \brief  MSP430 Hardware Multiplier 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/


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

int8_t msp430_hwmul_read8 (uint16_t addr)
{
  int8_t  res8;
  int16_t res16;
  
  res16 = msp430_hwmul_read(addr & ~0x1);
  res8  = ((addr & 0x1) == 0) ? (res16 & 0xff) : ((res16 >> 8) & 0xff); 
  return res8;
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
      HW_DMSG_HWMUL("msp430:hwmul:0x%04x: write op1 [0x%04x] = 0x%04x\n",
		    mcu_get_pc(),addr,val);
      break;

    case HWMUL_OP2      : 
      MCU_HWMUL.op2  = val;
      HW_DMSG_HWMUL("msp430:hwmul:0x%04x: write op2 [0x%04x] = 0x%04x\n",
		    mcu_get_pc(),addr,val);
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
	  HW_DMSG_HWMUL("msp430:hwmul:0x%04x: MPY unsigned mult. 0x%04x * 0x%04x = 0x%04x:%04x\n",
			mcu_get_pc(),MCU_HWMUL.op1,MCU_HWMUL.op2,MCU_HWMUL.reshi,MCU_HWMUL.reslo);
	  break;

	case HWMUL_MPYS: /* signed multiply */
	  MCU_HWMUL.mult_out.s = MCU_HWMUL.op2 * MCU_HWMUL.op1;
	  MCU_HWMUL.sumext     = (MCU_HWMUL.mult_out.s < 0) ? 0xffff : 0x0000;
	  MCU_HWMUL.reslo      =  MCU_HWMUL.mult_out.s        & 0xffff;
	  MCU_HWMUL.reshi      = (MCU_HWMUL.mult_out.s >> 16) & 0xffff;
	  HW_DMSG_HWMUL("msp430:hwmul:0x%04x: MPYS signed mult. 0x%04x * 0x%04x = 0x%04x:%04x\n",
			mcu_get_pc(),MCU_HWMUL.op1,MCU_HWMUL.op2,MCU_HWMUL.reshi,MCU_HWMUL.reslo);
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
	    HW_DMSG_HWMUL("msp430:hwmul:0x%04x: MAC 0x%04x:%04x + 0x%04x * 0x%04x = 0x%04x:%04x\n",
			  mcu_get_pc(),MCU_HWMUL.reshi,MCU_HWMUL.reslo,
			  MCU_HWMUL.op1,MCU_HWMUL.op2,MCU_HWMUL.reshi,MCU_HWMUL.reslo);
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
	    HW_DMSG_HWMUL("msp430:hwmul:0x%04x: MACS 0x%04x:%04x + 0x%04x * 0x%04x = 0x%04x:%04x\n",
			  mcu_get_pc(),MCU_HWMUL.reshi,MCU_HWMUL.reslo,
			  MCU_HWMUL.op1,MCU_HWMUL.op2,MCU_HWMUL.reshi,MCU_HWMUL.reslo);
	  }
	  break;

	default:
	  ERROR("msp430:hwmul:0x%04x: wrong operation type, mode has not been initialized\n",
		mcu_get_pc());
	  break;
	}
      break;

    case HWMUL_RESLO    : 
      MCU_HWMUL.reslo = val;
      HW_DMSG_HWMUL("msp430:hwmul:0x%04x: write reslo [0x%04x] = 0x%04x\n",mcu_get_pc(),addr,val);
      break;
    case HWMUL_RESHI    : 
      MCU_HWMUL.reshi = val;
      HW_DMSG_HWMUL("msp430:hwmul:0x%04x: write reshi [0x%04x] = 0x%04x\n",mcu_get_pc(),addr,val);
      break;
    case HWMUL_SUMEXT   : 
      ERROR("msp430:hwmul:0x%04x: write on SUMEXT which is a read-only register\n",mcu_get_pc());
      break;
    default:
      ERROR("msp430:hwmul:0x%04x: write address error [0x%04x] = 0x%04x\n",mcu_get_pc(),addr,val);
      break;
    }
}

void msp430_hwmul_write8 (uint16_t addr, int8_t val)
{
  int16_t val16;
  int16_t addr16;

  addr16 = addr & ~0x1;
  val16  = 0;

  switch (addr16)
    {
    case HWMUL_MPY      : /* fall through */
    case HWMUL_MPYS     : /* fall through */
    case HWMUL_MAC      : /* fall through */
    case HWMUL_MACS     : val16 = MCU_HWMUL.op1;   break;
    case HWMUL_OP2      : val16 = MCU_HWMUL.op2;   break;
    case HWMUL_RESLO    : val16 = MCU_HWMUL.reslo; break;
    case HWMUL_RESHI    : val16 = MCU_HWMUL.reshi; break;
    case HWMUL_SUMEXT   : 
      ERROR("msp430:hwmul:0x%04x: write on SUMEXT which is a read-only register\n",mcu_get_pc());
      return;
    default:
      ERROR("msp430:hwmul:0x%04x: write address error [0x%04x] = 0x%04x\n",mcu_get_pc(),addr,val);
      return;
    }

  HW_DMSG_HWMUL("msp430:hwmul:0x%04x: write8 [0x%04x] = 0x%02x\n",mcu_get_pc(),addr,val);

  /* msp430 is little-endian : 0x130 == low byte, 0x131 == high byte */
  if ((addr & 0x1) != 0)
    {
      int16_t t = val;
      val16 = ((t << 8) & 0xff00) | (val16 & 0xff);
    }
  else
    {
      val16 = (val16 & 0xff00) | (val);
    }

  msp430_hwmul_write(addr16,val16);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // have_hwmul


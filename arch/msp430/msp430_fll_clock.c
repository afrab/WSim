
/**
 *  \file   msp430_fll_clock.c
 *  \brief  MSP430x4xx FLL+ clock module
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_fll_and_xt2)

/*************************/
/* print each clock step */
/*************************/
//#define DEBUG_EACH_STEP
/*******************************************************************/
/* check when clocks are disables but still used for signal source */
/*******************************************************************/
//#define DEBUG_SRC_OFF 

#define BITMASK(n) ~(0xfffffffful << n)
#define MCUFLL MCU.fll_clock
#define NANO   (1000*1000*1000)

static void msp430_fll_clock_adjust_lfxt1_freq();
static void msp430_fll_clock_adjust_dcoclk_freq();
static void msp430_fll_clock_printstate();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_fll_clock_puc()
{
  MCUFLL.scfqctl.s        = 0x1f; // puc
  MCUFLL.scfi0.s          = 0x40; // puc
  MCUFLL.scfi1.s          = 0x00; // puc 
  MCUFLL.ctl0.s           = 0x03; // puc
  MCUFLL.ctl1.s           = 0x00; // puc
  MCU.sfr.ifg1.b.ofifg    = 1;    // puc
  MCUFLL.ACLKn_bitmask    = BITMASK(MCUFLL.ctl1.b.fll_div);

  msp430_fll_clock_adjust_lfxt1_freq();
  msp430_fll_clock_adjust_dcoclk_freq();

  MCUFLL.lfxt1_cycle_nanotime  = NANO / MCUFLL.lfxt1_freq;
  MCUFLL.xt2_cycle_nanotime    = NANO / MCUFLL.xt2_freq;
  MCUFLL.dcoclk_cycle_nanotime = NANO / MCUFLL.dcoclk_freq;

  msp430_fll_clock_printstate();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_fll_clock_reset()
{
  msp430_fll_clock_puc();
  msp430_fll_clock_printstate();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * - ACLK: Auxiliary clock. The ACLK is the LFXT1CLK clock source. ACLK
 *  is software selectable for individual peripheral modules.
 * - ACLK/n: Buffered output of the ACLK. The ACLK/n is ACLK divided by
 *    1,2,4 or 8 and only used externally.
 * - MCLK: Master clock. MCLK is software selectable as LFXT1CLK,
 *  XT2CLK (if available), or DCOCLK. MCLK can be divided by 1, 2, 4, or 8
 *  within the FLL block. MCLK is used by the CPU and system.
 * - SMCLK: Sub-main clock. SMCLK is software selectable as XT2CLK (if
 *  available), or DCOCLK. SMCLK is software selectable for individual
 *  peripheral modules.
 *
 * ACLK = fCrystal is 32768 Hz
 * DCOPLUS = 0: fDCOCLK = (N + 1) x fACLK
 * DCOPLUS = 1: fDCOCLK = D x (N + 1) x fACLK  D=2^flld
 *
 * Software can disable LFXT1 by setting OSCOFF if this signal does not source
 * MCLK (SELM  3 or CPUOFF = 1 ).
 *
 *
 **/

int msp430_fll_clock_update(int clock_add)
{
  int nano_add = 0;

  /********************************************/
  /* Update nano time                         */
  /********************************************/

  /* switch on MCLK source, compute other clocks */
  /* compute the nano equivalent time            */
  switch (MCUFLL.ctl1.b.selm)
    {
    case 0: /* DCOCLK */
    case 1:
      nano_add = clock_add * MCUFLL.dcoclk_cycle_nanotime;
      break;
    case 2: /* XT2CLK */
      nano_add = clock_add * MCUFLL.xt2_cycle_nanotime;
      break;
    case 3: /* LFXT1CLK */
      nano_add = clock_add * MCUFLL.lfxt1_cycle_nanotime;
      break;
    }

  /********************************************/
  /* Update internals clocks                  */
  /********************************************/

#define CLOCK_DIVMOD_TEMP(inc,tmp,nanotime)   \
  do {                                        \
    tmp += nano_add;                          \
    inc  = tmp / nanotime;                    \
    tmp %= nanotime;                          \
  } while (0)                     


  /* Software can disable LFXT1 by setting OSCOFF if this signal does not source */
  /* MCLK (SELM != 3 or CPUOFF = 1 ). */
  if ((MCU_READ_OSCOFF == 0) || (MCUFLL.ctl1.b.selm == 3 && MCU_READ_CPUOFF == 0))
    {
      CLOCK_DIVMOD_TEMP(MCUFLL.lfxt1_increment,MCUFLL.lfxt1_temp,MCUFLL.lfxt1_cycle_nanotime);
      MCUFLL.lfxt1_counter   += MCUFLL.lfxt1_increment;
    }
  else
    {
      // HW_DMSG_CLOCK("    lfxt1 not updated\n");
    }

  /* The XT2OFF bit disables the XT2 oscillator if XT2CLK is unused for MCLK */
  /* (SELMx  2 or CPUOFF = 1) and SMCLK (SELS = 0 or SMCLKOFF = 1). */
  if ((MCUFLL.ctl1.b.xt2off == 0) || 
      (MCUFLL.ctl1.b.selm   == 2 && MCU_READ_CPUOFF == 0) || 
      (MCUFLL.ctl1.b.sels   == 1 && MCUFLL.ctl1.b.smclkoff == 0))
    {
      CLOCK_DIVMOD_TEMP(MCUFLL.xt2_increment,MCUFLL.xt2_temp,MCUFLL.xt2_cycle_nanotime);
      MCUFLL.xt2_counter     += MCUFLL.xt2_increment;
#if defined(DEBUG_SRC_OFF)
      if (MCUFLL.ctl1.b.xt2off == 1)
	{
	  ERROR("xt2 should be stopped by xt2off == 1 but it sources another clock\n");
	}
#endif
    }
  else
    {
      // HW_DMSG_CLOCK("    xt2 not updated\n");
    }

  /* Software can disable DCOCLK by setting SCG0 when it is not used to source */
  /* SMCLK or MCLK in active mode, as shown in Figure 4-4. */
  /* so it runs it SCG0 == 0 or it sources MCLK or SMCLK */
  if ((MCU_READ_SCG0     == 0) || 
      (MCUFLL.ctl1.b.selm  < 2 && MCU_READ_CPUOFF == 0) || 
      (MCUFLL.ctl1.b.sels == 0 && MCUFLL.ctl1.b.smclkoff == 0))
    {
      CLOCK_DIVMOD_TEMP(MCUFLL.dcoclk_increment,MCUFLL.dcoclk_temp,MCUFLL.dcoclk_cycle_nanotime);
      MCUFLL.dcoclk_counter     += MCUFLL.dcoclk_increment;
#if defined(DEBUG_SRC_OFF)
      if (MCU_READ_SCG0 == 1)
	{
	  ERROR("dco should be stopped by SCG0 == 1 but it sources another clock\n");
	}
      if (MCU_READ_SCG1 == 1)
	{
	  ERROR("dco should be stopped by SCG1 == 1 but it sources another clock\n");
	}
#endif
    }
  else
    {
      // HW_DMSG_CLOCK("    dco not updated\n");
    }

  /********************************************/
  /* Update external signals                  */
  /********************************************/

  /* MCLK */
  /* clock_add is given for MCLK */
  if (MCU_READ_CPUOFF == 0)
    {
      MCUFLL.MCLK_increment   = clock_add; 
      MCUFLL.MCLK_counter    += clock_add;
    }
  
  /* ACLK */
  MCUFLL.ACLK_increment      += MCUFLL.lfxt1_increment;
  MCUFLL.ACLK_counter        += MCUFLL.ACLK_increment;

  /* ACLKn */
  MCUFLL.ACLKn_temp           += MCUFLL.ACLK_increment;
  MCUFLL.ACLKn_increment       = MCUFLL.ACLKn_temp >> MCUFLL.ctl1.b.fll_div;
  MCUFLL.ACLKn_temp           &= MCUFLL.ACLKn_bitmask;
  MCUFLL.ACLKn_counter        += MCUFLL.ACLKn_increment;

  /* SMCLK */
  if (MCU_READ_SCG1 == 0 && MCUFLL.ctl1.b.smclkoff == 0)
    {
      if (MCUFLL.ctl1.b.sels == 1)
	MCUFLL.SMCLK_increment = MCUFLL.xt2_increment;
      else 
	MCUFLL.SMCLK_increment = MCUFLL.dcoclk_increment;
    }

  /********************************************/
  /*                                          */
  /********************************************/
#if defined(DEBUG_EACH_STEP)
  HW_DMSG_CLOCK("  fll clock: cycles+%d = nano+%d / lfxt1+%d xt2+%d dcoclk+%d / MCLK+%d ACLK+%d ACLKn+%d SMCLK+%d / %" PRId64 "ns\n",
		clock_add,nano_add,
		MCUFLL.lfxt1_increment,MCUFLL.xt2_increment,MCUFLL.dcoclk_increment,
		MCUFLL.MCLK_increment,MCUFLL.ACLK_increment,MCUFLL.ACLKn_increment,MCUFLL.SMCLK_increment,
		MACHINE_TIME_GET_NANO());
#endif

  return nano_add;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_fll_clock_update_done()
{
  MCUFLL.lfxt1_increment  = 0;
  MCUFLL.xt2_increment    = 0;
  MCUFLL.dcoclk_increment = 0;

  MCUFLL.MCLK_increment   = 0;
  MCUFLL.ACLK_increment   = 0;
  MCUFLL.ACLKn_increment  = 0;
  MCUFLL.SMCLK_increment  = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_fll_clock_read (uint16_t addr)
{
  uint8_t res = 0;
  switch (addr)
    {
    case FLL_SCFI0:
      res = MCUFLL.scfi0.s;
      break;
    case FLL_SCFI1:
      res = MCUFLL.scfi1.s;
      break;
    case FLL_SCFQCTL:
      res = MCUFLL.scfqctl.s;
      break;
    case FLL_CTL0:
      res = MCUFLL.ctl0.s;
      break;
    case FLL_CTL1:
      res = MCUFLL.ctl1.s;
      break;
    default:
      ERROR("FLL+ Clock wrong read address 0x%04x\n",addr);
      break;
    }
  HW_DMSG_CLOCK("FLL+ Clock read : [0x%04x] = 0x%02x\n",addr,res);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_fll_clock_write(uint16_t addr, int8_t val)
{
  switch (addr)
    {
    case FLL_SCFI0:
      MCUFLL.scfi0.s = val;
      msp430_fll_clock_adjust_dcoclk_freq();
      break;
    case FLL_SCFI1:
      MCUFLL.scfi1.s = val;
      msp430_fll_clock_adjust_dcoclk_freq(); 
      break;
    case FLL_SCFQCTL:
      MCUFLL.scfqctl.s = val;
      msp430_fll_clock_adjust_dcoclk_freq();
      break;
    case FLL_CTL0:
      {
	union {
	  struct fll_ctl0_t    b;
	  uint8_t              s;
	} ctl0;

	ctl0.s = val;
	MCUFLL.ctl0.s = val;
	/* check dcoplus */
	msp430_fll_clock_adjust_dcoclk_freq(); 

	if (MCUFLL.ctl0.b.xts_fll == 1)
	  {
	    ERROR("FLL clock does not support external source for LFXT1CLK\n");
	  }
      }
      break;

    case FLL_CTL1:
      {
#if defined(DEBUG)
	union {
	  struct fll_ctl1_t    b;
	  uint8_t              s;
	} ctl1;
	
	ctl1.s = val;
	if (ctl1.b.selm != MCUFLL.ctl1.b.selm)
	  {
	    HW_DMSG_CLOCK(" FLL clock switching ctl1.selm to %d\n",ctl1.b.selm);
	  }
	if (ctl1.b.sels != MCUFLL.ctl1.b.sels)
	  {
	    HW_DMSG_CLOCK(" FLL clock switching ctl1.sels to %d\n",ctl1.b.sels);
	  }
#endif
	MCUFLL.ctl1.s = val;
	MCUFLL.ACLKn_bitmask = BITMASK(MCUFLL.ctl1.b.fll_div);
      }
      break;
    default:
      ERROR("FLL+ Clock wrong write address [0x%04x]=0x%02x\n",addr,val & 0xff);
      break;
    }
  HW_DMSG_CLOCK("FLL+ Clock write : [0x%04x] = 0x%02x\n",addr,val & 0xff); 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_fll_clock_adjust_lfxt1_freq()
{
  /*  MCUFLL.lfxt1_freq           = DEFAULT_LFXT1_FREQ; */
  MCUFLL.lfxt1_cycle_nanotime = NANO / MCUFLL.lfxt1_freq;
  msp430_fll_clock_adjust_dcoclk_freq();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_fll_clock_adjust_dcoclk_freq()
{
  if (MCUFLL.ctl0.b.dcoplus == 0)
    {
      MCUFLL.dcoclk_freq      = (MCUFLL.scfqctl.b.N + 1) * MCUFLL.lfxt1_freq; 
    }
  else
    {
      MCUFLL.dcoclk_freq      = MCUFLL.scfi0.b.flld * (MCUFLL.scfqctl.b.N + 1) * MCUFLL.lfxt1_freq; 
    }
  MCUFLL.dcoclk_cycle_nanotime   = NANO / MCUFLL.dcoclk_freq;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_fll_clock_printstate()
{
  HW_DMSG_CLOCK("=== FLL Clock state ===\n");
  HW_DMSG_CLOCK("ACLK\n");
  HW_DMSG_CLOCK("ACLKn\n");
  HW_DMSG_CLOCK("MCLK\n");
  HW_DMSG_CLOCK("SMCLK\n");

  HW_DMSG_CLOCK("FLL+ dcoplus=%d, xts_fll=%d\n",
	      MCUFLL.ctl0.b.dcoplus,
	      MCUFLL.ctl0.b.xts_fll );
  HW_DMSG_CLOCK("FLL+ scfqctl.scfq=%d scfqctl.N=%d,  scfi0.flld=%d\n",
	      MCUFLL.scfqctl.b.scfq_m,
	      MCUFLL.scfqctl.b.N,
	      MCUFLL.scfi0.b.flld );

  HW_DMSG_CLOCK("=== End of FLL Clock state ===\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_fll */

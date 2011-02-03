
/**
 *  \file   msp430_basic_clock_plus.c
 *  \brief  MSP430x2xx basic clock plus definition 
 *  \author Julien Carpentier
 *  \date   2011
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_basic_clock_plus)

/*************************/
/* print each clock step */
/*************************/
#undef DEBUG_EACH_STEP

/*******************************************************************/
/* check when clocks are disables but still used for signal source */
/*******************************************************************/
#undef DEBUG_SRC_OFF

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

#if defined(HIGH_RES_CLOCK)
#define HRCTYPE "g"
#else
#define HRCTYPE "d"
#endif

#define BITMASK(n) ~(0xfffffffful << n)
#define MCUBCP MCU.basic_clock_plus
#define NANO  (1000*1000*1000)

static void msp430_basic_clock_plus_adjust_lfxt1_freq();
static void msp430_basic_clock_plus_adjust_dco_freq();
static void msp430_basic_clock_plus_adjust_vlo_freq();
static void msp430_basic_clock_plus_printstate();

/**
   After a PUC, MCLK and SMCLK are sourced from DCOCLK at ~800 kHz (see
   device-specific datasheet for parameters) and ACLK is sourced from LFXT1
   in LF mode.

   Status register control bits SCG0, SCG1, OSCOFF, and CPUOFF configure
   the MSP430 operating modes and enable or disable portions of the basic clock
   module. See Chapter System Resets, Interrupts and Operating Modes. The
   DCOCTL, BCSCTL1, and BCSCTL2 registers configure the basic clock
   module

   The basic clock can be configured or reconfigured by software at any time
   during program execution, for example:
*/


void 
msp430_basic_clock_plus_reset()
{
  static int firsttime = 0;

  /* 
   * dco initial state 0x60 = 0110 0000
   *     bcsctl2.rsel = 4
   *     dco.dco      = 3
   *   initial freq ~= 800 kHz
   *
   * The modulator mixing formula is:
   *      t =(32- MODx) × tDCO + MODx × tDCO+1
   *
   * bcsctl1 initial state 0x80 = 1000 0000
   *     bcsctl1.xt2off = 1 : xt2 is off 
   *
   * MCLK  comes from DCO   bcsctl2.divm=0
   * ACLK  comes from DCO   bcsctl1.diva=0
   * SMCLK comes from DCO   bcsctl2.divs=0
   */

  MCUBCP.dco.s            = 0x60;
  MCUBCP.bcsctl1.s        = 0x84;
  MCUBCP.bcsctl2.s        = 0x00;

  MCUBCP.ACLK_bitmask     = BITMASK(MCUBCP.bcsctl1.b.diva);
  MCUBCP.MCLK_bitmask     = BITMASK(MCUBCP.bcsctl2.b.divm);
  MCUBCP.SMCLK_bitmask    = BITMASK(MCUBCP.bcsctl2.b.divs);

  msp430_basic_clock_plus_adjust_lfxt1_freq();
  msp430_basic_clock_plus_adjust_dco_freq();

#if defined(HIGH_RES_CLOCK)
  MCUBCP.lfxt1_cycle_nanotime = (MCUBCP.lfxt1_freq > 0) ? ((float)NANO / (float)MCUBCP.lfxt1_freq) : 0.0;
  MCUBCP.xt2_cycle_nanotime   = (MCUBCP.xt2_freq   > 0) ? ((float)NANO / (float)MCUBCP.xt2_freq)   : 0.0;
  MCUBCP.dco_cycle_nanotime   = (float)NANO / (float)MCUBCP.dco_freq;
#else
  MCUBCP.lfxt1_cycle_nanotime = (MCUBCP.lfxt1_freq > 0) ? (NANO / MCUBCP.lfxt1_freq) : 0;
  MCUBCP.xt2_cycle_nanotime   = (MCUBCP.xt2_freq   > 0) ? (NANO / MCUBCP.xt2_freq)   : 0;
  MCUBCP.dco_cycle_nanotime   = NANO / MCUBCP.dco_freq;
#endif

  msp430_basic_clock_plus_printstate();
  if (firsttime == 0)
    {
      msp430_basic_clock_plus_speed_tracer_init();
      firsttime = 1;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int 
msp430_basic_clock_plus_update(int clock_add)
{
#if defined(HIGH_RES_CLOCK)
  float nano_add = 0;
#else
  int   nano_add = 0;
#endif

  /********************************************/
  /* Update nano time                         */
  /********************************************/

  /* switch on MCLK source, compute other clocks */
  /* compute the nano equivalent time            */
  int clock_add_mul = clock_add << MCUBCP.bcsctl2.b.divm;
  switch (MCUBCP.bcsctl2.b.selm)
    {
    case 0: /* DCOCLK */
    case 1: /* DCOCLK */
      nano_add = clock_add_mul * MCUBCP.dco_cycle_nanotime;
      break;
    case 2: /* XT2CLK */
      nano_add = clock_add_mul * MCUBCP.xt2_cycle_nanotime;
      break;
    case 3: /* LFXT1CLK */
      nano_add = clock_add_mul * MCUBCP.lfxt1_cycle_nanotime;      
      break;
    }

  /********************************************/
  /* Update internals clocks                  */
  /********************************************/
#if defined(HIGH_RES_CLOCK)
#define CLOCK_DIVMOD_TEMP(inc,tmp,nanotime)   \
  do {                                        \
    tmp += nano_add;                          \
    inc  = tmp / nanotime;                    \
    tmp  = tmp - (inc * nanotime);            \
  } while (0)                     
#else
#define CLOCK_DIVMOD_TEMP(inc,tmp,nanotime)   \
  do {                                        \
    tmp += nano_add;                          \
    inc = tmp / nanotime;                     \
    tmp %= nanotime;                          \
  } while (0)                     
#endif

  /* OSCOFF -> LFXT1 */
  /* Software can disable LFXT1 by setting OSCOFF, if this signal does not source */
  /* SMCLK or MCLK, as shown in Figure 4-2. */
  /* so it runs if OSCOFF == 0 or it sources MCLK */
  if ((MCU_READ_OSCOFF == 0) || (MCUBCP.bcsctl2.b.selm == 3 && MCU_READ_CPUOFF == 0)) 
    {
      CLOCK_DIVMOD_TEMP(MCUBCP.lfxt1_increment,MCUBCP.lfxt1_temp,MCUBCP.lfxt1_cycle_nanotime);
      MCUBCP.lfxt1_counter   += MCUBCP.lfxt1_increment;
    }
  else
    {
      // HW_DMSG_CLOCK("    lfxt1 not updated\n");
    }

  /* The XT2OFF bit disables the XT2 oscillator if XT2CLK is not used for */
  /* MCLK or SMCLK as shown in Figure 4-3. */ 
  /* so it runs if xt2off == 0 or it sources MCLK or SMCLK */
  if ((MCUBCP.bcsctl1.b.xt2off == 0) || 
      (MCUBCP.bcsctl2.b.selm   == 2  && MCU_READ_CPUOFF == 0) || 
      (MCUBCP.bcsctl2.b.sels   == 1))
    {
      CLOCK_DIVMOD_TEMP(MCUBCP.xt2_increment,MCUBCP.xt2_temp,MCUBCP.xt2_cycle_nanotime);
      MCUBCP.xt2_counter     += MCUBCP.xt2_increment;
#if defined(DEBUG_SRC_OFF)
      if (MCUBCP.bcsctl1.b.xt2off == 1)
	{
	  HW_DMSG_CLOCK("msp430:basic_clock_plus: xt2 stopped by xt2off = 1 but sources ");
	  if (MCUBCP.bcsctl2.b.selm == 2)
	    HW_DMSG_CLOCK("MCLK "); 
	  if (MCUBCP.bcsctl2.b.sels == 1)
	    HW_DMSG_CLOCK("SMCLK");
	  HW_DMSG_CLOCK("\n");
	  msp430_basic_clock_plus_printstate();
	}
#endif
    }
  else
    {
      // HW_DMSG_CLOCK("    xt2 not updated\n");
    }

  /* Software can disable DCOCLK by setting SCG0 when it is not used to source */
  /* SMCLK or MCLK in active mode, as shown in Figure 4-4. */
  /* so it runs if SCG0 == 0 or it sources MCLK or SMCLK */
  if ((MCU_READ_SCG0        == 0) || 
      (MCUBCP.bcsctl2.b.selm  < 2  && MCU_READ_CPUOFF == 0) || 
      (MCUBCP.bcsctl2.b.sels == 0))
    {
      CLOCK_DIVMOD_TEMP(MCUBCP.dco_increment,MCUBCP.dco_temp,MCUBCP.dco_cycle_nanotime);
      MCUBCP.dco_counter     += MCUBCP.dco_increment;
#if defined(DEBUG_SRC_OFF)
      if (MCU_READ_SCG0 == 1)
	{
	  HW_DMSG_CLOCK("msp430:basic_clock_plus: dco stopped by SCG0 = 1 but sources ");
	  if (MCUBCP.bcsctl2.b.selm < 2)
	    HW_DMSG_CLOCK("MCLK "); 
	  if (MCUBCP.bcsctl2.b.sels == 0)
	    HW_DMSG_CLOCK("SMCLK");
	  HW_DMSG_CLOCK("\n");
	  msp430_basic_clock_plus_printstate();
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
  /* clock_add is given in MCLK clock ticks from CPU */
  if (MCU_READ_CPUOFF == 0)
    {
      MCUBCP.MCLK_increment   = clock_add; 
      MCUBCP.MCLK_counter    += clock_add;
    }
  else
    {
      MCUBCP.MCLK_increment   = 0;
    }

  /* ACLK */
  MCUBCP.ACLK_temp           += MCUBCP.lfxt1_increment;
  MCUBCP.ACLK_increment       = MCUBCP.ACLK_temp >> MCUBCP.bcsctl1.b.diva;
  MCUBCP.ACLK_temp           &= MCUBCP.ACLK_bitmask;
  MCUBCP.ACLK_counter        += MCUBCP.ACLK_increment;

  /* SMCLK */
  if (MCU_READ_SCG1 == 0)
    {
      MCUBCP.SMCLK_temp      += (MCUBCP.bcsctl2.b.sels == 0) ? MCUBCP.dco_increment : MCUBCP.xt2_increment;
      MCUBCP.SMCLK_increment  = MCUBCP.SMCLK_temp >> MCUBCP.bcsctl2.b.divs;
      MCUBCP.SMCLK_temp      &= MCUBCP.SMCLK_bitmask;
      MCUBCP.SMCLK_counter   += MCUBCP.SMCLK_increment;
    }
  else
    {
      MCUBCP.SMCLK_increment  = 0;
      // #define RUNNING_MODE_FROM_REG(R) ((R >> 4) & 0x0fu)
      //
      // #define MCU_READ_GIE     ((uint16_t)(SR & MASK_GIE   ) >> 3)
      // #define MCU_READ_CPUOFF  ((uint16_t)(SR & MASK_CPUOFF) >> 4) 
      // #define MCU_READ_OSCOFF  ((uint16_t)(SR & MASK_OSCOFF) >> 5)
      // #define MCU_READ_SCG0    ((uint16_t)(SR & MASK_SCG0  ) >> 6)
      // #define MCU_READ_SCG1    ((uint16_t)(SR & MASK_SCG1  ) >> 7)
      //
      //      SCG1 SCG0 OSCOFF CPUOFF
      //
      // 13 1101  
      //  4 0100
      // 12 1100
      //      WARNING("SCGA = 1 // ");
    }

  /********************************************/
  /*                                          */
  /********************************************/
#if defined(DEBUG_EACH_STEP)
  HW_DMSG_CLOCK("msp430:basic_clock_plus: cycles+%d = nano+%"HRCTYPE" / lfxt1+%d xt2+%d dco+%d / MCLK+%d ACLK+%d SMCLK+%d / %" PRId64 "ns\n",
		clock_add,nano_add,
		MCUBCP.lfxt1_increment,MCUBCP.xt2_increment,MCUBCP.dco_increment,
		MCUBCP.MCLK_increment,MCUBCP.ACLK_increment,MCUBCP.SMCLK_increment,
		MACHINE_TIME_GET_NANO());
#endif

  return nano_add;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_basic_clock_plus_update_done()
{
  MCUBCP.lfxt1_increment  = 0;
  MCUBCP.xt2_increment    = 0;
  MCUBCP.dco_increment    = 0;

  MCUBCP.ACLK_increment   = 0;
  MCUBCP.MCLK_increment   = 0;
  MCUBCP.SMCLK_increment  = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_basic_clock_plus_read (uint16_t addr)
{
  HW_DMSG_CLOCK("msp430:basic_clock_plus: read : [0x%04x]\n",addr);
  switch (addr)
    {
    case BCP_DCOCTL:
      return MCUBCP.dco.s;
    case BCP_BCSCTL1:
      return MCUBCP.bcsctl1.s;
    case BCP_BCSCTL2:
      return MCUBCP.bcsctl2.s;
    default:
      ERROR("msp430:basic_clock_plus: bad read address [0x%04x]\n",addr);
      break;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_basic_clock_plus_write(uint16_t addr, int8_t val)
{
  /*  HW_DMSG("Basic clock write : [0x%04x] = 0x%02x\n",addr,val & 0xffu); */

  switch (addr)
    {
      /* **************************************** */
      /* **************************************** */
    case BCP_DCOCTL:
      {
	union {
	  struct dcoctl_t  b;
	  int8_t           s;
	} dco;
	
	dco.s = val;

	if (dco.b.dco   != MCUBCP.dco.b.dco) {
	  HW_DMSG_CLOCK("msp430:basic_clock_plus: dco.dcox modified\n");
	}
	if (dco.b.mod   != MCUBCP.dco.b.mod) {
	  HW_DMSG_CLOCK("msp430:basic_clock_plus: dco.modx modified\n");
	}
	MCUBCP.dco.s = val;
	msp430_basic_clock_plus_adjust_dco_freq();
	msp430_basic_clock_plus_printstate();
	msp430_basic_clock_plus_speed_tracer_update();
      }
      break;
      /* **************************************** */
      /* **************************************** */
    case BCP_BCSCTL1:
      {
	union {
	  struct bcsctl1_t b;
	  int8_t           s;
	} bcsctl1;

	bcsctl1.s = val;

	if (bcsctl1.b.xt2off != MCUBCP.bcsctl1.b.xt2off)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: xt2off modified\n");
	  }
	if (bcsctl1.b.rsel   != MCUBCP.bcsctl1.b.rsel)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: DCO rsel modified\n");
	  }
	if (bcsctl1.b.xts    != MCUBCP.bcsctl1.b.xts)
	  {
	    if ((bcsctl1.b.xts) && (MCU_CLOCK.lfxt1_freq < 4150000))
	      {
		WARNING("msp430:basic_clock_plus: bcsctl1 setting XTS with low frequency XIN\n");
	      }
	  }
	if (bcsctl1.b.diva   != MCUBCP.bcsctl1.b.diva)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: ACLK diva shifter modified\n");
	  }
	if (bcsctl1.b.xt5v   != 0)
	  {
	    ERROR("msp430:basic_clock_plus: bcsctl1 xt5v should always be reset\n");
	  }

	MCUBCP.bcsctl1.s = val;
	MCUBCP.ACLK_bitmask  = BITMASK(MCUBCP.bcsctl1.b.diva);
	msp430_basic_clock_plus_adjust_lfxt1_freq();	
	msp430_basic_clock_plus_adjust_dco_freq();
	msp430_basic_clock_plus_printstate();
	msp430_basic_clock_plus_speed_tracer_update();
      }
      break;
      /* **************************************** */
      /* **************************************** */
    case BCP_BCSCTL2:
      {
	union {
	  struct bcsctl2_t b;
	  int8_t           s;
	} bcsctl2;
	
	bcsctl2.s = val;
	if (bcsctl2.b.selm   != MCUBCP.bcsctl2.b.selm)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: MCLK selm modified\n");
	  }
	if (bcsctl2.b.divm   != MCUBCP.bcsctl2.b.divm)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: MCLK divm modified\n");
	  }
	if (bcsctl2.b.sels   != MCUBCP.bcsctl2.b.sels)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: SMCLK sels modified\n");
	  }
	if (bcsctl2.b.divs   != MCUBCP.bcsctl2.b.divs)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: SMCLK divs modified\n");
	  }
	if (bcsctl2.b.dcor   != MCUBCP.bcsctl2.b.dcor)
	  {
	    HW_DMSG_CLOCK("msp430:basic_clock_plus: DCO dcor modified\n");
	    if (bcsctl2.b.dcor == 1)
	      ERROR("msp430:basic_clock_plus: setting dcor to external resistor not implemented, will behave as dcor=0\n");
	  }
	
	MCUBCP.bcsctl2.s = val;
	MCUBCP.MCLK_bitmask  = BITMASK(MCUBCP.bcsctl2.b.divm);
	MCUBCP.SMCLK_bitmask = BITMASK(MCUBCP.bcsctl2.b.divs);
	msp430_basic_clock_plus_printstate();
	msp430_basic_clock_plus_speed_tracer_update();
      }
      break;
      /* **************************************** */
      /* **************************************** */
    default:
      ERROR("msp430:basic_clock_plus: bad write address 0x%04x val 0x%02x\n",addr,val & 0xff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void 
msp430_basic_clock_plus_adjust_lfxt1_freq()
{
  /*  MCUBCP.lfxt1_freq           = DEFAULT_LFXT1_FREQ; */
#if defined(HIGH_RES_CLOCK)
  MCUBCP.lfxt1_cycle_nanotime = (float)NANO / (float)MCUBCP.lfxt1_freq;
#else
  MCUBCP.lfxt1_cycle_nanotime = NANO / MCUBCP.lfxt1_freq;
#endif
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
msp430_basic_clock_plus_adjust_dco_freq()
{
#define K *1000
  /* msp430f1611.pdf, page 36 */
  /* fnom is taken for Vcc=3V */

#if defined(HIGH_RES_CLOCK)
  float fdco,fdco1;
  static float fnom[8] = /* fnom[rsel], dco=3 */
    { 130 K, 180 K, 280 K, 470 K, 750 K, 1300 K, 2000 K, 3200 K };
#else
  uint64_t fdco, fdco1;
  static uint32_t fnom[8] = /* fnom[rsel], dco=3 */
    { 130 K, 180 K, 280 K, 470 K, 750 K, 1300 K, 2000 K, 3200 K };
#endif

  /* S_{Rsel} = f_{rsel+1} / f_{rsel} = 1.65 @ 3V */ 
#define S_RSEL 1.65
  /* S_{dco}  = f_{dco+1} / f_{dco}   = 1.12 @ 3V */
#define S_DCO 1.12

  /* resistor = (MCUBCP.bcsctl2.b.dcor == 0) ? "internal" : "external" */
  if (MCUBCP.bcsctl2.b.dcor == 0)
    { /* internal dco up to 5MHz */
      fdco  = (float)fnom[MCUBCP.bcsctl1.b.rsel] * (1.0 + (float)(MCUBCP.dco.b.dco - 3) / 10.0);
      fdco1 = S_DCO * (float)fdco;
    }
  else /* dcor == 1 */
    { 
      fdco  = (float)fnom[MCUBCP.bcsctl1.b.rsel] * (1.0 + (float)(MCUBCP.dco.b.dco - 3) / 10.0);
      fdco1 = S_DCO * (float)fdco;
    }

  /* When MODx = 0 the modulator is off. */
  /* dco_freq_time t = (32- MODx) × tDCO + MODx × tDCO+1    */
  
  /* from msp430f1611.pdf, page 36 */
  /* dco_freq = (32 * fdco * fdco1) / (MODx * fdco + (32 - MODx) * fdco1) */
  MCUBCP.dco_freq = (32 * fdco * fdco1) / (MCUBCP.dco.b.mod * fdco + (32 - MCUBCP.dco.b.mod) * fdco1);
#if defined(HIGH_RES_CLOCK)
  MCUBCP.dco_cycle_nanotime = (float)NANO / (float)MCUBCP.dco_freq;
#else
  MCUBCP.dco_cycle_nanotime = NANO / MCUBCP.dco_freq;
#endif
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
msp430_basic_clock_plus_printstate()
{
  HW_DMSG_CLOCK("msp430:basic_clock_plus: === Basic Clock state === PC 0x%04x\n",mcu_get_pc());
  HW_DMSG_CLOCK("msp430:basic_clock_plus:  ACLK  ");
  HW_DMSG_CLOCK("    div: 1 << %d\n",MCUBCP.bcsctl1.b.diva);

  HW_DMSG_CLOCK("msp430:basic_clock_plus:  MCLK  ");
  HW_DMSG_CLOCK("    div: 1 << %d",MCUBCP.bcsctl2.b.divm);
  HW_DMSG_CLOCK("    src:");
  switch (MCUBCP.bcsctl2.b.selm)
    {
    case 0: 
    case 1: HW_DMSG_CLOCK("dco"); break;
    case 2: HW_DMSG_CLOCK("xt2"); break;
    case 3: HW_DMSG_CLOCK("lfxt1"); break;
    }
  HW_DMSG_CLOCK("    run:%s\n",(MCU_READ_CPUOFF == 0) ? "on" : "off");

  HW_DMSG_CLOCK("msp430:basic_clock_plus:  SMCLK ");
  HW_DMSG_CLOCK("    div: 1 << %d",MCUBCP.bcsctl2.b.divs);
  HW_DMSG_CLOCK("    src:%s",(MCUBCP.bcsctl2.b.sels == 0) ? "dco" : "xt2");
  HW_DMSG_CLOCK("    run:%s\n",(MCU_READ_SCG1 == 0) ? "on" : "off");

  HW_DMSG_CLOCK("msp430:basic_clock_plus:  lfxt1 ");
  HW_DMSG_CLOCK("    freq:%dHz",MCUBCP.lfxt1_freq);
  HW_DMSG_CLOCK("    time:%" HRCTYPE "ns",MCUBCP.lfxt1_cycle_nanotime);
  HW_DMSG_CLOCK("    run:%s\n",(MCU_READ_OSCOFF == 0) ? "on" : "off");

  HW_DMSG_CLOCK("msp430:basic_clock_plus:  xt2   ");
  HW_DMSG_CLOCK("    freq:%dHz",MCUBCP.xt2_freq);
  HW_DMSG_CLOCK("    time:%" HRCTYPE "ns",MCUBCP.xt2_cycle_nanotime);
  HW_DMSG_CLOCK("    xt2off:%d\n",MCUBCP.bcsctl1.b.xt2off);

  HW_DMSG_CLOCK("msp430:basic_clock_plus:  dco   ");
  HW_DMSG_CLOCK("    freq:%dHz",MCUBCP.dco_freq);
  HW_DMSG_CLOCK("    time:%" HRCTYPE "ns",MCUBCP.dco_cycle_nanotime);
  HW_DMSG_CLOCK("    dcox:%d\n",MCUBCP.dco.b.dco);
  HW_DMSG_CLOCK("msp430:basic_clock_plus:            rselx:%d, mod:%d, resistor:%s, run:%s\n",
		MCUBCP.bcsctl1.b.rsel,MCUBCP.dco.b.mod,
		(MCUBCP.bcsctl2.b.dcor == 0) ? "internal" : "external",
		(MCU_READ_SCG0 == 0) ? "on" : "off");
  HW_DMSG_CLOCK("msp430:basic_clock_plus: === End of Basic Clock state ===\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if TRACER_SPEED != 0
void msp430_basic_clock_plus_speed_tracer_init()
{
  msp430_basic_clock_plus_speed_tracer_update();
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(TRACER_SPEED)
void msp430_basic_clock_plus_speed_tracer_update()
{
  unsigned int lfxt1 = 0;
  unsigned int xt2   = 0;
  unsigned int dco   = 0;

  unsigned int aclk  = 0;
  unsigned int mclk  = 0;
  unsigned int smclk = 0;

  /*
   * slau049e.pdf page 115/432 - section 4-3  
   */

  // internal freqs
  lfxt1 = (MCU_READ_OSCOFF == 0)        ? MCUBCP.lfxt1_freq : 0;
  xt2   = (MCUBCP.bcsctl1.b.xt2off == 0) ? MCUBCP.xt2_freq   : 0;
  dco   = (MCU_READ_SCG0 == 0)          ? MCUBCP.dco_freq   : 0;
  
  // multiplex
  switch (MCUBCP.bcsctl2.b.selm)
    {
    case 0: 
    case 1: mclk = dco;   break;
    case 2: mclk = xt2;   break;
    case 3: mclk = lfxt1; break;
    }
  smclk = (MCUBCP.bcsctl2.b.sels == 0) ? dco : xt2;

  // divider
  aclk  =                          lfxt1 >> MCUBCP.bcsctl1.b.diva    ;
  mclk  = (MCU_READ_CPUOFF == 0) ? mclk  >> MCUBCP.bcsctl2.b.divm : 0;
  smclk = (MCU_READ_SCG1   == 0) ? smclk >> MCUBCP.bcsctl2.b.divs : 0;

  TRACER_TRACE_ACLK (aclk);
  TRACER_TRACE_MCLK (mclk);
  TRACER_TRACE_SMCLK(smclk);
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

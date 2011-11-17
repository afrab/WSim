/**
 *  \file   msp430_ucs.c
 *  \brief  MSP430x5xx / MSP430x6xx UCS clock module
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_ucs)

#define BITMASK(n) ~(0xfffffffful << n)
#define MCUUCS MCU.ucs
#define NANO   (1000*1000*1000)

static void msp430_ucs_adjust_lfxt1_freq();
static void msp430_ucs_adjust_dcoclk_freq();
static void msp430_ucs_printstate();

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_ucs_create()
{
  msp430_io_register_range16(UCS_START, UCS_END, msp430_ucs_read, msp430_ucs_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
//PUC = Power up Condition?

void msp430_ucs_puc()
{
  MCUUCS.ucsctl0.s = 0x0000; // puc
  MCUUCS.ucsctl1.s = 0x0020; // puc
  MCUUCS.ucsctl2.s = 0x101f; // puc
  MCUUCS.ucsctl3.s = 0x0000; // puc
  MCUUCS.ucsctl4.s = 0x0044; // puc
  MCUUCS.ucsctl5.s = 0x0000; // puc
  MCUUCS.ucsctl6.s = 0xc1cd; // puc
  MCUUCS.ucsctl7.s = 0x0703; // puc
  MCUUCS.ucsctl8.s = 0x0707; // puc
  MCU.sfr.ifg1.b.ofifg = 1; // puc WHAT?
  MCUUCS.ACLKn_bitmask = BITMASK(MCUUCS.ucsctl5.b.diva);

  msp430_ucs_adjust_lfxt1_freq();
  msp430_ucs_adjust_dcoclk_freq();

  MCUUCS.lfxt1_cycle_nanotime = NANO / MCUUCS.lfxt1_freq;
  MCUUCS.xt2_cycle_nanotime = NANO / MCUUCS.xt2_freq;
  MCUUCS.vlo_cycle_nanotime = NANO / MCUUCS.vlo_freq;
  MCUUCS.refo_cycle_nanotime = NANO / MCUUCS.refo_freq;
  MCUUCS.dcoclk_cycle_nanotime = NANO / MCUUCS.dcoclk_freq;

  msp430_ucs_printstate();
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_ucs_reset()
{
  msp430_ucs_puc();
  msp430_ucs_printstate();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * (from slau259B / CC430 User guide)
 * - XT1CLK: Low-frequency oscillator that can be used with low-frequency 
 *   32768-Hz watch crystals
 * - VLOCLK: Internal very low power, low frequency oscillator with 10 kHz 
 *   typical frequency
 * - REFOCLK: Internal, trimmed, low-frequency oscillator with 32768 Hz 
 *   typical frequency, with the ability to be used as a clock reference 
 *   into the FLL
 * - DCOCLK: Internal digitally-controlled oscillator (DCO) that can be 
 *   stabilized by the FLL
 * - XT2CLK: RF XT2 oscillator required for radio functionality
 *
 * - ACLK: Auxiliary clock. The ACLK is software selectable as XT1CLK,
 *   REFOCLK, VLOCLK, DCOCLK, DCOCLKDIV, and when available, XT2CLK.
 *   DCOCLKDIV is the DCOCLK frequency divided by 1, 2, 4,8, 16, or 32
 *   within the FLL block. ACLK can be divided by 1, 2, 4, 8, 16, or 32.
 * - ACLK/n is ACLK divided by 1, 2, 4, 8, 16, or 32 and is available
 *   externally at a pin. ACLK is software selectable by
 *   individual peripheral modules.
 * - MCLK: Master clock. MCLK is software selectable as XT1CLK, REFOCLK,
 *   VLOCLK, DCOCLK, DCOCLKDIV, and XT2CLK. DCOCLKDIV is the DCOCLK
 *   frequency divided by 1, 2, 4, 8, 16, or 32 within the FLL block.
 *   MCLK can be divided by 1, 2, 4, 8, 16, or 32. MCLK is used by the CPU
 *   and system.
 * - SMCLK: Subsystem master clock. SMCLK is software selectable as XT1CLK,
 *   REFOCLK, VLOCLK,DCOCLK, DCOCLKDIV, and XT2CLK. DCOCLKDIV is the DCOCLK
 *   frequency divided by 1, 2, 4, 8, 16, or 32 within the FLL block. SMCLK
 *   can be divided by 1, 2, 4, 8, 16, or 32. SMCLK is software selectable
 *   by individual peripheral modules.
 *
 *
 * fDCOCLK = D * (N + 1) * (fFLLREFCLK / n) ; n=2^FLLREFDIV, D=2^FLLD
 *
 *
 **/

int msp430_ucs_update(int clock_add)
{
  int nano_add = 0;

  /********************************************/
  /* Update nano time                         */
  /********************************************/

  /* switch on MCLK source, compute other clocks */
  /* compute the nano equivalent time            */
  switch (MCUUCS.ucsctl4.b.selm) {
  case 0: /* XT1CLK */
    nano_add = clock_add * MCUUCS.lfxt1_cycle_nanotime;
    break;
  case 1: /* VLOCLK */
    nano_add = clock_add * MCUUCS.vlo_cycle_nanotime;
    break;
  case 2: /* REFOCLK*/
    nano_add = clock_add * MCUUCS.refo_cycle_nanotime;
    break;
  case 3: /* DCOCLK */
    nano_add = (clock_add << MCUUCS.ucsctl2.b.flld) * MCUUCS.dcoclk_cycle_nanotime;
    break;
  case 4: /* DCOCLKDIV */
    nano_add = clock_add * MCUUCS.dcoclk_cycle_nanotime;
    break;
  case 5: /* XT2CLK */
  case 6:
  case 7:
    nano_add = clock_add * MCUUCS.xt2_cycle_nanotime;
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

  /*
   * The VLO is enabled when it is used to source ACLK, MCLK, or SMCLK (SELA = {1} or SELM = {1} or
   * SELS = {1}).
   */
  if (MCUUCS.ucsctl4.b.sela == 4 || MCUUCS.ucsctl4.b.selm == 1 || MCUUCS.ucsctl4.b.sels == 1) {
    CLOCK_DIVMOD_TEMP(MCUUCS.vlo_increment, MCUUCS.vlo_temp, MCUUCS.vlo_cycle_nanotime);
    MCUUCS.vlo_counter += MCUUCS.vlo_increment;
  } else {
    HW_DMSG_CLOCK("msp430:ucs: vlo not updated\n");
  }

  /*
   * XT1 is enabled under any of the following conditions:
   *  - XT1 is a source for ACLK (SELA = {0}) and in active mode (AM) through LPM3 (OSCOFF = 0)
   *  - XT1 is a source for MCLK (SELM = {0}) and in active mode (AM) (CPUOFF = 0)
   *  - XT1 is a source for SMCLK (SELS = {0}) and in active mode (AM) through LPM1 (SMCLKOFF = 0)
   *  - XT1 is a source for FLLREFCLK (SELREF = {0}) and the DCO is a source for ACLK (SELA = {3,4})
   *    and in active mode (AM) through LPM3 (OSCOFF = 0)
   *  - XT1 is a source for FLLREFCLK (SELREF = {0}) and the DCO is a source for MCLK (SELM = {3,4})
   *    and in active mode (AM) (CPUOFF = 0)
   *  - XT1 is a source for FLLREFCLK (SELREF = {0}) and the DCO is a source for SMCLK (SELS = {3,4})
   *    and in active mode (AM) through LPM1 (SMCLKOFF = 0)
   *  - XT1OFF = 0. XT1 enabled in active mode (AM) through LPM4.
   */
  if ((MCU_READ_OSCOFF == 0 && MCUUCS.ucsctl4.b.sela == 0)
    || (MCU_READ_CPUOFF == 0 && MCUUCS.ucsctl4.b.selm == 0)
    || (MCUUCS.ucsctl4.b.sels == 0 && MCUUCS.ucsctl6.b.smclkoff == 0)
    || (MCUUCS.ucsctl3.b.selref == 0 && (MCUUCS.ucsctl4.b.sela == 3 || MCUUCS.ucsctl4.b.sela == 4) && MCU_READ_OSCOFF)
    || (MCUUCS.ucsctl3.b.selref == 0 && (MCUUCS.ucsctl4.b.selm == 3 || MCUUCS.ucsctl4.b.selm == 4) && MCU_READ_CPUOFF)
    || (MCUUCS.ucsctl3.b.selref == 0 && (MCUUCS.ucsctl4.b.sels == 3 || MCUUCS.ucsctl4.b.sels == 4) && MCUUCS.ucsctl6.b.smclkoff == 0)
    ) {
    CLOCK_DIVMOD_TEMP(MCUUCS.lfxt1_increment, MCUUCS.lfxt1_temp, MCUUCS.lfxt1_cycle_nanotime);
    MCUUCS.lfxt1_counter += MCUUCS.lfxt1_increment;
  } else {
    HW_DMSG_CLOCK("msp430:ucs: xt1 not updated\n");
  }

  /*
   * REFO is enabled under any of the following conditions:
   * - REFO is a source for ACLK (SELA = {2}) and in active mode (AM) through LPM3 (OSCOFF = 0)
   * - REFO is a source for MCLK (SELM = {2}) and in active mode (AM) (CPUOFF = 0)
   * - REFO is a source for SMCLK (SELS = {2}) and in active mode (AM) through LPM1 (SMCLKOFF = 0)
   * - REFO is a source for FLLREFCLK (SELREF = {2}) and the DCO is a source for ACLK (SELA = {3,4})
   *   and in active mode (AM) through LPM3 (OSCOFF = 0)
   * - REFO is a source for FLLREFCLK (SELREF = {2}) and the DCO is a source for MCLK (SELM = {3,4})
   *   and in active mode (AM) (CPUOFF = 0)
   * - REFO is a source for FLLREFCLK (SELREF = {2}) and the DCO is a source for SMCLK
   *   (SELS = {3,4}) and in active mode (AM) through LPM1 (SMCLKOFF = 0)
   */
  if ((MCU_READ_OSCOFF == 0 && MCUUCS.ucsctl4.b.sela == 2)
    || (MCU_READ_CPUOFF == 0 && MCUUCS.ucsctl4.b.selm == 2)
    || (MCUUCS.ucsctl4.b.sels == 2 && MCUUCS.ucsctl6.b.smclkoff == 0)
    || (MCUUCS.ucsctl3.b.selref == 2 && (MCUUCS.ucsctl4.b.sela == 3 || MCUUCS.ucsctl4.b.sela == 4) && MCU_READ_OSCOFF)
    || (MCUUCS.ucsctl3.b.selref == 2 && (MCUUCS.ucsctl4.b.selm == 3 || MCUUCS.ucsctl4.b.selm == 4) && MCU_READ_CPUOFF)
    || (MCUUCS.ucsctl3.b.selref == 2 && (MCUUCS.ucsctl4.b.sels == 3 || MCUUCS.ucsctl4.b.sels == 4) && MCUUCS.ucsctl6.b.smclkoff == 0)
    ) {
    CLOCK_DIVMOD_TEMP(MCUUCS.refo_increment, MCUUCS.refo_temp, MCUUCS.refo_cycle_nanotime);
    MCUUCS.refo_counter += MCUUCS.refo_increment;
  } else {
    HW_DMSG_CLOCK("msp430:ucs: refo not updated\n");
  }

  //XT2 TODO: update only if radio is running (on CC430) or on dependencies (in MSP430x5xx/x6xx)
  CLOCK_DIVMOD_TEMP(MCUUCS.xt2_increment, MCUUCS.xt2_temp, MCUUCS.xt2_cycle_nanotime);
  MCUUCS.xt2_counter += MCUUCS.xt2_increment;

  //DCO TODO: find out, when DCO is not updated
  CLOCK_DIVMOD_TEMP(MCUUCS.dcoclk_increment, MCUUCS.dcoclk_temp, MCUUCS.dcoclk_cycle_nanotime);
  MCUUCS.dcoclk_counter += MCUUCS.dcoclk_increment;

  /********************************************/
  /* Update external signals                  */
  /********************************************/

  /* MCLK */
  /* clock_add is given for MCLK */
  if (MCU_READ_CPUOFF == 0) {
    MCUUCS.MCLK_increment = clock_add;
    MCUUCS.MCLK_counter += clock_add;
  }

  /* ACLK */
  switch (MCUUCS.ucsctl4.b.sela) {
  case 0: /* XT1CLK */
    MCUUCS.ACLK_temp += MCUUCS.lfxt1_increment;
    break;
  case 1: /* VLOCLK */
    MCUUCS.ACLK_temp += MCUUCS.vlo_increment;
    break;
  case 2: /* REFOCLK*/
    MCUUCS.ACLK_temp += MCUUCS.refo_increment;
    break;
  case 3: /* DCOCLK */
    MCUUCS.ACLK_temp += (MCUUCS.dcoclk_increment << MCUUCS.ucsctl2.b.flld);
    break;
  case 4: /* DCOCLKDIV */
    MCUUCS.ACLK_temp += MCUUCS.dcoclk_increment;
    break;
  case 5: /* XT2CLK */
  case 6:
  case 7:
    MCUUCS.ACLK_temp += MCUUCS.xt2_increment;
    break;
  }
  MCUUCS.ACLK_increment = MCUUCS.ACLK_temp >> MCUUCS.ucsctl5.b.diva;
  MCUUCS.ACLK_temp &= MCUUCS.ACLK_bitmask;
  MCUUCS.ACLK_counter += MCUUCS.ACLK_increment;

  /* ACLKn */
  MCUUCS.ACLKn_temp += MCUUCS.ACLK_increment;
  MCUUCS.ACLKn_increment = MCUUCS.ACLKn_temp >> MCUUCS.ucsctl5.b.divpa;
  MCUUCS.ACLKn_temp &= MCUUCS.ACLKn_bitmask;
  MCUUCS.ACLKn_counter += MCUUCS.ACLKn_increment;

  /* SMCLK */
  switch (MCUUCS.ucsctl4.b.sels) {
  case 0: /* XT1CLK */
    MCUUCS.SMCLK_temp += MCUUCS.lfxt1_increment;
    break;
  case 1: /* VLOCLK */
    MCUUCS.SMCLK_temp += MCUUCS.vlo_increment;
    break;
  case 2: /* REFOCLK*/
    MCUUCS.SMCLK_temp += MCUUCS.refo_increment;
    break;
  case 3: /* DCOCLK */
    MCUUCS.SMCLK_temp += (MCUUCS.dcoclk_increment << MCUUCS.ucsctl2.b.flld);
    break;
  case 4: /* DCOCLKDIV */
    MCUUCS.SMCLK_temp += MCUUCS.dcoclk_increment;
    break;
  case 5: /* XT2CLK */
  case 6:
  case 7:
    MCUUCS.SMCLK_temp += MCUUCS.xt2_increment;
    break;
  }
  MCUUCS.SMCLK_increment = MCUUCS.SMCLK_temp >> MCUUCS.ucsctl5.b.divs;
  MCUUCS.SMCLK_temp &= MCUUCS.SMCLK_bitmask;
  MCUUCS.SMCLK_counter += MCUUCS.SMCLK_increment;

  return nano_add;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_ucs_update_done()
{
  MCUUCS.lfxt1_increment = 0;
  MCUUCS.xt2_increment = 0;
  MCUUCS.vlo_increment = 0;
  MCUUCS.refo_increment = 0;
  MCUUCS.dcoclk_increment = 0;

  MCUUCS.MCLK_increment = 0;
  MCUUCS.ACLK_increment = 0;
  MCUUCS.ACLKn_increment = 0;
  MCUUCS.SMCLK_increment = 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t msp430_ucs_read(uint16_t addr)
{
  uint16_t res = 0;
  switch (addr) {
  case UCSCTL0:
    res = MCUUCS.ucsctl0.s;
    break;
  case UCSCTL1:
    res = MCUUCS.ucsctl1.s;
    break;
  case UCSCTL2:
    res = MCUUCS.ucsctl2.s;
    break;
  case UCSCTL3:
    res = MCUUCS.ucsctl3.s;
    break;
  case UCSCTL4:
    res = MCUUCS.ucsctl4.s;
    break;
  case UCSCTL5:
    res = MCUUCS.ucsctl5.s;
    break;
  case UCSCTL6:
    res = MCUUCS.ucsctl6.s;
    break;
  case UCSCTL7:
    res = MCUUCS.ucsctl7.s;
    break;
  case UCSCTL8:
    res = MCUUCS.ucsctl8.s;
    break;
  default:
    ERROR("msp430:ucs: wrong read address 0x%04x\n", addr);
    break;
  }
  HW_DMSG_CLOCK("msp430:ucs: read : [0x%04x] = 0x%02x\n", addr, res);
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_ucs_write(uint16_t addr, int16_t val)
{
  switch (addr) {
  case UCSCTL0:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL1:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL2:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL3:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL4:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL5:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL6:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL7:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  case UCSCTL8:
    MCUUCS.ucsctl0.s = val;
    msp430_ucs_adjust_dcoclk_freq();
    break;
  default:
    ERROR("msp430:ucs: wrong write address [0x%04x]=0x%02x\n", addr, val & 0xff);
    break;
  }
  //TODO reactions on writes
  HW_DMSG_CLOCK("msp430:ucs: write : [0x%04x] = 0x%02x\n", addr, val & 0xff);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

static void msp430_ucs_adjust_lfxt1_freq()
{
  MCUUCS.lfxt1_cycle_nanotime = NANO / MCUUCS.lfxt1_freq;
  msp430_ucs_adjust_dcoclk_freq();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * fDCOCLK = D * (N + 1) * (fFLLREFCLK / n) ; n ~ 2^FLLREFDIV, D=2^FLLD
 */
static void msp430_ucs_adjust_dcoclk_freq()
{
  int fllref_freq, n, D;
  switch (MCUUCS.ucsctl3.b.selref) {
  case 0:
  case 1: /*XT1*/
    fllref_freq = MCUUCS.lfxt1_freq;
    break;
  case 2:
  case 3:
  case 4: /*refo clk*/
    fllref_freq = MCUUCS.refo_freq;
    break;
  case 5:
  case 6:
  case 7:
    fllref_freq = MCUUCS.xt2_freq;
  }

  switch (MCUUCS.ucsctl3.b.fllrefdiv) {
  case 0:
    n = 1;
    break;
  case 1:
    n = 2;
    break;
  case 2:
    n = 4;
    break;
  case 3:
    n = 8;
    break;
  case 4:
    n = 12;
    break;
  case 5:
  case 6:
  case 7:
    n = 16;
    break;
  }

  switch (MCUUCS.ucsctl2.b.flld) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    D = 1 << MCUUCS.ucsctl2.b.flld;
    break;
  case 6:
  case 7:
    D = 32;
    break;
  }
  MCUUCS.dcoclk_freq = D * (MCUUCS.ucsctl2.b.flln + 1) * (fllref_freq / n);
  MCUUCS.dcoclk_cycle_nanotime = NANO / MCUUCS.dcoclk_freq;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

static void msp430_ucs_printstate()
{
  HW_DMSG_CLOCK("msp430:ucs: === UCS state === PC 0x%04x\n", mcu_get_pc());

  HW_DMSG_CLOCK("msp430:ucs: MCLK  ");
  HW_DMSG_CLOCK("    src:");
  switch (MCUUCS.ucsctl4.b.selm) {
  case 0: HW_DMSG_CLOCK("xt1");
    break;
  case 1: HW_DMSG_CLOCK("vlo");
    break;
  case 2: HW_DMSG_CLOCK("refo");
    break;
  case 3: HW_DMSG_CLOCK("dco");
    break;
  case 4: HW_DMSG_CLOCK("dcodiv");
    break;
  case 5:
  case 6:
  case 7: HW_DMSG_CLOCK("xt2");
    break;
  }
  HW_DMSG_CLOCK("    run:%s\n", (MCU_READ_CPUOFF == 0) ? "on" : "off");

  HW_DMSG_CLOCK("msp430:ucs: === End of UCS state ===\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_ucs */


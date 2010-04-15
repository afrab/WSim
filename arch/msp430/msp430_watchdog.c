
/**
 *  \file   msp430_watchdog.c
 *  \brief  MSP430 Watchdog timer definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <assert.h> 
#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(DEBUG_WATCHDOG)
static char *str_ssel[] = { "SMCLK", "ACLK" };
#endif

static int wdt_intervals[] = { 32768, 8192, 512, 64 };

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void 
msp430_watchdog_reset(void)
{
  MCU.watchdog.wdtctl.s    = 0x6900u;
  MCU.watchdog.wdtcnt      = 0;
  MCU.watchdog.wdtinterval = wdt_intervals[MCU.watchdog.wdtctl.b.wdtis];
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */



#define WDT_NMI_RST       0
#define WDT_NMI_NMI       1


#define WDT_SSEL_SMCLK  0
#define WDT_SSEL_ACLK   1

void 
msp430_watchdog_update(void)
{
  if (MCU.watchdog.wdtctl.b.wdthold)
    return ;

  switch (MCU.watchdog.wdtctl.b.wdtssel)
    {
    case WDT_SSEL_SMCLK:
      MCU.watchdog.wdtcnt += MCU_CLOCK.SMCLK_increment;
      break;
    case WDT_SSEL_ACLK:
      MCU.watchdog.wdtcnt += MCU_CLOCK.ACLK_increment;
      break;
    }

  if (MCU.watchdog.wdtcnt > MCU.watchdog.wdtinterval)
    {
      HW_DMSG_WD("msp430:watchdog: interval wrapping\n");
      if (MCU.watchdog.wdtctl.b.wdttmsel == WDT_MODE_WATCHDOG)
	{
#define NO_WATCHDOG_ONLY_WARNS
#if defined(WATCHDOG_ONLY_WARNS)
	  WARNING("msp430:watchdog: =======================================\n");
	  WARNING("msp430:watchdog: set interrupt RESET\n");
	  WARNING("msp430:watchdog: =======================================\n");
	  MCU.watchdog.wdtcnt = 0;
	  MCU.watchdog.wdtctl.b.wdttmsel = WDT_MODE_INTERVAL;
#else
	  msp430_interrupt_set(INTR_RESET);
	  MCU.sfr.ifg1.b.wdtifg = 1;
#endif
	}
      else /* interval */
	{
	  MCU.sfr.ifg1.b.wdtifg = 1;
	  if (MCU.sfr.ie1.b.wdtie)
	    {
	      msp430_interrupt_set(INTR_WATCHDOG);
	    }
	}

      MCU.watchdog.wdtcnt -= MCU.watchdog.wdtinterval;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t 
msp430_watchdog_read (uint16_t addr)
{
  assert(addr == WDTCTL);
  HW_DMSG_WD("msp430:watchdog: read wdtctl\n");
  return MCU.watchdog.wdtctl.s;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define WD_PASSWORD 0x5a

void 
msp430_watchdog_write(uint16_t addr, int16_t val)
{
  union {
    struct wdtctl_t b;
    uint16_t        s;
  } wdtctl;

  assert(addr == WDTCTL);
  wdtctl.s = val;

  HW_DMSG_WD("msp430:watchdog: wdtctl = 0x%04x\n",val);
  if (wdtctl.b.wdtpw != WD_PASSWORD)
    {
      ERROR("msp430:watchdog: security key violation, PUC triggered\n");
      mcu_reset();
    }

  if (wdtctl.b.wdthold != MCU.watchdog.wdtctl.b.wdthold)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdthold 0x%02x\n",wdtctl.b.wdthold);
    }
  if (wdtctl.b.wdtnmies != MCU.watchdog.wdtctl.b.wdtnmies)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtnmies 0x%02x\n",wdtctl.b.wdtnmies);
      ERROR("msp430:watchdog: control of RST/NMI pin not implemented\n");
    }
  if (wdtctl.b.wdtnmi != MCU.watchdog.wdtctl.b.wdtnmi)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtnmi 0x%02x\n",wdtctl.b.wdtnmi);
      ERROR("msp430:watchdog: control of RST/NMI pin not implemented\n");
    }
  if (wdtctl.b.wdttmsel != MCU.watchdog.wdtctl.b.wdttmsel)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdttmsel 0x%02x\n",wdtctl.b.wdttmsel);
    }
  if (wdtctl.b.wdtcntcl != MCU.watchdog.wdtctl.b.wdtcntcl)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtcntcl 0x%02x (clear)\n",wdtctl.b.wdtcntcl);
      MCU.watchdog.wdtcnt = 0;
      wdtctl.b.wdtcntcl   = 0; 
    }
  if (wdtctl.b.wdtssel != MCU.watchdog.wdtctl.b.wdtssel)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtssel 0x%02x (%s)\n",
		 wdtctl.b.wdtssel,str_ssel[wdtctl.b.wdtssel]);
    }
  if (wdtctl.b.wdtis != MCU.watchdog.wdtctl.b.wdtis)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtis 0x%02x (/%d)\n",
		 wdtctl.b.wdtis, wdt_intervals[wdtctl.b.wdtis]);
      MCU.watchdog.wdtinterval = wdt_intervals[wdtctl.b.wdtis];
    }

  MCU.watchdog.wdtctl.s = wdtctl.s; /* wdtctl.s is modified during write */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum watchdog_mode_t msp430_watchdog_getmode(void)
{ 
  return MCU.watchdog.wdtctl.b.wdttmsel; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_watchdog_chkifg()
{
  if (MCU.sfr.ifg1.b.wdtifg && MCU.sfr.ie1.b.wdtie)
    {
      msp430_interrupt_set(INTR_WATCHDOG);
      return 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

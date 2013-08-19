
/**
 *  \file   msp430_watchdog.c
 *  \brief  MSP430 Watchdog timer definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <assert.h> 
#include "arch/common/hardware.h"
#include "msp430.h"

#if DEBUG_WATCHDOG
static char *str_ssel[] = { "SMCLK", "ACLK" };
#endif

#if defined(__msp430_have_watchdog)

// GCC Sourcecode definitions for watchdog intervals
// #define WDTIS__2G         (0x0000)  /* WDT - Timer Interval Select: /2G */
// #define WDTIS__128M       (0x0001)  /* WDT - Timer Interval Select: /128M */
// #define WDTIS__8192K      (0x0002)  /* WDT - Timer Interval Select: /8192k */
// #define WDTIS__512K       (0x0003)  /* WDT - Timer Interval Select: /512k */
// #define WDTIS__32K        (0x0004)  /* WDT - Timer Interval Select: /32k */
// #define WDTIS__8192       (0x0005)  /* WDT - Timer Interval Select: /8192 */
// #define WDTIS__512        (0x0006)  /* WDT - Timer Interval Select: /512 */
// #define WDTIS__64         (0x0007)  /* WDT - Timer Interval Select: /64 */

static int wdt_intervals[] = { 1073741824, 16777216, 8388608, 524288, 32768, 8192, 512, 64 };

/* ************************************************** */
/* defined WATCHDOG_ONLY_WARNS to only get warning    */
/* messages instead of reboots                        */
/* ************************************************** */

#define WATCHDOG_ONLY_WARNS 0

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void 
msp430_watchdog_create(void)
{
  msp430_io_register_range16(WATCHDOG_START,WATCHDOG_END,msp430_watchdog_read,msp430_watchdog_write);
  msp430_io_register_range8(WATCHDOG_START, WATCHDOG_END, msp430_watchdog_read8, msp430_watchdog_write8);
}


void 
msp430_watchdog_reset(void)
{
  MCU.watchdog.wdtctl.b.wdthold = 1; //disable watchdog on reset
  MCU.watchdog.wdtcnt      = 0;
  MCU.watchdog.wdtinterval = wdt_intervals[MCU.watchdog.wdtctl.b.wdtis];
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */



#define WDT_NMI_RST       0
#define WDT_NMI_NMI       1

void 
msp430_watchdog_update(void)
{
	if (MCU.watchdog.wdtctl.b.wdthold)
		return;
  
	#define WDT_SSEL_SMCLK  0 /* Watchdog clock SMCLK */
	#define WDT_SSEL_ACLK   1 /* Watchdog clock ACLK */
	#define WDT_SSEL_VLOCLK   2 /* Watchdog clock VLOCLK */
	#define WDT_SSEL_X_CLK   3 /* Watchdog clock X_CLK */
	switch (MCU.watchdog.wdtctl.b.wdtssel)
	{
		case WDT_SSEL_SMCLK:
		MCU.watchdog.wdtcnt += MCU_CLOCK.SMCLK_increment;
		break;
		case WDT_SSEL_ACLK:
		MCU.watchdog.wdtcnt += MCU_CLOCK.ACLK_increment;
		break;
		#ifdef WATCHDOG_THREE_BITS_WDTIS
		case WDT_SSEL_VLOCLK:
		MCU.watchdog.wdtcnt += MCU_CLOCK.ACLK_increment;
		break;
		case WDT_SSEL_X_CLK:
		MCU.watchdog.wdtcnt += MCU_CLOCK.ACLK_increment;
		break;
		#endif
	}

	if (MCU.watchdog.wdtcnt > MCU.watchdog.wdtinterval)
	{
		HW_DMSG_WD("msp430:watchdog: interval wrapping\n");
		if (MCU.watchdog.wdtctl.b.wdttmsel == WDT_MODE_WATCHDOG)
		{
			#if WATCHDOG_ONLY_WARNS != 0
			WARNING("msp430:watchdog: =======================================\n");
			WARNING("msp430:watchdog: set interrupt RESET\n");
			WARNING("msp430:watchdog: =======================================\n");
			MCU.watchdog.wdtcnt = 0;
			MCU.watchdog.wdtctl.b.wdttmsel = WDT_MODE_INTERVAL;
			#else
			//ERROR("msp430:watchdog reset! %u to %u\n", MCU.watchdog.wdtcnt, MCU.watchdog.wdtinterval);
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

#define WD_PASSWORD_READ 0x69
/**
 * From documentation:
   Reading WDTCTL returns 0x69 in the upper byte, so reading WDTCTL and writing the value back violates,
   the password and causes a reset.
 */
int16_t 
msp430_watchdog_read (uint16_t addr)
{
  assert(addr == WDTCTL);
  
  // copy wdtctl, set upper byte to WD_PASSWORD_READ, set wdtcntcl to 0
  union {
    struct wdtctl_t b;
    uint16_t        s;
  } wdtctl;
  wdtctl.s = MCU.watchdog.wdtctl.s;
  wdtctl.b.wdtcntcl = 0; // always reads as 0
  wdtctl.b.wdtpw = WD_PASSWORD_READ;
  
  HW_DMSG_WD("msp430:watchdog: read wdtctl\n");
  return wdtctl.s;
}

int8_t msp430_watchdog_read8(uint16_t addr) {
	assert(addr == WDTCTL);
	//ERROR("msp430:watchdog: read at 0x%04x\n",addr);
	return MCU.watchdog.wdtctl.s & 0xff;
}

void msp430_watchdog_write8(uint16_t addr, int8_t val) {}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define WD_PASSWORD 0x5a

void 
msp430_watchdog_write(uint16_t addr, int16_t val)
{
  assert(addr == WDTCTL);
  
  union {
    struct wdtctl_t b;
    uint16_t        s;
  } wdtctl;
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
	  //ERROR("msp430:watchdog: wdtctl.wdthold 0x%02x\n",wdtctl.b.wdthold);
    }
  #ifndef WATCHDOG_THREE_BITS_WDTIS
  if (wdtctl.b.wdtnmies != MCU.watchdog.wdtctl.b.wdtnmies)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtnmies 0x%02x\n",wdtctl.b.wdtnmies);
      ERROR("msp430:watchdog: control of RST/NMI pin not implemented: wdtctl.wdtnmies 0x%02x\n",wdtctl.b.wdtnmies);
    }
  if (wdtctl.b.wdtnmi != MCU.watchdog.wdtctl.b.wdtnmi)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtnmi 0x%02x\n",wdtctl.b.wdtnmi);
      ERROR("msp430:watchdog: control of RST/NMI pin not implemented: wdtctl.wdtnmies 0x%02x\n",wdtctl.b.wdtnmi);
    }
  #endif
  if (wdtctl.b.wdttmsel != MCU.watchdog.wdtctl.b.wdttmsel)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdttmsel 0x%02x\n",wdtctl.b.wdttmsel);
    }
  if (wdtctl.b.wdtcntcl != MCU.watchdog.wdtctl.b.wdtcntcl)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtcntcl 0x%02x (clear)\n",wdtctl.b.wdtcntcl);
      MCU.watchdog.wdtcnt = 0;
      wdtctl.b.wdtcntcl   = 0;
	  //ERROR("msp430:watchdog clear!\n");
    }
  if (wdtctl.b.wdtssel != MCU.watchdog.wdtctl.b.wdtssel)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtssel 0x%02x (%s)\n",
		 wdtctl.b.wdtssel,str_ssel[wdtctl.b.wdtssel]);
    }
  if (wdtctl.b.wdtis != MCU.watchdog.wdtctl.b.wdtis)
    {
      HW_DMSG_WD("msp430:watchdog: wdtctl.wdtis 0x%02x (/%d)\n", wdtctl.b.wdtis, wdt_intervals[wdtctl.b.wdtis]);
      MCU.watchdog.wdtinterval = wdt_intervals[wdtctl.b.wdtis];
	  //ERROR("msp430:watchdog interval set to index %u with value %u!\n", wdtctl.b.wdtis, MCU.watchdog.wdtinterval);
    }

  /* wdtctl.s is modified during write */
  MCU.watchdog.wdtctl.s = wdtctl.s;
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
#endif

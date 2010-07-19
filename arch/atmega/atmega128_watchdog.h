
/**
 *  \file   atmega128_watchdog.h
 *  \brief  Atmega128 Watchdog timer definition 
 *  \author Joe Nassimian
 *  \date   2010
 **/

#ifndef ATMEGA128_WATCHDOG_H
#define ATMEGA128_WATCHDOG_H

#if defined(__atmega128_have_watchdog)

#define WATCHDOG_START 0x0120
#define WATCHDOG_END   0x0120

#define WDTCTL 0x120

enum watchdog_mode_t {
  WDT_MODE_WATCHDOG = 0,
  WDT_MODE_INTERVAL = 1
};

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) wdtctl_t {
  uint16_t
    wdtpw:8,
    wdthold:1,
    wdtnmies:1,
    wdtnmi:1,
    wdttmsel:1,
    wdtcntcl:1,
    wdtssel:1,
    wdtis:2;
};
#else
struct __attribute__ ((packed)) wdtctl_t {
  uint16_t
    wdtis:2,
    wdtssel:1,
    wdtcntcl:1,
    wdttmsel:1,
    wdtnmi:1,
    wdtnmies:1,
    wdthold:1,
    wdtpw:8;
};
#endif

/**
 * Watchdog Data Structure
 **/
struct msp430_watchdog_t
{
  union {
    struct wdtctl_t b;
    uint16_t        s;
  } wdtctl;
  
  /* we keep int for counters to detect overflow */
  int wdtcnt;
  int wdtinterval;
};

void    atmega128_watchdog_reset  (void);
void    atmega128_watchdog_update (void);
int16_t atmega128_watchdog_read   (uint16_t addr);
void    atmega128_watchdog_write  (uint16_t addr, int16_t val);

enum watchdog_mode_t msp430_watchdog_getmode(void);
/* #define msp430_watchdog_getmode() MCU.watchdog.wdtctl.b.wdttmsel */

/* ifg flag is in sfr registers */
int     msp430_watchdog_chkifg();

#endif /* defined */
#endif /* header */

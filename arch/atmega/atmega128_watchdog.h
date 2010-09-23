
/**
 *  \file   atmega128_watchdog.h
 *  \brief  Atmega128 Watchdog timer definition 
 *  \author Joe Nassimian
 *  \date   2010
 **/

#ifndef ATMEGA128_WATCHDOG_H
#define ATMEGA128_WATCHDOG_H

#if defined(__atmega128_have_watchdog)

// If MCU has wathdog, lines to be added in mcu_t structure need to go here
#define MCU_WATCHDOG()        \
    struct atmega128_watchdog_t     watchdog;

#define WATCHDOG_START 0x41

#define WDTCR 0x41

enum watchdog_mode_t {
  WDT_MODE_WATCHDOG = 0,
  WDT_MODE_INTERVAL = 1
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
// Watchdog timer control register

struct __attribute__ ((packed)) wdtcr_t {
  uint8_t
    wdp0:1,
    wdp1:1,
    wdp2:1,
    wde: 1,
    wdce:1,
    reserved:3;
};

/**
 * Watchdog Data Structure
 **/
struct atmega128_watchdog_t
{
  union {
    struct wdtcr_t b;
    uint8_t        s;
  } wdtcr;
  
  /* we keep int for counters to detect overflow */
  int wdtcnt;
  int wdtinterval;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    atmega128_watchdog_init   (void);

void    atmega128_watchdog_reset  (void);
void    atmega128_watchdog_update (void);

int16_t atmega128_watchdog_read   (uint16_t addr);
void    atmega128_watchdog_write  (uint16_t addr, int16_t val);

/* ifg flag is in sfr registers */
int     atmega128_watchdog_chkifg();

#else
// In case no watchdogs in mcu, define empty macro
#define MCU_WATCHDOG()
#endif /* defined */
#endif /* header */


/**
 *  \file   msp430_watchdog.h
 *  \brief  MSP430 Watchdog timer definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_WATCHDOG_H
#define MSP430_WATCHDOG_H

#if defined(__msp430_have_watchdog)

#if !defined(WATCHDOG_BASE)
#define WATCHDOG_BASE 0x0120
#endif

#define WATCHDOG_START WATCHDOG_BASE
#define WATCHDOG_END   WATCHDOG_BASE

#define WDTCTL WATCHDOG_BASE

enum watchdog_mode_t {
  WDT_MODE_WATCHDOG = 0,
  WDT_MODE_INTERVAL = 1
};

#ifdef WATCHDOG_THREE_BITS_WDTIS // some msp430 have three bytes for watchdog interval selection

	// GCC Definitions
	// #define WDTIS0              (0x0001)  /* WDT - Timer Interval Select 0 */
	// #define WDTIS1              (0x0002)  /* WDT - Timer Interval Select 1 */
	// #define WDTIS2              (0x0004)  /* WDT - Timer Interval Select 2 */
	// #define WDTCNTCL            (0x0008)  /* WDT - Timer Clear */
	// #define WDTTMSEL            (0x0010)  /* WDT - Timer Mode Select */
	// #define WDTSSEL0            (0x0020)  /* WDT - Timer Clock Source Select 0 */
	// #define WDTSSEL1            (0x0040)  /* WDT - Timer Clock Source Select 1 */
	// #define WDTHOLD             (0x0080)  /* WDT - Timer hold */

	#if defined(WORDS_BIGENDIAN)
	struct __attribute__ ((packed)) wdtctl_t {
	uint16_t
		wdtpw:8,
		wdthold:1,
		wdtssel:2,
		wdttmsel:1,
		wdtcntcl:1,
		wdtis:3;
	};
	#else
	struct __attribute__ ((packed)) wdtctl_t {
	uint16_t
		wdtis:3, /* WDT - Timer Interval Select 0-2 */
		wdtcntcl:1, /* WDT - Timer Clear */
		wdttmsel:1, /* WDT - Timer Mode Select */
		wdtssel:2, /* WDT - Timer Clock Source Select 0-1 */
		wdthold:1, /* WDT - Timer hold */
		wdtpw:8;
	};
	#endif
#else
	// GCC Definitions
	// #define WDTIS0              (0x0001) /* WDT - Timer Interval Select 0 */
	// #define WDTIS1              (0x0002) /* WDT - Timer Interval Select 1 */
	// #define WDTSSEL             (0x0004) /* WDT - Timer Clock Source Select 0 */
	// #define WDTCNTCL            (0x0008) /* WDT - Timer Clear */
	// #define WDTTMSEL            (0x0010) /* WDT - Timer Mode Select */
	// #define WDTNMI              (0x0020) /* output pin */
	// #define WDTNMIES            (0x0040) /* output pin */
	// #define WDTHOLD             (0x0080) /* WDT - Timer hold */

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

void    msp430_watchdog_create (void);
void    msp430_watchdog_reset  (void);
void    msp430_watchdog_update (void);
int16_t msp430_watchdog_read   (uint16_t addr);
int8_t msp430_watchdog_read8   (uint16_t addr);
void    msp430_watchdog_write  (uint16_t addr, int16_t val);
void msp430_watchdog_write8(uint16_t addr, int8_t val);

enum watchdog_mode_t msp430_watchdog_getmode(void);
int     msp430_watchdog_chkifg ();

#else
#define msp430_watchdog_create() do { } while (0)
#define msp430_watchdog_reset()  do { } while (0)
#endif /* defined */
#endif /* header */


/**
 *  \file   msp430_basic_timer.h
 *  \brief  MSP430 Basic Timer definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_BASIC_TIMER_H
#define MSP430_BASIC_TIMER_H

#if defined(__msp430_have_basic_timer)

#define BASIC_TIMER_START 0x046
#define BASIC_TIMER_END   0x047

#define BT_CTL  0x40
#define BT_CNT1 0x46
#define BT_CNT2 0x47

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) btctl_t {
  uint8_t
    btssel:1,
    bthold:1,
    btdiv:1,
    btfrfq:2,
    btip:3;
};
#else
struct __attribute__ ((packed)) btctl_t {
  uint8_t
    btip:3,
    btfrfq:2,
    btdiv:1,
    bthold:1,
    btssel:1;
};
#endif

/**
 * Basic_Timer Data Structure
 **/
struct msp430_basic_timer_t
{
  union {
    struct btctl_t b;
    uint8_t        s;
  } ctl;

  int32_t ctl1;  /* 0x46 */
  int32_t ctl2;  /* 0x47 */
};


void   msp430_basic_timer_reset ();
void   msp430_basic_timer_update();
int8_t msp430_basic_timer_read  (uint16_t addr);
void   msp430_basic_timer_write (uint16_t addr, int8_t val);
int    msp430_basic_timer_chkifg();

#endif
#endif

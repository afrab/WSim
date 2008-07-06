
/**
 *  \file   msp430_timer.h
 *  \brief  MSP430 Timers definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_TIMER_H
#define MSP430_TIMER_H


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) tiv_t {
  uint16_t
    dummy2:12,
    tiv:3,
    dummy1:1;
};
#else
struct __attribute__ ((packed)) tiv_t {
  uint16_t
    dummy1:1,
    tiv:3,
    dummy2:12;
};
#endif /* defined(WORDS_BIGENDIAN) */

enum timer_mode_t {
  TIMER_STOP           = 0x0, /* stop                           */
  TIMER_UP             = 0x1, /* up       : from 0 to CCR0      */
  TIMER_CONT           = 0x2, /* continue : from 0 to [limit]   */
  TIMER_UD             = 0x3  /* up/down  : from 0 to CCR0 to 0 */
};

enum timer_ud_mode_t {
  TIMER_UD_UP          = 0x0,
  TIMER_UD_DOWN        = 0x1
};

enum timer_source_t {
  TIMER_SOURCE_TxCLK   = 0x0,
  TIMER_SOURCE_ACLK    = 0x1,
  TIMER_SOURCE_SMCLK   = 0x2,
  TIMER_SOURCE_INTxCLK = 0x3
};

/***************************************************/
/** Timer A3 ***************************************/
/***************************************************/
#if defined(__msp430_have_timera3)

/* Timer1_A3 - Timer 0 */

enum timerA3_addr_t {
  TAIV      = 0x012e, /* read only */

  TACTL     = 0x0160,
  TACCTL0   = 0x0162,
  TACCTL1   = 0x0164,
  TACCTL2   = 0x0166,
  TA_RES1   = 0x0168, /* reserved */
  TA_RES2   = 0x016a, /* reserved */
  TA_RES3   = 0x016c, /* reserved */
  TA_RES4   = 0x016e, /* reserved */

  TAR       = 0x0170,
  TACCR0    = 0x0172,
  TACCR1    = 0x0174,
  TACCR2    = 0x0176,
  TA_RES5   = 0x0178, /* reserved */
  TA_RES6   = 0x017a, /* reserved */
  TA_RES7   = 0x017c, /* reserved */
  TA_RES8   = 0x017e  /* reserved */
};

#define TIMER_A3_START 0x0160
#define TIMER_A3_END   0x017e

/**
 * Timer A Data Structure
 */

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) tactl_t {
  uint16_t
    padding:6, /* 16 */
    tassel:2,
    id:2,      /* 8 */  /** input divider    */
    mc:2,               /** mode             */
    unused:1,  /* 4 */
    taclr:1,            /** clear            */
    taie:1,             /** interrupt enable */
    taifg:1;
};
#else
struct __attribute__ ((packed)) tactl_t {
  uint16_t
    taifg:1,
    taie:1,             /** interrupt enable */
    taclr:1,            /** clear            */
    unused:1,  /* 4 */
    mc:2,               /** mode             */
    id:2,      /* 8 */  /** input divider    */
    tassel:2,
    padding:6; /* 16 */
};
#endif

#if defined(WORDS_BIGENDIAN)
struct  __attribute__ ((packed)) tacctl_t {
  uint16_t
    cm:2,      /* 16 */
    ccis:2,
    scs:1,     /* 12 */
    scci:1,
    dummy:1,
    cap:1,
    outmod:3,  /* 8 */
    ccie:1,
    cci:1,     /* 4 */
    out:1,
    cov:1,
    ccifg:1;
};
#else
struct  __attribute__ ((packed)) tacctl_t {
  uint16_t
    ccifg:1,
    cov:1,
    out:1,
    cci:1,     /* 4 */
    ccie:1,
    outmod:3,  /* 8 */
    cap:1,
    dummy:1,
    scci:1,
    scs:1,     /* 12 */
    ccis:2,
    cm:2;      /* 16 */
};
#endif

union tacctlu_t {
    struct tacctl_t  b;
    uint16_t         s;
};

struct msp430_timerA3_t 
{
  union {
    struct tactl_t   b;
    uint16_t         s;
  } tactl;                 /* 0x160 */

  union tacctlu_t tacctl0; /* 0x162 */
  union tacctlu_t tacctl1; /* 0x164 */
  union tacctlu_t tacctl2; /* 0x166 */

  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int      tar;            /* 0x170 */
  
  int      taccr0;         /* 0x172 */
  int      taccr1;         /* 0x174 */
  int      taccr2;         /* 0x176 */

  int      b_taccr0;  /* used in compare mode to detect */
  int      b_taccr1;
  int      b_taccr2;

  int      equ0;
  int      equ1;
  int      equ2;

  union {
    struct tiv_t     b;   
    uint16_t         s;
  } tiv;                   /* 0x12e */

  enum timer_ud_mode_t  udmode;
};

void    msp430_timerA3_reset (void);
void    msp430_timerA3_update(void);
int16_t msp430_timerA3_read  (uint16_t addr);
void    msp430_timerA3_write (uint16_t addr, int16_t val);
int8_t  msp430_timerA3_read8 (uint16_t addr);
void    msp430_timerA3_write8(uint16_t addr, int8_t val);
int     msp430_timerA3_chkifg();

#endif

/***************************************************/
/** Timer A5 ***************************************/
/***************************************************/
#if defined(__msp430_have_timera5)

enum timerA5_addr_t {
  TA1IV     = 0x011e,
  TA1CTL    = 0x0180,
  TA1CCTL0  = 0x0182,
  TA1CCTL1  = 0x0184,
  TA1CCTL2  = 0x0186,
  TA1CCTL3  = 0x0188,
  TA1CCTL4  = 0x018a,
  TA1R      = 0x0190,
  TA1CCR0   = 0x0192,
  TA1CCR1   = 0x0194,
  TA1CCR2   = 0x0196,
  TA1CCR3   = 0x0198,
  TA1CCR4   = 0x019a
};

#define TIMER_A5_START  0x180
#define TIMER_A5_END    0x19e

struct msp430_timerA5_t  
{
};

void    msp430_timerA5_reset (void);
void    msp430_timerA5_update(void);
int16_t msp430_timerA5_read  (uint16_t addr);
void    msp430_timerA5_write (uint16_t addr, int16_t val);
#define msp430_timerA5_chkifg() 0

#endif /* have_timera5 */

/***************************************************/
/** Timer B ****************************************/
/***************************************************/

#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)

#if defined(__msp430_have_timerb7)
#define TIMERBNAME "timerB7"
#else
#define TIMERBNAME "timerB3"
#endif

enum timerB_addr_t {
 TBIV      = 0x011e,
 TBCTL     = 0x0180,
 TBCCTL0   = 0x0182,
 TBCCTL1   = 0x0184,
 TBCCTL2   = 0x0186,
 TBCCTL3   = 0x0188,
 TBCCTL4   = 0x018a,
 TBCCTL5   = 0x018c,
 TBCCTL6   = 0x018e,
 TBR       = 0x0190,
 TBCCR0    = 0x0192,
 TBCCR1    = 0x0194,
 TBCCR2    = 0x0196,
 TBCCR3    = 0x0198,
 TBCCR4    = 0x019a,
 TBCCR5    = 0x019c,
 TBCCR6    = 0x019e
};

#define TIMER_B_START  0x180
#define TIMER_B_END    0x19e

/* tbr length is selectable */
#define TBR_8  0xffu
#define TBR_10 0x3ffu
#define TBR_12 0xfffu
#define TBR_16 0xffffu

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) tbctl_t {
  uint16_t
    padding:1, /* 16 */
    tbclgrp:2,
    cntl:2,
    padding2:1,
    tbssel:2,
    id:2,      /* 8 */  /** input divider    */
    mc:2,               /** mode             */
    unused:1,  /* 4 */
    tbclr:1,            /** clear            */
    tbie:1,             /** interrupt enable */
    tbifg:1;
};
#else
struct __attribute__ ((packed)) tbctl_t {
  uint16_t
    tbifg:1,
    tbie:1,             /** interrupt enable */
    tbclr:1,            /** clear            */
    unused:1,  /* 4 */
    mc:2,               /** mode             */
    id:2,      /* 8 */  /** input divider    */
    tbssel:2,
    padding2:1,
    cntl:2,
    tbclgrp:2,
    padding:1; /* 16 */
};
#endif

#if defined(WORDS_BIGENDIAN)
struct  __attribute__ ((packed)) tbcctl_t {
  uint16_t
    cm:2,      /* 16 */
    ccis:2,
    scs:1,     /* 12 */
    clld:2,
    cap:1,
    outmod:3,  /* 8 */
    ccie:1,
    cci:1,     /* 4 */
    out:1,
    cov:1,
    ccifg:1;
};
#else
struct  __attribute__ ((packed)) tbcctl_t {
  uint16_t
    ccifg:1,
    cov:1,
    out:1,
    cci:1,     /* 4 */
    ccie:1,
    outmod:3,  /* 8 */
    cap:1,
    clld:2,
    scs:1,     /* 12 */
    ccis:2,
    cm:2;      /* 16 */
};
#endif

union tbcctlu_t {
    struct tbcctl_t  b;
    uint16_t         s;
};

struct msp430_timerB_t  
{
  union {
    struct tbctl_t   b;
    uint16_t         s;
  } tbctl;                  /* 0x180 */
  
  union tbcctlu_t  tbcctl0; /* 0x182 */
  union tbcctlu_t  tbcctl1; /* 0x184 */
  union tbcctlu_t  tbcctl2; /* 0x186 */
#if defined(__msp430_have_timerb7)
  union tbcctlu_t  tbcctl3; /* 0x188 */
  union tbcctlu_t  tbcctl4; /* 0x18A */
  union tbcctlu_t  tbcctl5; /* 0x18C */
  union tbcctlu_t  tbcctl6; /* 0x18E */
#endif

  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int              tbr;     /* 0x190 */
  int              tbccr0;  /* 0x192 */
  int              tbccr1;  /* 0x194 */
  int              tbccr2;  /* 0x196 */
#if defined(__msp430_have_timerb7)
  int              tbccr3;  /* 0x198 */
  int              tbccr4;  /* 0x19a */
  int              tbccr5;  /* 0x19c */
  int              tbccr6;  /* 0x19e */
#endif

  int      tbcl0;
  int      tbcl1;
  int      tbcl2;
#if defined(__msp430_have_timerb7)
  int      tbcl3;
  int      tbcl4;
  int      tbcl5;
  int      tbcl6;
#endif

  int      b_tbcl0;  /* used in compare mode to detect */
  int      b_tbcl1;
  int      b_tbcl2;
#if defined(__msp430_have_timerb7)
  int      b_tbcl3;
  int      b_tbcl4;
  int      b_tbcl5;
  int      b_tbcl6;
#endif

  int      equ0;
  int      equ1;
  int      equ2;
#if defined(__msp430_have_timerb7)
  int      equ3;
  int      equ4;
  int      equ5;
  int      equ6;
#endif

  union {
    struct tiv_t     b; 
    uint16_t         s;
  } tiv;                    /* 0x12e */

  enum timer_ud_mode_t  udmode;
  int  tbr_limit;
};

void    msp430_timerB_reset (void);
void    msp430_timerB_update(void);
int16_t msp430_timerB_read  (uint16_t addr);
void    msp430_timerB_write (uint16_t addr, int16_t val);
int     msp430_timerB_chkifg(void);

#endif /* have_timerb */

/***************************************************/
/***************************************************/
/***************************************************/

#endif

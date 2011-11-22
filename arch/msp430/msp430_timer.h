
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

enum timer_outmod_t {
  TIMER_OUTMOD_OUTPUT       = 0,
  TIMER_OUTMOD_SET          = 1,
  TIMER_OUTMOD_TOGGLE_RESET = 2,
  TIMER_OUTMOD_SET_RESET    = 3,
  TIMER_OUTMOD_TOGGLE       = 4,
  TIMER_OUTMOD_RESET        = 5,
  TIMER_OUTMOD_TOGGLE_SET   = 6,
  TIMER_OUTMOD_RESET_SET    = 7
};

/***************************************************/
/** Timer A3 ***************************************/
/***************************************************/
#if defined(__msp430_have_timera3)

#if defined(__msp430_have_timera3)
#define TIMERANAME "timerA3"
#define TIMERA_COMPARATOR 3
#endif

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

struct msp430_timerA_t 
{
  union {
    struct tactl_t   b;
    uint16_t         s;
  } tactl;                 /* 0x160 */


  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int      tar;            /* 0x170 */
  
  union tacctlu_t  tacctl [TIMERA_COMPARATOR]; 
  int            b_taccr  [TIMERA_COMPARATOR];
  int              taccr  [TIMERA_COMPARATOR];        
  int              equ    [TIMERA_COMPARATOR];
  int              out    [TIMERA_COMPARATOR];

  union {
    struct tiv_t     b;   
    uint16_t         s;
  } tiv;                   /* 0x12e */

  enum timer_ud_mode_t  udmode;
};

void    msp430_timerA_create (void);
void    msp430_timerA_reset  (void);
void    msp430_timerA_update (void);
void    msp430_timerA_capture(void);
int16_t msp430_timerA_read   (uint16_t addr);
void    msp430_timerA_write  (uint16_t addr, int16_t val);
int8_t  msp430_timerA_read8  (uint16_t addr);
void    msp430_timerA_write8 (uint16_t addr, int8_t val);
int     msp430_timerA_chkifg ();

#else
#define msp430_timerA_create()  do { } while (0)
#define msp430_timerA_reset()   do { } while (0)
#define msp430_timerA_update()  do { } while (0)
#define msp430_timerA_capture() do { } while (0)
#define msp430_timerA_chkifg()  0
#endif

/***************************************************/
/** Timer B ****************************************/
/***************************************************/

#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)

#if defined(__msp430_have_timerb3)
#define TIMERBNAME "timerB3"
#define TIMERB_COMPARATOR 3
#elif defined(__msp430_have_timerb7)
#define TIMERBNAME "timerB7"
#define TIMERB_COMPARATOR 7
#endif

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
  
  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int              tbr;     /* 0x190 */
  union tbcctlu_t  tbcctl [TIMERB_COMPARATOR];
  int              tbcl   [TIMERB_COMPARATOR];
  int            b_tbcl   [TIMERB_COMPARATOR];
  int              tbccr  [TIMERB_COMPARATOR];
  int              equ    [TIMERB_COMPARATOR];
  int              out    [TIMERB_COMPARATOR];

  union {
    struct tiv_t     b; 
    uint16_t         s;
  } tiv;                    /* 0x12e */

  enum timer_ud_mode_t  udmode;
  int  tbr_limit;
};

void    msp430_timerB_create (void);
void    msp430_timerB_reset  (void);
void    msp430_timerB_update (void);
void    msp430_timerB_capture(void);
int16_t msp430_timerB_read   (uint16_t addr);
void    msp430_timerB_write  (uint16_t addr, int16_t val);
int     msp430_timerB_chkifg (void);

#else
#define msp430_timerB_create()  do { } while (0)
#define msp430_timerB_reset()   do { } while (0)
#define msp430_timerB_update()  do { } while (0)
#define msp430_timerB_capture() do { } while (0)
#define msp430_timerB_chkifg()  0
#endif /* have_timerb */

/***************************************************/
/***************************************************/
/***************************************************/


/***************************************************/
/** New Timer_A ************************************/
/***************************************************/

#if defined(__msp430_have_timerTA0)
#define TIMER_TA0_START  TIMER_TA0_BASE
#define TIMER_TA0_END    (TIMER_TA0_BASE + 0x2e)

enum timerTA0_addr_t {
  TA0CTL     = (TIMER_TA0_BASE + 0x00),
  TA0CCTL0   = (TIMER_TA0_BASE + 0x02),
  TA0CCTL1   = (TIMER_TA0_BASE + 0x04),
  TA0CCTL2   = (TIMER_TA0_BASE + 0x06),
  TA0CCTL3   = (TIMER_TA0_BASE + 0x08),
  TA0CCTL4   = (TIMER_TA0_BASE + 0x0a),

  TA0R       = (TIMER_TA0_BASE + 0x10),
  TA0CCR0    = (TIMER_TA0_BASE + 0x12),
  TA0CCR1    = (TIMER_TA0_BASE + 0x14),
  TA0CCR2    = (TIMER_TA0_BASE + 0x16),
  TA0CCR3    = (TIMER_TA0_BASE + 0x18),
  TA0CCR4    = (TIMER_TA0_BASE + 0x1a),

  TA0EX0     = (TIMER_TA0_BASE + 0x20),
  TA0IV      = (TIMER_TA0_BASE + 0x2e)
};

/**
 * Timer TA0 Data Structure
 */

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ta0ctl_t {
  uint16_t
    unused:6,
    tassel:2,
    id:2,
    mc:2,
    unused1:1,
    taclr:1,
    taie:1,
    taifg:0;
};
#else
struct __attribute__ ((packed)) ta0ctl_t {
  uint16_t
    taifg:1,
    taie:1,
    taclr:1,
    unused1:1,
    mc:2,
    id:2,
    tassel:2,
    unused:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute ((packed)) ta0r_t {
  uint16_t
    count:16;
};
#else
struct __attribute ((packed)) ta0r_t {
  uint16_t
    count:16;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct  __attribute__ ((packed)) ta0cctl_t {
  uint16_t
    cm:2,
    ccis:2,
    scs:1,
    scci:1,
    unused:1,
    cap:1,
    outmod:3,
    ccie:1,
    cci:1,
    out:1,
    cov:1,
    ccifg:1;
};
#else
struct  __attribute__ ((packed)) ta0cctl_t {
  uint16_t
    ccifg:1,
    cov:1,
    out:1,
    cci:1,
    ccie:1,
    outmod:3,
    cap:1,
    unused:1,
    scci:1,
    scs:1,
    ccis:2,
    cm:2;
};
#endif

union ta0cctln_t {
    struct ta0cctl_t  b;
    uint16_t         s;
};

#define TIMERTA0_COMPARATOR 5

struct msp430_timerTA0_t
{
  union {
    struct ta0ctl_t   b;
    uint16_t         s;
  } ta0ctl;


  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int      tar;            /* 0x170 */


  union ta0cctln_t  ta0cctl [TIMERTA0_COMPARATOR];
  int            b_ta0ccr  [TIMERTA0_COMPARATOR];
  int              ta0ccr  [TIMERTA0_COMPARATOR];
  int              equ    [TIMERTA0_COMPARATOR];
  int              out    [TIMERTA0_COMPARATOR];

  union {
    struct tiv_t     b;
    uint16_t         s;
  } tiv;                   /* 0x12e */

  enum timer_ud_mode_t  udmode;
};

void    msp430_timerTA0_create (void);
void    msp430_timerTA0_reset  (void);
void    msp430_timerTA0_update (void);
void    msp430_timerTA0_capture(void);
int16_t msp430_timerTA0_read   (uint16_t addr);
void    msp430_timerTA0_write  (uint16_t addr, int16_t val);
int8_t  msp430_timerTA0_read8  (uint16_t addr);
void    msp430_timerTA0_write8 (uint16_t addr, int8_t val);
int     msp430_timertA0_chkifg ();

#else
#define msp430_timerTA0_create()  do { } while (0)
#define msp430_timerTA0_reset()   do { } while (0)
#define msp430_timerTA0_update()  do { } while (0)
#define msp430_timerTA0_capture() do { } while (0)
#endif

/***************************************************/
/** TimerTA1 ***************************************/
/***************************************************/

#if defined(__msp430_have_timerTA1)
#define TIMER_TA1_START  TIMER_TA1_BASE
#define TIMER_TA1_END    (TIMER_TA1_BASE + 0x2e)

enum timerTA1_addr_t {
  TA1CTL     = (TIMER_TA1_BASE + 0x00),
  TA1CCTL0   = (TIMER_TA1_BASE + 0x02),
  TA1CCTL1   = (TIMER_TA1_BASE + 0x04),
  TA1CCTL2   = (TIMER_TA1_BASE + 0x06),
  TA1CCTL3   = (TIMER_TA1_BASE + 0x08),
  TA1CCTL4   = (TIMER_TA1_BASE + 0x0a),

  TA1R       = (TIMER_TA1_BASE + 0x10),
  TA1CCR0    = (TIMER_TA1_BASE + 0x12),
  TA1CCR1    = (TIMER_TA1_BASE + 0x14),
  TA1CCR2    = (TIMER_TA1_BASE + 0x16),
  TA1CCR3    = (TIMER_TA1_BASE + 0x18),
  TA1CCR4    = (TIMER_TA1_BASE + 0x1a),

  TA1EX0     = (TIMER_TA1_BASE + 0x20),
  TA1IV      = (TIMER_TA1_BASE + 0x2e)
};

/**
 * Timer TA1 Data Structure
 */

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ta1ctl_t {
  uint16_t
    unused:6,
    tassel:2,
    id:2,
    mc:2,
    unused1:1,
    taclr:1,
    taie:1,
    taifg:0;
};
#else
struct __attribute__ ((packed)) ta1ctl_t {
  uint16_t
    taifg:1,
    taie:1,
    taclr:1,
    unused1:1,
    mc:2,
    id:2,
    tassel:2,
    unused:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute ((packed)) ta1r_t {
  uint16_t
    count:16;
};
#else
struct __attribute ((packed)) ta1r_t {
  uint16_t
    count:16;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct  __attribute__ ((packed)) ta1cctl_t {
  uint16_t
    cm:2,
    ccis:2,
    scs:1,
    scci:1,
    unused:1,
    cap:1,
    outmod:3,
    ccie:1,
    cci:1,
    out:1,
    cov:1,
    ccifg:1;
};
#else
struct  __attribute__ ((packed)) ta1cctl_t {
  uint16_t
    ccifg:1,
    cov:1,
    out:1,
    cci:1,
    ccie:1,
    outmod:3,
    cap:1,
    unused:1,
    scci:1,
    scs:1,
    ccis:2,
    cm:2;
};
#endif

union ta1cctln_t {
    struct ta1cctl_t  b;
    uint16_t         s;
};

#define TIMERTA1_COMPARATOR 5

struct msp430_timerTA1_t
{
  union {
    struct ta1ctl_t   b;
    uint16_t         s;
  } ta1ctl;


  /* input clock is taken before div */
  /* tar is incremented by (divbuffer >> id) */
  unsigned int divbuffer;    /* counts input clocks           */
  unsigned int divuppermask; /* tar += divbuffer >> timer.id  */
  unsigned int divlowermask; /* divbuffer &= divlowermask     */

  /* we keep int for counters to detect overflow */
  int      tar;            /* 0x170 */


  union ta1cctln_t  ta1cctl [TIMERTA1_COMPARATOR];
  int            b_ta1ccr  [TIMERTA1_COMPARATOR];
  int              ta1ccr  [TIMERTA1_COMPARATOR];
  int              equ    [TIMERTA1_COMPARATOR];
  int              out    [TIMERTA1_COMPARATOR];

  union {
    struct tiv_t     b;
    uint16_t         s;
  } tiv;                   /* 0x12e */

  enum timer_ud_mode_t  udmode;
};

void    msp430_timerTA1_create (void);
void    msp430_timerTA1_reset  (void);
void    msp430_timerTA1_update (void);
void    msp430_timerTA1_capture(void);
int16_t msp430_timerTA1_read   (uint16_t addr);
void    msp430_timerTA1_write  (uint16_t addr, int16_t val);
int8_t  msp430_timerTA1_read8  (uint16_t addr);
void    msp430_timerTA1_write8 (uint16_t addr, int8_t val);
int     msp430_timertA0_chkifg ();

#else
#define msp430_timerTA1_create()  do { } while (0)
#define msp430_timerTA1_reset()   do { } while (0)
#define msp430_timerTA1_update()  do { } while (0)
#define msp430_timerTA1_capture() do { } while (0)
#endif

#endif // timerb

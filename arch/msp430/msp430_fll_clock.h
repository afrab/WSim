
/**
 *  \file   msp430_fll_clock.h
 *  \brief  MSP430x4xx FLL+ clock module 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_FLL_CLOCK_H
#define MSP430_FLL_CLOCK_H

#if defined(__msp430_have_fll_and_xt2)

#define FLL_START     0x50
#define FLL_END       0x54

#define FLL_SCFI0     0x50
#define FLL_SCFI1     0x51
#define FLL_SCFQCTL   0x52
#define FLL_CTL0      0x53
#define FLL_CTL1      0x54

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) fll_scfqctl_t {
  uint8_t
    scfq_m:1,
    N:7;
};
#else
struct __attribute__ ((packed)) fll_scfqctl_t {
  uint8_t
    N:7,
    scfq_m:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) fll_scfi0_t {
  uint8_t 
    flld:2,
    fn:4,
    mod:2;
};
#else
struct __attribute__ ((packed)) fll_scfi0_t {
  uint8_t 
    mod:2,
    fn:4,
    flld:2;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) fll_scfi1_t {
  uint8_t 
    dco:5,
    mod:3;
};
#else
struct __attribute__ ((packed)) fll_scfi1_t {
  uint8_t
    mod:3,
    dco:5;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) fll_ctl0_t {
  uint8_t 
    dcoplus:1,
    xts_fll:1,
    xcapxpf:2,
    xt2of:1,
    xt1of:1,
    lfof:1,
    dcof:1;
};
#else
struct __attribute__ ((packed)) fll_ctl0_t {
  uint8_t 
    dcof:1,
    lfof:1,
    xt1of:1,
    xt2of:1,
    xcapxpf:2,
    xts_fll:1,
    dcoplus:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) fll_ctl1_t {
  uint8_t 
    unused:1,
    smclkoff:1,
    xt2off:1,
    selm:2,
    sels:1,
    fll_div:2;
};
#else
struct __attribute__ ((packed)) fll_ctl1_t {
  uint8_t 
    fll_div:2,
    sels:1,
    selm:2,
    xt2off:1,
    smclkoff:1,
    unused:1;
};
#endif

/**
 * FLL Data Structure
 **/
struct msp430_fll_clock_t
{
  union {
    struct fll_scfi0_t   b;
    uint8_t              s;
  } scfi0;
  union {
    struct fll_scfi1_t   b;
    uint8_t              s;
  } scfi1;
  union {
    struct fll_scfqctl_t b;
    uint8_t              s;
  } scfqctl;
  union {
    struct fll_ctl0_t    b;
    uint8_t              s;
  } ctl0;
  union {
    struct fll_ctl1_t    b;
    uint8_t              s;
  } ctl1;

  /* external clocks */

  uint64_t MCLK_counter;
  int      MCLK_increment;

  uint64_t ACLK_counter;
  int      ACLK_increment;

  uint64_t ACLKn_counter;
  int      ACLKn_temp;
  int      ACLKn_increment;
  uint32_t ACLKn_bitmask;

  uint64_t SMCLK_counter;
  int      SMCLK_increment;

  /* internal clocks */

  uint32_t lfxt1_freq;
  uint32_t lfxt1_cycle_nanotime;
  uint64_t lfxt1_counter;
  int      lfxt1_temp;
  int      lfxt1_increment;

  uint32_t xt2_freq;
  uint32_t xt2_cycle_nanotime;
  uint64_t xt2_counter;
  int      xt2_temp;
  int      xt2_increment;

  uint32_t dcoclk_freq;
  uint32_t dcoclk_cycle_nanotime;
  uint64_t dcoclk_counter;
  int      dcoclk_temp;
  int      dcoclk_increment;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_fll_clock_create();
void    msp430_fll_clock_reset();

int     msp430_fll_clock_update(int clock_add);
void    msp430_fll_clock_update_done(); 

int8_t  msp430_fll_clock_read (uint16_t addr);
void    msp430_fll_clock_write(uint16_t addr, int8_t val);

#define MCU_CLOCK_SYSTEM_UPDATE(n)       msp430_fll_clock_update(n)
#define MCU_CLOCK_SYSTEM_UPDATE_DONE()   msp430_fll_clock_update_done()
#define MCU_CLOCK_SYSTEM_SPEED_TRACER()  do { } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#else
#define msp430_fll_clock_create() do { } while (0)
#define msp430_fll_clock_reset()  do { } while (0)
#endif
#endif /* _H_ */

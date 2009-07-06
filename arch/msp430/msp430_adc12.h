/**
 *  \file   msp430_adc12.h
 *  \brief  MSP430 ADC12 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_ADC12_H
#define MSP430_ADC12_H

#if defined(__msp430_have_adc12)

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12ctl0_t {
  uint16_t
    sht1x:4,
    sht0x:4,
    msc:1,
    ref2_5v:1,
    refon:1,
    adc12on:1,
    adc12ovie:1,
    adc12tovie:1,
    enc:1,
    adc12sc:1;
};
#else
struct __attribute__ ((packed)) adc12ctl0_t {
  uint16_t
    adc12sc:1,
    enc:1,
    adc12tovie:1,
    adc12ovie:1,
    adc12on:1,
    refon:1,
    ref2_5v:1,
    msc:1,
    sht0x:4,
    sht1x:4;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12ctl1_t {
  uint16_t
    cstartaddx:4,
    shsx:2,
    shp:1,
    issh:1,
    adc12divx:3,
    adc12sselx:2,
    conseqx:2,
    adc12busy:1;
};
#else
struct __attribute__ ((packed)) adc12ctl1_t {
  uint16_t
    adc12busy:1,
    conseqx:2,
    adc12sselx:2,
    adc12divx:3,
    issh:1,
    shp:1,
    shsx:2,
    cstartaddx:4;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12memx_t {
  uint16_t
    bzero:4,
    value:12;
};
#else
struct __attribute__ ((packed)) adc12memx_t {
  uint16_t
    value:12,
    bzero:4;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12mtclx_t {
  uint8_t
    eos:1,
    sref:3,
    inch:4;
};
#else
struct __attribute__ ((packed)) adc12mctlx_t {
  uint8_t
    inch:4,
    sref:3,
    eos:1;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12ie_t {
  uint16_t ie;
};
#else
struct __attribute__ ((packed)) adc12ie_t {
  uint16_t ie;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12ifg_t {
  uint16_t ifg;
};
#else
struct __attribute__ ((packed)) adc12ifg_t {
  uint16_t ifg;
};
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc12iv_t {
  uint16_t
    bzero0:10,
    adc12iv:5,
    bzero1:1;
};
#else
struct __attribute__ ((packed)) adc12iv_t {
  uint16_t
    bzero1:1,
    adc12iv:5,
    bzero0:10;
};
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum adc12_addr_t {
  ADC12CTL0   = 0x01A0, /* 16 */
  ADC12CTL1   = 0x01A2, /* 16 */
  ADC12IFG    = 0x01A4, /* 16 */
  ADC12IE     = 0x01A6, /* 16 */
  ADC12IV     = 0x01A8, /* 16 */
  
  ADC12MEM0   = 0x0140, /* 16 */
  ADC12MEM1   = 0x0142,
  ADC12MEM2   = 0x0144,
  ADC12MEM3   = 0x0146,
  ADC12MEM4   = 0x0148,
  ADC12MEM5   = 0x014A,
  ADC12MEM6   = 0x014C,
  ADC12MEM7   = 0x014E,
  ADC12MEM8   = 0x0150,
  ADC12MEM9   = 0x0152,
  ADC12MEM10  = 0x0154,
  ADC12MEM11  = 0x0156,
  ADC12MEM12  = 0x0158,
  ADC12MEM13  = 0x015A,
  ADC12MEM14  = 0x015C,
  ADC12MEM15  = 0x015E,
  
  ADC12MCTL0  = 0x080, /*  8 */
  ADC12MCTL1  = 0x081,
  ADC12MCTL2  = 0x082,
  ADC12MCTL3  = 0x083,
  ADC12MCTL4  = 0x084,
  ADC12MCTL5  = 0x085,
  ADC12MCTL6  = 0x086,
  ADC12MCTL7  = 0x087,
  ADC12MCTL8  = 0x088,
  ADC12MCTL9  = 0x089,
  ADC12MCTL10 = 0x08A,
  ADC12MCTL11 = 0x08B,
  ADC12MCTL12 = 0x08C,
  ADC12MCTL13 = 0x08D,
  ADC12MCTL14 = 0x08E,
  ADC12MCTL15 = 0x08F
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADC12_CHANNELS 16

struct msp430_adc12_t {
  union {
    struct adc12ctl0_t  b;
    uint16_t            s;
  } ctl0;
  union {
    struct adc12ctl1_t  b;
    uint16_t            s;
  } ctl1;
  uint16_t ifg;
  uint16_t ie;
  uint16_t iv;

  union {
    struct adc12memx_t  b;
    uint16_t            s;
  } mem[16];

  union {
    struct adc12mctlx_t b;
    uint8_t             s;
  } mctl[16];

  uint32_t chann_ptr[ADC12_CHANNELS];
  wsimtime_t   chann_time[ADC12_CHANNELS];
  wsimtime_t   chann_period[ADC12_CHANNELS];
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int     msp430_adc12_option_add (void);
int     msp430_adc12_init       (void);
void    msp430_adc12_reset      (void);
void    msp430_adc12_update     (void);
int16_t msp430_adc12_read16     (uint16_t addr);
void    msp430_adc12_write16    (uint16_t addr, int16_t val);
int8_t  msp430_adc12_read8      (uint16_t addr);
void    msp430_adc12_write8     (uint16_t addr, int8_t val);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_adc12 */
#endif

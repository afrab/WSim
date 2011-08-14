/**
 *  \file   msp430_adc10.h
 *  \brief  MSP430 ADC10 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_ADC10_H
#define MSP430_ADC10_H

#if defined(__msp430_have_adc10)

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc10ctl0_t {
  uint16_t
    srefx:3,
    ssetx:2,
    adc10sr:1,
    refout:1,
    refburst:1,
    msc:1,
    ref2_5v:1,
    refon:1,
    adc10on:1,
    adc10ie:1,
    adc10ifg:1,
    enc:1,
    adc10sc:1;
};
#else
struct __attribute__ ((packed)) adc10ctl0_t {
  uint16_t
    adc10sc:1,
    enc:1,
    adc10ifg:1,
    adc10ie:1,
    adc10on:1,
    refon:1,
    ref2_5v:1,
    msc:1,
    reburst:1,
    refout:1,
    adc10sr:1,
    shtx:2,
    srefx:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc10ctl1_t {
  uint16_t
    inchx:4,
    shsx:2,
    adc10df:1,
    issh:1,
    adc10divx:3,
    adc10sselx:2,
    conseqx:2,
    adc10busy:1;
};
#else
struct __attribute__ ((packed)) adc10ctl1_t {
  uint16_t
    adc10busy:1,
    conseqx:2,
    adc10sselx:2,
    adc10divx:3,
    issh:1,
    adc10df:1,
    shsx:2,
    inchx:4;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc10dtc0_t {
  uint8_t
    reserved:4,
    tb:1,
    ct:1,
    b1:1,
    fetch:1;
};
#else
struct __attribute__ ((packed)) adc10dtc0_t {
  uint8_t
    fetch:1,
    b1:1,
    ct:1,
    tb:1,
    reserved:4;
};
#endif

enum adc10ssel_t {
  ADC10_SSEL_ADC10OSC      = 0,
  ADC10_SSEL_ACLK          = 1,
  ADC10_SSEL_MCLK          = 2,
  ADC10_SSEL_SMCLK         = 3
};

enum adc10modes_t {
  ADC10_MODE_SINGLE        = 0,
  ADC10_MODE_SEQ_CHAN      = 1,
  ADC10_MODE_REPEAT_SINGLE = 2,
  ADC10_MODE_REPEAT_SEQ    = 3
};

enum adc10state_t {
  ADC10_STATE_OFF          = 0,
  ADC10_STATE_WAIT_ENABLE  = 1,
  ADC10_STATE_WAIT_TRIGGER = 2,
  ADC10_STATE_SAMPLE       = 3,
  ADC10_STATE_CONVERT      = 4,
  ADC10_STATE_STORE        = 5
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADC10_CHANNELS 16

struct msp430_adc10_t {
  union {
    struct adc10ctl0_t  b;
    uint16_t            s;
  } ctl0;
  union {
    struct adc10ctl1_t  b;
    uint16_t            s;
  } ctl1;

  uint8_t  ae0;
  uint8_t  ae1;
  uint16_t mem;
  uint16_t adc10mem;
  union {
    struct adc10dtc0_t  b;
    uint8_t             s;
  } dtc0;
  uint8_t  dtc1;
  uint16_t sa;


  uint32_t        chann_ptr[ADC10_CHANNELS];     /* current ptr in data */
  wsimtime_t     chann_time[ADC10_CHANNELS];
  wsimtime_t   chann_period[ADC10_CHANNELS];
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int     msp430_adc10_option_add (void);

void    msp430_adc10_create     (void);
void    msp430_adc10_reset      (void);
void    msp430_adc10_update     (void);

int16_t msp430_adc10_read16     (uint16_t addr);
void    msp430_adc10_write16    (uint16_t addr, int16_t val);
int8_t  msp430_adc10_read8      (uint16_t addr);
void    msp430_adc10_write8     (uint16_t addr, int8_t val);

#define msp430_adc10_chkifg()  0

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#else
#define msp430_adc10_option_add() do { } while(0)
#define msp430_adc10_create()     do { } while (0)
#define msp430_adc10_reset()      do { } while (0)
#define msp430_adc10_update()     do { } while (0)
#endif /* have_adc10 */
#endif

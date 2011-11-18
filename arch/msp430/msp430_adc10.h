/**
 *  \file   msp430_adc10.h
 *  \brief  MSP430 ADC10 controller
 *  \author Antoine Fraboulet & Julien Carpentier
 *  \date   2006, 2011
 **/

#ifndef MSP430_ADC10_H
#define MSP430_ADC10_H

#if defined(__msp430_have_adc10)

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) adc10ctl0_t {
  uint16_t
    srefx:3,
    adc10shtx:2,
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
    adc10shtx:2,
    srefx:3;
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
  
  struct adc_channels_t  channels;
  enum adcssel_t         ssel;
  enum adcmodes_t        modes;
  enum adcstate_t        state;
   
  uint32_t adc10osc_freq;
  uint64_t adc10osc_counter;
  int      adc10osc_increment;
  int      adc10osc_temp;
  uint32_t adc10osc_cycle_nanotime;
  
  uint64_t adc10clk_counter;
  int      adc10clk_increment;
  int      adc10clk_temp;

  int      sht_increment;
  int      sht_temp;

  int      sampcon;
  int      current_x;
  uint16_t sample;
  uint64_t adc10clk_reftime;
  
  
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


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

/**
 *  \file   msp430_adc.h
 *  \brief  MSP430 Adc10/12 common
 *  \author Julien Carpentier
 *  \date   2011
 **/

#ifndef MSP430_ADC_H
#define MSP430_ADC_H


#if defined(__msp430_have_adc12) || defined(__msp430_have_adc10)
#include "src/options.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADC_CHANNELS 16

#define ADC_NONE      0
#define ADC_CHANN_PTR 1
#define ADC_RND       2
#define ADC_WSNET     3

  enum adcssel_t {
    ADC_SSEL_ADCOSC        = 0,
    ADC_SSEL_ACLK          = 1,
    ADC_SSEL_MCLK          = 2,
    ADC_SSEL_SMCLK         = 3
  };

  enum adcmodes_t {
    ADC_MODE_SINGLE        = 0,
    ADC_MODE_SEQ_CHAN      = 1,
    ADC_MODE_REPEAT_SINGLE = 2,
    ADC_MODE_REPEAT_SEQ    = 3
  };

  enum adcstate_t {
    ADC_STATE_OFF          = 0,
    ADC_STATE_WAIT_ENABLE  = 1,
    ADC_STATE_WAIT_TRIGGER = 2,
    ADC_STATE_SAMPLE       = 3,
    ADC_STATE_CONVERT      = 4,
    ADC_STATE_STORE        = 5
  };




  struct adc_channels_t{
  
    int         channels_valid[ADC_CHANNELS];
    char        channels_name[ADC_CHANNELS][MAX_FILENAME];
    uint16_t*   channels_data[ADC_CHANNELS];
    uint32_t    channels_data_max[ADC_CHANNELS];
  
    uint32_t    chann_ptr[ADC_CHANNELS];     /* current ptr in data */
    wsimtime_t  chann_time[ADC_CHANNELS];
    wsimtime_t  chann_period[ADC_CHANNELS];
   
    
  };




/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int            msp430_adc_option_add (void);
int            msp430_adc_init(struct adc_channels_t* channels, int width, struct moption_t* opt);
uint16_t       msp430_adc_sample_input(struct adc_channels_t* channels, int hw_channel_x, int current_x);

int            msp430_adc_find_inputs(struct adc_channels_t* channels, struct moption_t* opt);
int            msp430_adc_read_inputs(struct adc_channels_t* channels);
int            msp430_adc_delete_inputs(struct adc_channels_t* channels);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else
#define msp430_adc_option_add() do { } while(0)

#endif /* have adc10-adc12 */
#endif
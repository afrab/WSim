/**
 *  \file   msp430_adc10.c
 *  \brief  MSP430 Adc10 controller
 *  \author Antoine Fraboulet & Julien Carpentier
 *  \date   2006, 2011
 **/

#include <stdlib.h>
#include <string.h>
#include "arch/common/hardware.h"
#include "msp430.h"
#include "src/options.h"

#if defined(__msp430_have_adc10)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DMSG_ADC10(x...) HW_DMSG_MCUDEV(x)

#define ADC10_DEBUG_LEVEL_2 0

#if ADC10_DEBUG_LEVEL_2 != 0
#define HW_DMSG_2_DBG(x...) HW_DMSG_ADC10(x)
#else
#define HW_DMSG_2_DBG(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if !defined(ADC10_BASE)
#define ADC10_BASE 0
#endif

enum adc10_addr_t {
  ADC10AE0   = ADC10_BASE + 0x004A, /*  8 */
  ADC10AE1   = ADC10_BASE + 0x004B, /*  8 */
  ADC10DTC0 = ADC10_BASE + 0x0048, /*  8 */
  ADC10DTC1 = ADC10_BASE + 0x0049, /*  8 */
  ADC10CTL0 = ADC10_BASE + 0x01B0, /* 16 */
  ADC10CTL1 = ADC10_BASE + 0x01B2, /* 16 */
  ADC10MEM  = ADC10_BASE + 0x01B4, /* 16 */
  ADC10SA   = ADC10_BASE + 0x01BC  /* 16 */
};

tracer_id_t MSP430_TRACER_ADC10STATE;
tracer_id_t MSP430_TRACER_ADC10INPUT[ADC10_CHANNELS];

#define ADC10_TRACER_STATE(v)   tracer_event_record(MSP430_TRACER_ADC10STATE, v)
#define ADC10_TRACER_INPUT(i,v) tracer_event_record(MSP430_TRACER_ADC10INPUT[i], v)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADC10_NONE      0
#define ADC10_CHANN_PTR 1
#define ADC10_RND       2
#define ADC10_WSNET     3

int         msp430_adc10_channels_valid[ADC10_CHANNELS];
char        msp430_adc10_channels_name[ADC10_CHANNELS][MAX_FILENAME];
uint16_t*   msp430_adc10_channels_data[ADC10_CHANNELS];
uint32_t    msp430_adc10_channels_data_max[ADC10_CHANNELS];

#define ADC10_CHANNEL_NAMES 20
char trace_names[ADC10_CHANNELS][ADC10_CHANNEL_NAMES] = {
  "adc10_input_00", "adc10_input_01", "adc10_input_02", "adc10_input_03",
  "adc10_input_04", "adc10_input_05", "adc10_input_06", "adc10_input_07",
  "adc10_input_08", "adc10_input_09", "adc10_input_10", "adc10_input_11",
  "adc10_input_12", "adc10_input_13", "adc10_input_14", "adc10_input_15"
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#define ADC10_MODES        4
#define ADC10_MODES_NAMES 40
char adc10_modes[ADC10_MODES][ADC10_MODES_NAMES] = {
  "Single Channel Single Conversion",  "Sequence of Channels",
  "Repeat Single Channel",  "Repeat Sequence of Channels"
};

#define ADC10_STATES        6
#define ADC10_STATES_NAMES 40
char adc10_states[ADC10_STATES][ADC10_STATES_NAMES] = {
  "Off", "Wait enable", "Wait trigger", "Sample", "Convert", "Store"
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ADC12 internal OSC is ~ 5MHz */
/* cycle_nanotime == 200        */
#define NANO                    (  1000*1000*1000)
#define ADC10OSC_FREQ           (     5*1000*1000)

#define ADC10OSC_CYCLE_NANOTIME (NANO / ADC10OSC_FREQ) 


static struct moption_t adc10_in_opt = {
  .longname    = "msp430_adc10",
  .type        = required_argument,
  .helpstring  = "msp430 adc10 input",
  .value       = NULL
};

int msp430_adc10_option_add (void)
{
  options_add( &adc10_in_opt );
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int msp430_adc10_init(void)
{
  int i;
  for(i=0; i<ADC10_CHANNELS; i++)
    {
      MSP430_TRACER_ADC10INPUT[i]       = 0;
      msp430_adc10_channels_valid[i]    = ADC10_NONE;
      msp430_adc10_channels_data[i]     = NULL;
      msp430_adc10_channels_data_max[i] = 0;
      strcpy(msp430_adc10_channels_name[i], "none");
      
      MCU.adc10.chann_ptr[i]    = 0;
      MCU.adc10.chann_time[i]   = 0;
      MCU.adc10.chann_period[i] = 0;
    }

  if (adc10_in_opt.isset)
    {
      //msp430_adc10_find_inputs();
      //msp430_adc10_read_inputs();
    }

  MSP430_TRACER_ADC10STATE = tracer_event_add_id(1, "adc10_state", "msp430");
  for(i=0; i<ADC10_CHANNELS; i++)
    {
      if (msp430_adc10_channels_valid[i] != ADC10_NONE)
	{
	  MSP430_TRACER_ADC10INPUT[i] = tracer_event_add_id(16, trace_names[i], "msp430");
	}
    }

  MCU.adc10.mem = 0;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_create()
{
  msp430_io_register_addr8 (ADC10AE0 ,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10AE1 ,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC0,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC1,msp430_adc10_read8 ,msp430_adc10_write8);

  msp430_io_register_addr16(ADC10CTL0,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10CTL1,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10MEM ,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10SA  ,msp430_adc10_read16,msp430_adc10_write16);

  msp430_adc10_init();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_reset()
{
  int i;
  /* set initial values */
  HW_DMSG_ADC10("msp430:adc10:reset()\n");
  MCU.adc10.ctl0.s = 0;             //clear IFG, IE, SREFx
  MCU.adc10.ctl1.s = 0;             //clear INCHx
  MCU.adc10.adc10osc_freq           = ADC10OSC_FREQ;
  MCU.adc10.adc10osc_cycle_nanotime = ADC10OSC_CYCLE_NANOTIME;
}

static inline void ADC10_SET_STATE(int state)
{
  int current_state = MCU.adc10.state;

  MCU.adc10.state = state;
  ADC10_TRACER_STATE(MCU.adc10.state);
  if (current_state != state)
    {
      HW_DMSG_ADC10("msp430:adc10: mode \"%s\", state \"%s\" -> \"%s\"\n", 
		    adc10_modes[MCU.adc10.ctl1.b.conseqx], 
		    adc10_states[current_state],
		    adc10_states[state]);
    }
  switch (state)
    {
    case ADC10_STATE_SAMPLE:
    case ADC10_STATE_CONVERT:
    case ADC10_STATE_STORE:
            MCU.adc10.ctl1.b.adc10busy = 1; /* operation done */
	    break;
    default:
            MCU.adc10.ctl1.b.adc10busy = 0; /* operation done */
	    break;
    }
}

static const int shtdiv[4] = 
{
  4, 8, 16, 64 
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_update()
{
    if (MCU.adc10.ctl0.b.adc10on == 0)
    {
      ADC10_SET_STATE( ADC10_STATE_OFF );
      return;
      
    }
    
    //TODO
    
    
    
    
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc10_read16 (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write16 (uint16_t addr, int16_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%04x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc10_read8  (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write8 (uint16_t addr, int8_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%02x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

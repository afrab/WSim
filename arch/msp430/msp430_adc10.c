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

/* ADC10 internal OSC is ~ 5MHz */
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

int msp430_adc_option_add (void)
{
  options_add( &adc10_in_opt );
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int msp430_adc10_init(void)
{

  msp430_adc_init(& MCU.adc10.channels, 10, &adc10_in_opt);

  MSP430_TRACER_ADC10STATE = tracer_event_add_id(1, "adc10_state", "msp430");
  int i;
  for(i=0; i<ADC10_CHANNELS; i++)
    {
      if (MCU.adc10.channels.channels_valid[i] != ADC_NONE)
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
    case ADC_STATE_SAMPLE:
    case ADC_STATE_CONVERT:
    case ADC_STATE_STORE:
            MCU.adc10.ctl1.b.adc10busy = 1; /* operation done */
	    break;
    default:
            MCU.adc10.ctl1.b.adc10busy = 0; /* operation done */
	    break;
	    
    }
    
}

#define CHECK_ENC()					\
  do {							\
    if (MCU.adc10.ctl0.b.enc == 0)			\
      {							\
	ADC10_SET_STATE( ADC_STATE_WAIT_ENABLE );	\
	return;						\
      }							\
  } while(0)

static const int shtdiv[4] = 
{
  4, 8, 16, 64 
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#define ENABLE 0
void msp430_adc10_update()
{
#if ENABLE

    if (MCU.adc10.ctl0.b.adc10on == 0)
    {
      ADC10_SET_STATE( ADC_STATE_OFF );
      return; 
    }
    
    //ADC10 Oscillator
    MCU.adc10.adc10osc_temp     += MACHINE_TIME_GET_INCR();
    MCU.adc10.adc10osc_increment = MCU.adc10.adc10osc_temp / MCU.adc10.adc10osc_cycle_nanotime;
    MCU.adc10.adc10osc_temp      = MCU.adc10.adc10osc_temp % MCU.adc10.adc10osc_cycle_nanotime;
    MCU.adc10.adc10osc_counter  += MCU.adc10.adc10osc_increment;

    //ADC10 Clock
    MCU.adc10.adc10clk_temp     += MCU.adc10.adc10osc_increment;
    MCU.adc10.adc10clk_increment = MCU.adc10.adc10clk_temp / (MCU.adc10.ctl1.b.adc10divx + 1);
    MCU.adc10.adc10clk_temp      = MCU.adc10.adc10clk_temp % (MCU.adc10.ctl1.b.adc10divx + 1);
    MCU.adc10.adc10clk_counter  += MCU.adc10.adc10clk_increment;
    
    //ADC10 Sample and hold time
    MCU.adc10.sht_temp         += MCU.adc10.adc10clk_increment;
    MCU.adc10.sht_increment     = MCU.adc10.sht_temp / (shtdiv[MCU.adc10.ctl0.b.adc10shtx]);
    MCU.adc10.sht_temp          = MCU.adc10.sht_temp % (shtdiv[MCU.adc10.ctl0.b.adc10shtx]);
    MCU.adc10.sampcon           = MCU.adc10.sht_increment;
	
    
    switch (MCU.adc10.state)
    {
      /***************/
      /* OFF         */
      /***************/
    case ADC_STATE_OFF: 
      ADC10_SET_STATE( ADC_STATE_WAIT_ENABLE );
      /* no break */

      /***************/
      /* ENABLE      */
      /***************/
    case ADC_STATE_WAIT_ENABLE:
      if (MCU.adc10.ctl0.b.enc)
	{
	  ADC10_SET_STATE( ADC_STATE_WAIT_TRIGGER ); 
	}
      if ((MCU.adc10.ctl0.b.enc) && 
	  (MCU.adc10.ctl0.b.adc10sc == 1) &&
	  (MCU.adc10.ctl1.b.shsx == 0))
	{
	  ADC10_SET_STATE( ADC_STATE_SAMPLE );
	}
      break;
      
      /***************/
      /* TRIGGER     */
      /***************/
    case ADC_STATE_WAIT_TRIGGER:
      CHECK_ENC();
      if (MCU.adc10.sampcon > 0)
	{
	  ADC10_SET_STATE( ADC_STATE_SAMPLE );
	  MCU.adc10.sampcon --;
	}
      else
	{
	  return;
	}
      /* no break */

      /***************/
      /* SAMPLE      */
      /***************/
    case ADC_STATE_SAMPLE:
      CHECK_ENC();
      if (MCU.adc10.sampcon > 0)
	{
	  /* 
	   * check port configuration. 
	   *   SEL = 1  // Selector  = 0:GPIO  1:peripheral
	   *   DIR = 0  // Direction = 0:input 1:output
	   */
	  MCU.adc10.sample = msp430_adc_sample_input(& MCU.adc10.channels,ADC10_CHANNELS, MCU.adc10.current_x);
	  ADC10_TRACER_INPUT( MCU.adc10.ctl1.b.inch, MCU.adc10.sample );

	  HW_DMSG_ADC10("msp430:adc10:     sampling on config %d hw_channel %d (%s) = 0x%04x [%"PRId64"]\n",
			MCU.adc10.current_x, 
			MCU.adc10.ctl1.b.inch,
			
			trace_names[MCU.adc10.ctl1.b.inch],
			MCU.adc10.sample,
			MACHINE_TIME_GET_NANO());
 
	  ADC10_SET_STATE( ADC_STATE_CONVERT );
	  MCU.adc10.adc10clk_reftime = MCU.adc10.adc10clk_counter;
	  MCU.adc10.sampcon --;
	  
	}
      else
	{
	  return;
	}
      break;

      /***************/
      /* CONVERT     */
      /***************/
    case ADC_STATE_CONVERT:
      CHECK_ENC();
      if (MCU.adc10.adc10clk_counter > (MCU.adc10.adc10clk_reftime + 12))
	{
	  HW_DMSG_ADC10("msp430:adc10:     convert = 0x%04x (%d) \n",MCU.adc10.sample,MCU.adc10.sample);
	  ADC10_SET_STATE( ADC_STATE_STORE );
	}
      else
	{ 
	  return;
	}
      /* no break */

#define SAMPLE MCU.adc10.sample
#define STATE  MCU.adc10.state
#define ENC    MCU.adc10.ctl0.b.enc
#define MSC    MCU.adc10.ctl0.b.msc
#define SHP    MCU.adc10.ctl1.b.shp
#define ADC10x MCU.adc10.current_x

      /***************/
      /* STORE       */
      /***************/
      
    case ADC10_STATE_STORE:
      
      //TODO 
      /*
      CHECK_ENC();
      HW_DMSG_ADC10("msp430:adc10:     ADC12MEM%d = 0x%04x\n", ADC10x, SAMPLE);
      MCU.adc10.mem[ ADC10x ].s = SAMPLE;

      if ((MCU.adc10.ifg & (1 << ADC10x)) == 0)
	{
	  HW_DMSG_ADC10("msp430:adc10: set interrupt for channel %d\n", ADC10x);
	}
      MCU.adc10.ifg |= 1 << ADC10x;
      msp430_adc10_chkifg();

      switch (MCU.adc10.ctl1.b.conseqx)
	{
	case ADC10_MODE_SINGLE:
	  ENC   = 0;
	  ADC10_SET_STATE( ADC10_STATE_WAIT_ENABLE );
	  break;
	*/
	case ADC10_MODE_SEQ_CHAN:
	//TODO 
	/*
	  if (MCU.adc12.mctl[ ADC12x ].b.eos == 1)
	    {
	      ENC   = 0;
	      ADC12_SET_STATE( ADC12_STATE_WAIT_ENABLE );
	    }
	  else
	    {
	      ADC12x = (ADC12x + 1) % ADC12_CHANNELS;
	      if (MSC && SHP)
		{
		  ADC12_SET_STATE( ADC12_STATE_SAMPLE );
		}
	      else
		{
		  ADC12_SET_STATE( ADC12_STATE_WAIT_TRIGGER );
		}
	    }
	  break;
	*/
	case ADC10_MODE_REPEAT_SINGLE:
	//TODO 
	/*
	  if (ENC == 0)
	    {
	      ADC12_SET_STATE( ADC12_STATE_WAIT_ENABLE );
	    }
	  else
	    {
	      if (MSC && SHP)
		{
		  ADC12_SET_STATE( ADC12_STATE_SAMPLE );
		}
	      else
		{
		  ADC12_SET_STATE( ADC12_STATE_WAIT_TRIGGER );
		}
	    }
	  break;
	  */
	case ADC10_MODE_REPEAT_SEQ:
	 //TODO  
	  /*
	  if ((ENC == 0) && (MCU.adc12.mctl[ ADC12x ].b.eos == 1))
	    {
	      ADC12_SET_STATE( ADC12_STATE_WAIT_ENABLE );
	    }
	  else
	    {
	      ADC12x = (ADC12x + 1) % ADC12_CHANNELS;
	      if (MSC && SHP)
		{
		  ADC12_SET_STATE( ADC12_STATE_SAMPLE );
		}
	      else
		{
		  ADC12_SET_STATE( ADC12_STATE_WAIT_TRIGGER );
		}
	    }
	  break;
	  */
	}
      break;
    
      
    }
#endif
    
    
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc10_read16 (uint16_t addr)
{
  //TODO
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write16 (uint16_t addr, int16_t val)
{
  //TODO
  ERROR("msp430:adc10: write [0x%04x] = 0x%04x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc10_read8  (uint16_t addr)
{
  //TODO
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write8 (uint16_t addr, int8_t val)
{
  //TODO
  ERROR("msp430:adc10: write [0x%04x] = 0x%02x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

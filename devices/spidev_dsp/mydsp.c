
/**
 *  \file   mydsp.c
 *  \brief  DSP device example
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/spidev_dsp/spidev_dsp_dev.h"
#include "src/options.h"
#include "mydsp.h"


/***************************************************/
/***************************************************/
/***************************************************/

#ifdef DEBUG
#define MSG_DEVICES       2
#define DEBUG_ME_HARDER
#define HW_DDSP(x...) VERBOSE(MSG_DEVICES,x)
#else
#define HW_DDSP(x...) do {} while(0)
#endif

#define microsecond (1000)
#define millisecond (1000*1000)
#define second      (1000*1000*1000)

#define OUTPUT_BYTES     4

/***************************************************/
/***************************************************/
/***************************************************/

#define START_DELTA (100*microsecond)

#define PERIOD0_DELTA    (5050*microsecond)
#define PERIOD0_DURATION (40*microsecond)

#define PERIOD1_DELTA    (157*millisecond)
#define PERIOD1_DURATION (139*microsecond)

#define PERIOD2_DELTA    (2*second)
#define PERIOD2_DURATION (41300*microsecond)

#define ETRACER_DSP_POWER_STANDBY 1
#define ETRACER_DSP_POWER_ACTIVE  2

struct ev_t {
  wsimtime_t start;
  wsimtime_t end;
  wsimtime_t period;
  wsimtime_t duration;
  tracer_id_t tr_id;
};

struct ev_t events[] = {
  { .period = PERIOD0_DELTA, .duration = PERIOD0_DURATION },
  { .period = PERIOD1_DELTA, .duration = PERIOD1_DURATION },
  { .period = PERIOD2_DELTA, .duration = PERIOD2_DURATION }
};

#define NEVENTS (sizeof(events)/sizeof(struct ev_t))

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_init_fsm(struct dsp_internal_state_t *st);

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_create(struct dsp_internal_state_t *st)
{
  unsigned int i;
  memset(st->dsp_data,0,DSP_MEM_SIZE);
  for(i=0; i<NEVENTS; i++)
    {
      char name[4];
      sprintf(name,"e%d",i);
      events[i].tr_id = tracer_event_add_id(8, name , "mydsp");
    }

#if defined(ETRACE)
  st->etrace_id  = ETRACER_PER_ID_GUEST;
#endif
  HW_DDSP("mydsp:create\n");
}

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_reset(struct dsp_internal_state_t *st)
{
  st->dsp_mode      = MYDSP_PASSIVE;
  st->dsp_index     = 0;
  st->dsp_index_max = 0;
  st->dsp_output_n  = 0;
  st->etrace_state  = 0;
  etracer_slot_event(st->etrace_id,
		     ETRACER_PER_EVT_MODE_CHANGED,
		     ETRACER_DSP_POWER_STANDBY, 0);
  HW_DDSP("mydsp:reset\n");
}

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_delete(struct dsp_internal_state_t UNUSED *st)
{
  HW_DDSP("mydsp:delete\n");
}

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_mode(struct dsp_internal_state_t *st, int mode)
{
  if (st->dsp_mode != mode)
    {
      switch (mode)
	{
	case MYDSP_PASSIVE:
	  st->dsp_index = 0;
	  HW_DDSP("mydsp:mode switch to PASSIVE\n");
	  break;
	case MYDSP_ACTIVE:
	  st->dsp_index_max = st->dsp_index;
	  st->dsp_index     = 0;
	  HW_DDSP("mydsp:mode switch to ACTIVE max = %d\n",st->dsp_index_max);
	  /* */
	  mydsp_init_fsm(st);
	  break;
	default:
	  HW_DDSP("mydsp:mode %d unknown\n",mode);
	  break;
	}
      st->dsp_mode = mode;
    }
  else
    {
      HW_DDSP("mydsp:mode %d already set\n",mode);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_write(struct dsp_internal_state_t *st, uint32_t val)
{
  if (st->dsp_mode == MYDSP_PASSIVE)
    {
      st->dsp_data[st->dsp_index] = val & 0xff;
      st->dsp_index = (st->dsp_index + 1) % DSP_MEM_SIZE;
      HW_DDSP("mydsp:write 0x%08x index %d\n",val, st->dsp_index);
    }
  else
    {
      HW_DDSP("mydsp:write 0x%08x while in ACTIVE mode\n",val);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_init_fsm(struct dsp_internal_state_t *st)
{
  unsigned int i;
  st->fsm_time_start         = MACHINE_TIME_GET_NANO() + START_DELTA;
  st->dsp_output_n           = 0;
  for(i=0; i<NEVENTS; i++)
    {
      events[i].start = st->fsm_time_start + events[i].period;
      events[i].end   = st->fsm_time_start + events[i].period + events[i].duration;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void dsp_set_etrace_active(struct dsp_internal_state_t *st)
{
  if (st->etrace_state == 0)
    {
      etracer_slot_event(st->etrace_id,
			 ETRACER_PER_EVT_MODE_CHANGED,
			 ETRACER_DSP_POWER_ACTIVE, 0);
    }
  st->etrace_state ++;
}

void dsp_set_etrace_standby(struct dsp_internal_state_t *st)
{
  if (st->etrace_state == 0)
    {
      ERROR("mydsp: ==================================================\n");
      ERROR("mydsp: incorrect state while going STANDBY\n");
      ERROR("mydsp: ==================================================\n");
    }

  st->etrace_state --;
  if (st->etrace_state == 0)
    {
      etracer_slot_event(st->etrace_id,
			 ETRACER_PER_EVT_MODE_CHANGED,
			 ETRACER_DSP_POWER_STANDBY, 0);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int  mydsp_update(struct dsp_internal_state_t *st, uint32_t *val, uint8_t tx_pending)
{
  int ret = 0;
  unsigned int i;
  wsimtime_t current_time;

  current_time = MACHINE_TIME_GET_NANO();

  if ((st->dsp_mode == MYDSP_ACTIVE) && (current_time >= st->fsm_time_start))
    {
      
      for(i=0; i<NEVENTS; i++)
	{
	  /* *** */
	  if (current_time >= events[i].start)
	    {
	      events[i].start += events[i].period;
	      tracer_event_record(events[i].tr_id,1);
	      switch (i)
		{
		case 0:
		  break;
		case 1:
		  dsp_set_etrace_active(st);
		  break;
		case 2:
		  dsp_set_etrace_active(st);
		  break;
		}
	    }

	  if (current_time >= events[i].end)
	    {
	      events[i].end   += events[i].period;
	      tracer_event_record(events[i].tr_id,0);
	      switch (i)
		{
		case 0:
		  break;
		case 1:
		  dsp_set_etrace_standby(st);
		  break;
		case 2:
		  dsp_set_etrace_standby(st);
		  st->dsp_output_n = OUTPUT_BYTES;
		  break;
		}
	    }

      /* *** */

	}

      
      /* *** */
      if ((tx_pending == 0) && (st->dsp_output_n > 0))
	{
	  *val             = st->dsp_data[st->dsp_index]; 
	  st->dsp_index    = (st->dsp_index + 1) % st->dsp_index_max;
	  st->dsp_output_n = st->dsp_output_n - 1;
	  HW_DDSP("mydsp:read 0x%08x n=%d\n",*val,st->dsp_output_n);
	  ret = 1;
	}
    }
  return ret;
}

/***************************************************/
/***************************************************/
/***************************************************/


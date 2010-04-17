
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

/***************************************************/
/***************************************************/
/***************************************************/

void mydsp_create(struct dsp_internal_state_t *st)
{
  memset(st->dsp_data,0,DSP_MEM_SIZE);
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

int  mydsp_update(struct dsp_internal_state_t *st, uint32_t *val, uint8_t tx_pending)
{
  // HW_DDSP("mydsp:write 0x%08x\n",val);
  if (st->dsp_mode == MYDSP_ACTIVE)
    {
      if (tx_pending == 0)
	{
	  *val = st->dsp_data[st->dsp_index]; 
	  HW_DDSP("mydsp:read 0x%08x\n",*val);
	  st->dsp_index = (st->dsp_index + 1) % st->dsp_index_max;
	  return 1;
	}
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/


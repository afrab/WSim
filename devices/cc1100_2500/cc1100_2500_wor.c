
/**
 *  \file   cc1100_2500_wor.c
 *  \brief  CC1100/CC2500 wake on radio functions
 *  \author Loic Lemaitre
 *  \date   2010
 **/

/*
 *  cc1100_2500_wor.c
 *  
 *
 *  Created by Loic Lemaitre on 16/03/10.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#include "cc1100_2500_internals.h"


/***************************************************/
/***************************************************/
uint64_t cc1100_get_wor_min_sleep_period(void)
{
  /* see application note swra126b.pdf p3 */
  return ((uint64_t) 750 * 1000 * 384) / ((uint64_t) CC1100_XOSC_FREQ_MHz); /* in ns */
}


/***************************************************/
/***************************************************/
uint64_t cc1100_get_event0_period(struct _cc1100_t *cc1100)
{
  uint16_t event0 = (cc1100_read_register(cc1100, CC1100_REG_WOREVT1) << 8)
    | (cc1100_read_register(cc1100, CC1100_REG_WOREVT0));
  uint8_t wor_res = (cc1100_read_register(cc1100, CC1100_REG_WORCTRL)) & 0x03;

  /* tevent0 calculation : see p45 */
  uint64_t tevent0 = ((750 * 1000) / CC1100_XOSC_FREQ_MHz) * event0 * (1 << (5 * wor_res)); /* in ns */

  CC1100_DBG_WOR("cc1100:wor: event0 calculation: event0=0x%04x, wor_res=0x%02x, tevent0=%"PRIu64"ns\n",
		 event0, wor_res, tevent0);

  return tevent0;
}


/***************************************************/
/***************************************************/
uint64_t cc1100_get_event1_period(struct _cc1100_t *cc1100)
{
  /* WORCTRL register see p81 */
  int worctrl_event1[8] = {4,6,8,12,16,24,32,48};
  int id = ((cc1100_read_register(cc1100, CC1100_REG_WORCTRL)) & 0x70) >> 4;
  uint64_t tevent1 = (750 * 1000 * worctrl_event1[id]) / CC1100_XOSC_FREQ_MHz; /* in ns */
  
  CC1100_DBG_WOR("cc1100:wor: event1 calculation: worctrl_event1[%d]=%d, tevent1=%"PRIu64"ns\n",
		 id, worctrl_event1[id], tevent1);

  return tevent1;
}


/***************************************************/
/***************************************************/
uint64_t cc1100_get_rx_timeout_period(struct _cc1100_t *cc1100)
{
  /* RX_TIME of MCSM2 register, see p73 */
  float wor_duty_cycle[2][7] = {{12.5,  6.250,  3.125,  1.563,  0.781, 0.391, 0.195},
				{ 1.95, 0.9765, 0.4883, 0.2441, 0,     0,     0    }}; /* in per cent */

  float rx_timeout_constants[4][7] = {{ 3.6058,  1.8029,  0.9014, 0.4507, 0.2254, 0.1127, 0.0563},
				      {18.0288,  9.0144,  4.5072, 2.2536, 1.1268, 0.5634, 0.2817},
				      {32.4519, 16.2260,  8.1130, 4.0565, 2.0282, 1.0141, 0.5071},
				      {46.8750, 23.4375, 11.7188, 5.8594, 2.9297, 1.4648, 0.7324}}; /* no unit */
  uint8_t wor_res = (cc1100_read_register(cc1100, CC1100_REG_WORCTRL)) & 0x03;
  uint8_t rx_time = (cc1100_read_register(cc1100, CC1100_REG_MCSM2)) & 0x07;

  if (cc1100->wor)
    {
      /* wake on radio mode */
      if (wor_res > 1)
	{
	  CC1100_DBG_EXC("cc1100:wor: WORCTRL.WOR_RES should be < 1 when wor mode is enabled\n");
	}
      else if ((wor_res == 1 && rx_time == 4) ||
	       (wor_res == 1 && rx_time == 5) ||
	       (wor_res == 1 && rx_time == 6))
	{
	  CC1100_DBG_EXC("cc1100:wor: duty cycle value not known for RX_TIME=%d and WOR_RES=1\n", rx_time);
	}
      /* p73 */
      return (uint64_t) (cc1100_get_event0_period(cc1100) * (wor_duty_cycle[wor_res][rx_time] / 100));
    }
  else
    {
      /* ordinary rx mode */
      /* p73 */
      return (uint64_t) ((cc1100_get_event0_period(cc1100) * rx_timeout_constants[wor_res][rx_time] * 26)
		       / CC1100_XOSC_FREQ_MHz);
    }
}


/***************************************************/
/***************************************************/
uint64_t cc1100_get_wor_sleep_period(struct _cc1100_t *cc1100)
{
  uint64_t idle_to_rx_time = 0;
  uint64_t sleep_time;

  /* fs_wakeup timing */
  idle_to_rx_time += CC1100_FS_WAKEUP_DELAY_NS;

  /* calibration timing */
  if (((cc1100->registers[CC1100_REG_MCSM0] >> 4) & 0x03) == 0x01)
    {
      idle_to_rx_time += CC1100_CALIBRATE_DELAY_NS;
    }

  /* settling timing */
  idle_to_rx_time += CC1100_SETTLING_DELAY_NS;

  /* AN047 p6 */
  sleep_time = cc1100_get_event0_period(cc1100) - cc1100_get_event1_period(cc1100) 
    - idle_to_rx_time - cc1100_get_rx_timeout_period(cc1100);
  
  if (sleep_time < cc1100_get_wor_min_sleep_period())
    {
      CC1100_DBG_EXC("cc1100:wor: tsleep (%"PRIu64"ns) should not be < tsleepmin (%"PRIu64"ns)\n", 
		     sleep_time, cc1100_get_wor_min_sleep_period());
    }

  return sleep_time;
}

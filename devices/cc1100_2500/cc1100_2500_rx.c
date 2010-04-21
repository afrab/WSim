
/**
 *  \file   cc1100_2500_rx.c
 *  \brief  CC1100/CC2500 Rx methods
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2006 / 2007
 **/

/*
 *  cc1100_2500_rx.c
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified by Antoine Fraboulet, 2007
 */

#include <ctype.h>
#include "cc1100_2500_internals.h"

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_compare_byte(uint8_t sfd, uint8_t rx) 
{
  uint8_t diff = sfd ^ rx;
  uint8_t i, cnt = 0;
  
  if (!(diff & 0x01))
    {
      cnt++;
    }
  
  for (i = 0; i < 7; i++) 
    {
      diff = diff >> 1;
      if (!(diff & 0x01))
	{
	  cnt++;
	}
    }
  
  return cnt;
}


/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_rx_filter(struct _cc1100_t *cc1100, double frequency, int modulation, 
		     double dBm, double UNUSED snr, char UNUSED data) 
{
  double freq_cc;

  /* Verify cc1100 state */
  if (cc1100->fsm_state !=  CC1100_STATE_RX) 
    {
      CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], not in rx state (%s)\n",
		    machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.',
		    cc1100_state_to_str(cc1100->fsm_state));
      return -1;
    }

  /* Check calibration */
  if (CC1100_IS_CALIBRATED(cc1100) == 0)
    {
      CC1100_DBG_EXC("cc1100:rx:filter:node %d: RX while fs calibration not done\n",
		     machine_get_node_id());
      return -1;
    }

  /* Verify cc1100 signal strength */
  if (dBm < -90) 
    {
      CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], below sensibility\n",
		    machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.');
      return -1;
    }
	
  /* Verify cc1100 frequency */
#define FREQ_FILTER_THRESHOLD 1.0 /* MHz */

  freq_cc = cc1100_get_frequency_mhz(cc1100);
  if ((freq_cc - frequency > FREQ_FILTER_THRESHOLD) || 
      (frequency - freq_cc > FREQ_FILTER_THRESHOLD)) /* fabs */
    {
      CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], frequency mismatch (device:%lf,pkt:%lf)\n",
		    machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.', freq_cc, frequency);
      return -1;
    }
	
  /* Record rssi */
  CC1100_RECORD_RSSI(cc1100, dBm);
  
  /* Verify cc1100 modulation */
  if (cc1100_get_modulation(cc1100) != modulation) 
    {
      CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], modulation mismatch (dev:%d,pkt:%d)\n", 
		    machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.', cc1100_get_modulation(cc1100),modulation);
      return -1;
    }
	
  
  /*
   * Do not consider coliding rx, it is handled by worldsens in the SNR computation 
   *                         rx_io_timer
   *                             |
   *                   |------X-----X-------|
   *       MACHINE_TIME_GET_NANO()          MACHINE_TIME_GET_NANO()
   *                              
   */

  if (cc1100->rx_io_timer > 0) // timer is set
    {  
      if      (MACHINE_TIME_GET_NANO() <  /* too early */
	       (cc1100->rx_io_timer - CC1100_SYNCHRO_DELAY_THRESHOLD))
	{
	  CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], early synchro io_timer:%"PRIu64" > time:%"PRIu64" (diff=%"PRId64", dur=%"PRIu64")\n", 
			machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.',
			cc1100->rx_io_timer,  MACHINE_TIME_GET_NANO(), 
			cc1100->rx_io_timer - MACHINE_TIME_GET_NANO(), 
			cc1100_get_tx_byte_duration(cc1100));
	  return -1;
	}
      else if (MACHINE_TIME_GET_NANO() > /* too late */
	       (cc1100->rx_io_timer + CC1100_SYNCHRO_DELAY_THRESHOLD))
	{
	  CC1100_DBG_RX("cc1100:rx:filter:node %d: dropping received data [0x%02x,%c], late synchro io_timer:%"PRIu64" < time:%"PRIu64" (diff=%"PRId64", dur=%"PRIu64")\n", 
			machine_get_node_id(), data & 0xff, isprint((unsigned char)data) ? data:'.',
			cc1100->rx_io_timer,  MACHINE_TIME_GET_NANO(), 
			MACHINE_TIME_GET_NANO() - cc1100->rx_io_timer,
			cc1100_get_tx_byte_duration(cc1100));
	  return -1;    
	}
    }	
	
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_rx_sfd(struct _cc1100_t *cc1100, uint8_t rx) {
	uint8_t sfd;
	static int quality = 0;
	
	if (cc1100->ioOffset == 0) {
		quality = 0;
		sfd = cc1100->registers[CC1100_REG_SYNC1];
	} else if (cc1100->ioOffset == 1) {
		sfd = cc1100->registers[CC1100_REG_SYNC0];
	} else if (cc1100->ioOffset == 2) {
		sfd = cc1100->registers[CC1100_REG_SYNC1];
	} else if (cc1100->ioOffset == 3) {
		sfd = cc1100->registers[CC1100_REG_SYNC0];
	} else {
		CC1100_DBG_EXC("cc1100: (rx EXCEPTION): should never be here\n");
		sfd = 0;
	}
	cc1100->ioOffset++;
	
	quality += cc1100_compare_byte(sfd, rx);
	
	if (cc1100->ioOffset == 2) {
		
		switch ((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) {
			case 1:
				if (quality >= 15)
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			case 2:
				if (quality >= 16)
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			case 5:
				if ((quality >= 15) && (CC1100_GET_CS(cc1100)))
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			case 6:
				if ((quality >= 16) && (CC1100_GET_CS(cc1100)))
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			default:
				break;
		}
		
	} else if (cc1100->ioOffset == 4) {
		
		switch ((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) {
			case 3:
				if (quality >= 30)
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			case 7:
				if ((quality >= 30) && (CC1100_GET_CS(cc1100)))
					CC1100_RX_EXPECT_DATA(cc1100);
				else	
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
				break;
			default:
				break;
		}
		
	}
	
	return;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_compute_cca(struct _cc1100_t *cc1100)
{
  if (cc1100->fsm_state != CC1100_STATE_RX)
    {
      cc1100->registers[CC1100_REG_PKTSTATUS] &= ~(0x10);
      return;
    }
  switch ((cc1100->registers[CC1100_REG_MCSM1] >> 4) && 0x03)
    {
    case 0:
      cc1100->registers[CC1100_REG_PKTSTATUS] |= (0x10);
      break;
    case 1:
      /* ToCheck: valeur de 0 */
      if (((int8_t) cc1100->registers[CC1100_REG_RSSI]) == 0) 
	{
	  cc1100->registers[CC1100_REG_PKTSTATUS] |= (0x10);
	} 
      else 
	{
	  cc1100->registers[CC1100_REG_PKTSTATUS] &= ~(0x10);
	}
      break;
    case 2:
      if (cc1100->fsm_ustate == 1) 
	{
	  cc1100->registers[CC1100_REG_PKTSTATUS] |= (0x10);
	} 
      else 
	{
	  cc1100->registers[CC1100_REG_PKTSTATUS] &= ~(0x10);
	}
      break;
    case 3:
      /* ToCheck: valeur de 0 */
      if ((cc1100->fsm_ustate == 1) && (((int8_t) cc1100->registers[CC1100_REG_RSSI]) == 0))
	{
	  cc1100->registers[CC1100_REG_PKTSTATUS] |= (0x10);
	} else {
	cc1100->registers[CC1100_REG_PKTSTATUS] &= ~(0x10);
      }
      break;
    }
  return;
} 

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_rx_state(struct _cc1100_t *cc1100) 
{
  /* cc1100 data sheet rev 1.1, page 68
   * MCSMx  Main Radio Control State Machine configuration

   * CC1100_REG_MCSM1 = Configuration 1
   * ===================================================================================

7:6 Reserved               R0
5:4 CCA_MODE[1:0]   3 (11) R/W Selects CCA_MODE; Reflected in CCA signal
3:2 RXOFF_MODE[1:0] 0 (00) R/W Select what should happen when a packet has been received
                                  Setting    Next state after finishing packet reception
                                  0 (00)     IDLE
                                  1 (01)     FSTXON
                                  2 (10)     TX
                                  3 (11)     Stay in RX
                               It is not possible to set RXOFF_MODE to be TX or FSTXON
                               and at the same time use CCA.
1:0 TXOFF_MODE[1:0] 0 (00) R/W Select what should happen when a packet has been sent (TX)

   * CC1100_REG_MCSM0 = Configuration 0
   * ===================================================================================

7:6 Reserved               R0
5:4 FS_AUTOCAL[1:0] 0 (00) R/W Automatically calibrate when going to RX or TX, or back to IDLE
                                  Setting   When to perform automatic calibration
                                  0 (00)    Never (manually calibrate using SCAL strobe)
                                  1 (01)    When going from IDLE to RX or TX (or FSTXON)
                                  2 (10)    When going from RX or TX back to IDLE
                                                    th
                                  3 (11)    Every 4 time when going from RX or TX to IDLE
                               In some automatic wake-on-radio (WOR) applications, using
                               setting 3 (11) can significantly reduce current consumption.
3:2 PO_TIMEOUT      1 (01) R/W Programs the number of times the six-bit ripple counter must
1   PIN_CTRL_EN     0      R/W Enables the pin radio control option
0   XOSC_FORCE_ON   0      R/W Force the XOSC to stay on in the SLEEP state.

   * ===================================================================================
   */

  int rx_off  = (cc1100->registers[CC1100_REG_MCSM1] & 0x0C) >> 2;
  int fs_auto = (cc1100->registers[CC1100_REG_MCSM0] & 0x30) >> 4;

  switch (rx_off)
    {
    case 0x00: /* Idle */ 
      switch (fs_auto)
	{
	case 0x00: /* never */
	  CC1100_RX_END_ENTER(cc1100);
	  break;
	case 0x01: /* idle -> Rx/Tx */
	  CC1100_RX_END_ENTER(cc1100);
	  break;
	case 0x02: /* Rx/Tx -> Idle */
	  CC1100_CALIBRATE_ENTER(cc1100);
	  break;
	case 0x03: /* Rx/Tx -> Idle / 4 :: NOTE / TODO = div 4 */
	  CC1100_CALIBRATE_ENTER(cc1100);
	  break;
	}
      break;
    case 0x01: /* fstxon */
      cc1100->fsm_pending = CC1100_STATE_FSTXON;
      CC1100_RXTX_SETTLING_ENTER(cc1100);
      break;
    case 0x02: /* Tx */
      cc1100->fsm_pending = CC1100_STATE_TX;
      CC1100_RXTX_SETTLING_ENTER(cc1100);
      break;
    case 0x03: /* stay in Rx */
      CC1100_RX_EXPECT_PREAMBLE(cc1100);
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_rx_data(struct _cc1100_t *cc1100, uint8_t rx,  double snr) {
	
	CC1100_COMPUTE_LQI(cc1100, snr);
	
	cc1100->ioCrc += rx;
	
	if (cc1100_put_rx_fifo(cc1100, rx))
		return;
	
	if (CC1100_FIXED_PKTLENGTH(cc1100)) {
		cc1100->ioLength = CC1100_PKTLENGTH(cc1100);
		
	} else if (CC1100_VAR_PKTLENGTH(cc1100) && (cc1100->ioOffset == 0)) {
		cc1100->ioLength =  rx + 1;
		if (CC1100_PKTLENGTH(cc1100) < (cc1100->ioLength - 1)) // ToCheck
			CC1100_DBG_EXC("cc1100: (rx EXCEPTION): variable packet length %d exceeds max registered packet length %d\n", 
						   (cc1100->ioLength - 1), cc1100->registers[CC1100_REG_PKTLEN]);
	} else  if (CC1100_INF_PKTLENGTH(cc1100)) {
		cc1100->ioLength = -1;
	}
	
	if (cc1100->ioLength != -1)
		cc1100->ioOffset++;

	if (cc1100->addressChk == 1) {
		switch (cc1100->registers[CC1100_REG_PKTCTRL1] & 0x03) {
			case 0:
				break;
			case 1:
				if ((rx != cc1100->registers[CC1100_REG_ADDR])) {
					cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
					return;
				}
				break;
			case 2:
				if ((rx != cc1100->registers[CC1100_REG_ADDR]) && (rx != 0x00)) {
					cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
					return;
				}
				break;
			case 3:
				if ((rx != cc1100->registers[CC1100_REG_ADDR]) && (rx != 0x00) && (rx != 0xFF)) {
					cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
					CC1100_RX_EXPECT_PREAMBLE(cc1100);
					return;
				}
				break;
		}
	}
	cc1100->addressChk++;
	
	if (cc1100->ioLength == cc1100->ioOffset) 
	  {
	    if (CC1100_COMPUTE_CRC(cc1100)) 
	      {
		CC1100_RX_EXPECT_CRC(cc1100);
	      } 
	    else 
	      {
		CC1100_RX_END(cc1100);
	      }
	  }
	
	return;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_rx_crc(struct _cc1100_t *cc1100, uint8_t rx) {
	static uint16_t crc = 0;
	
	if (cc1100->ioOffset == 0) {
		crc = rx;
		crc = (crc << 8) & 0xFF00;
		cc1100->ioOffset++;	
		return;
	} else {
		crc = crc | rx;
	}
	
	if (crc == cc1100->ioCrc) {
		CC1100_SET_CRC_TRUE(cc1100);
		CC1100_DBG_RX("cc1100:rx_crc: crc set true\n");
	} else {
		CC1100_SET_CRC_FALSE(cc1100);
	}
	
	if (CC1100_APPEND_CRC(cc1100)) {
		if (cc1100_put_rx_fifo(cc1100, cc1100->registers[CC1100_REG_RSSI]))
			return;
		if (cc1100_put_rx_fifo(cc1100, cc1100->registers[CC1100_REG_LQI]))
			return;
	}
	
	CC1100_RX_END(cc1100);	
	return;
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_rx_preamble(struct _cc1100_t *cc1100, uint8_t rx,  double UNUSED dBm, double snr) 
{
  if (cc1100->ioOffset == 0) 
    {
      switch ((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) 
	{
	case 0:
	  CC1100_RX_EXPECT_DATA(cc1100);
	  cc1100_rx_data(cc1100, rx, snr);
	  return;
	case 4:
	  if (CC1100_GET_CS(cc1100)) 
	    {
	      CC1100_RX_EXPECT_DATA(cc1100);
	      cc1100_rx_data(cc1100, rx, snr);
	      return;
	    }
	  return;
	default:
	  break;
	}	
    }
  
  if (rx != CC1100_PREAMBLE_PATTERN)
    {
      CC1100_DBG_RX("cc1100:rx_preamble: received a none preamble byte, reinit preamble phase\n"); 
      CC1100_RX_EXPECT_PREAMBLE(cc1100);
      return;
    }

  CC1100_UPDATE_PQT(cc1100, rx);
  cc1100->ioOffset++;
  
  if (cc1100->ioOffset == cc1100_get_preamble_length(cc1100)) 
    {
      if (CC1100_GET_PQT(cc1100)) 
	{
	  CC1100_RX_EXPECT_SFD(cc1100);
	} 
      else 
	{
	  CC1100_RX_EXPECT_PREAMBLE(cc1100);			
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

uint64_t cc1100_callback_rx(void* arg, struct wsnet_rx_info *rx)
{
  uint64_t duration;
  struct _cc1100_t *cc1100 = (struct _cc1100_t *) arg;
	
  if (cc1100_rx_filter(cc1100, rx->freq_mhz, rx->modulation, rx->power_dbm, rx->SiNR, rx->data) == -1)
    {
      /* data not taken into account */
      return 0;
    }

  /* everything is ok, synchronize for next byte */
  duration = cc1100_get_tx_byte_duration(cc1100);
  cc1100->rx_io_timer = MACHINE_TIME_GET_NANO() + duration;

  CC1100_DBG_RX("cc1100:rx:node %d: RX data [%02x,%c], freq: %fMHz, modulation: %d," \
		"Power: %lfdBm, time: %"PRId64"\n", 
		machine_get_node_id(), rx->data, 
		isprint(rx->data) ? rx->data : '.',
		rx->freq_mhz, rx->modulation, rx->power_dbm, 
		MACHINE_TIME_GET_NANO());

  switch (cc1100->fsm_ustate) 
    {
    case 1: /* SEND_PREAMBLE */
      cc1100_rx_preamble(cc1100, rx->data, rx->power_dbm, rx->SiNR);
      break;
    case 2: /* SEND_SFD */
      cc1100_rx_sfd(cc1100, rx->data);
      break;
    case 3: /* SEND_DATA */
      cc1100_rx_data(cc1100, rx->data, rx->SiNR);
      break;
    case 4: /* SEND_CRC */
      cc1100_rx_crc(cc1100, rx->data);
      break;
    case 5: /* END */
      break;
    default:
      ERROR("cc1100:rx:exception: callback_rx, invalid cc1100 internal state\n");
      break;
    }
  
  return duration;
}

/***************************************************/
/***************************************************/
/***************************************************/

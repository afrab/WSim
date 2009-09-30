
/**
 *  \file   cc1100_2500_tx.c
 *  \brief  CC1100/CC2500 Tx methods
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2006 / 2007
 **/

/*
 *  cc1100_2500_tx.c
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified by Antoine Fraboulet, 2007
 */
#include <math.h>
#include "cc1100_2500_internals.h"

/***************************************************/
/***************************************************/
/***************************************************/

double cc1100_get_frequency_mhz(struct _cc1100_t *cc1100) 
{
  int freq;
  int chan;     
  int chan_spcM;
  int chan_spcE;

  double Factor_Hz;
  double Freq_Hz;
  double Freq_MHz;

  /* page 49 */
  freq =
    ((cc1100->registers[CC1100_REG_FREQ2] << 16) & 0x00FF0000) |
    ((cc1100->registers[CC1100_REG_FREQ1] <<  8) & 0x0000FF00) |
    ((cc1100->registers[CC1100_REG_FREQ0])       & 0x000000FF);

  /* page 52 */
  chan_spcM = cc1100->registers[CC1100_REG_MDMCFG0];
  chan_spcE = cc1100->registers[CC1100_REG_MDMCFG1] & 0x03;

  /* page 48 */
  chan      = cc1100->registers[CC1100_REG_CHANNR];

  /* page 35 */

  Factor_Hz = ((double)CC1100_XOSC_FREQ_MHz * (double)1000000.0) / ((double)(1 << 16)) ;
  Freq_Hz   = Factor_Hz * (double)(freq + chan * ((256 + chan_spcM) * (1 << (chan_spcE - 1))));
  Freq_MHz  = Freq_Hz /  1000000.0;

  return Freq_MHz;
}


/***************************************************/
/***************************************************/
/***************************************************/

double cc1100_get_power_dbm(struct _cc1100_t UNUSED *cc1100, double freq)
{
  uint8_t pa_entry  = cc1100->registers[CC1100_REG_FREND0] & 0x07;
  uint8_t pa_value  = cc1100->patable[pa_entry];
  double dbm;

#if defined(CC2500)
  switch (pa_value) {
  case 0x00 : dbm = -50; break;
  case 0x50 : dbm = -30; break;
  case 0x44 : dbm = -28; break;
  case 0xC0 : dbm = -26; break;
  case 0x84 : dbm = -24; break;
  case 0x81 : dbm = -22; break;
  case 0x46 : dbm = -20; break;
  case 0x93 : dbm = -18; break;
  case 0x55 : dbm = -16; break;
  case 0x8D : dbm = -14; break;
  case 0xC6 : dbm = -12; break;
  case 0x97 : dbm = -10; break;
  case 0x6E : dbm = - 8; break;
  case 0x7F : dbm = - 6; break;
  case 0xA9 : dbm = - 4; break;
  case 0xBB : dbm = - 2; break;
  case 0xFE : dbm =   0; break;
  case 0xFF : dbm =   1; break;
    
  default   : dbm =   0;
  }


#elif defined(CC1100)
  double approx = 1;  /* MHz */
  if ((freq > 315.0 - approx) && (freq < 315.0 + approx)) {
      freq = 315.0;
  }
  if ((freq > 433.0 - approx) && (freq < 433.0 + approx)) {
      freq = 433.0;
  }
  if ((freq > 868.0 - approx) && (freq < 868.0 + approx)) {
      freq = 868.0;
  }
  if ((freq > 915.0 - approx) && (freq < 915.0 + approx)) {
      freq = 915.0;
  }

  switch((long int)freq) {

  case 315 :
      switch (pa_value) {
      case 0x04 : dbm = -30; break;
      case 0x17 : dbm = -20; break;
      case 0x1D : dbm = -15; break;
      case 0x26 : dbm = -10; break;
      case 0x57 : dbm = - 5; break;
      case 0x60 : dbm =   0; break;
      case 0x85 : dbm =   5; break;
      case 0xCB : dbm =   7; break;
      case 0xC6 : dbm = 8.7; break;
      case 0xC2 : dbm =  10; break;	
      default   : dbm =   0;
      };
      break;

  case 433 :
      switch (pa_value) {
      case 0x04 : dbm = -30; break;
      case 0x17 : dbm = -20; break;
      case 0x1C : dbm = -15; break;
      case 0x26 : dbm = -10; break;
      case 0x57 : dbm = - 5; break;
      case 0x60 : dbm =   0; break;
      case 0x85 : dbm =   5; break;
      case 0xC8 : dbm =   7; break;
      case 0xC6 : dbm = 8.7; break;
      case 0xC0 : dbm =  10; break;	
      default   : dbm =   0;
      };
      break;
      
  case 868 :
      switch (pa_value) {
      case 0x03 : dbm = -30; break;
      case 0x0D : dbm = -20; break;
      case 0x1C : dbm = -15; break;
      case 0x34 : dbm = -10; break;
      case 0x57 : dbm = - 5; break;
      case 0x8E : dbm =   0; break;
      case 0x85 : dbm =   5; break;
      case 0xCC : dbm =   7; break;
      case 0xC6 : dbm = 8.7; break;
      case 0xC3 : dbm =  10; break;
      default   : dbm =   0;
      };
      break;

  case 915 :
      switch (pa_value) {
      case 0x11 : dbm = -30; break;
      case 0x0D : dbm = -20; break;
      case 0x1C : dbm = -15; break;
      case 0x26 : dbm = -10; break;
      case 0x57 : dbm = - 5; break;
      case 0x8E : dbm =   0; break;
      case 0x83 : dbm =   5; break;
      case 0xC9 : dbm =   7; break;
      case 0xC6 : dbm = 8.7; break;
      case 0xC0 : dbm =  10; break;
      default   : dbm =   0;
      };
      break;

  default : dbm = 0;
  }
#else
#error "you must define CC1100 or CC2500 model"
#endif

  return dbm;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_get_modulation(struct _cc1100_t *cc1100) 
{
  /* page 51 */
  uint8_t conf;
  conf = (cc1100->registers[CC1100_REG_MDMCFG2] >> 4) & 0x07;

  switch (conf) 
    {
    case 0: return WSNET_MODULATION_2FSK;
    case 1: return WSNET_MODULATION_GFSK;
    case 3: return WSNET_MODULATION_ASK_OOK;
    case 7: return WSNET_MODULATION_MSK;
    default:
      return WSNET_MODULATION_UNKNOWN;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

/*
 *  Data transfer duration estimation
 *
 *  byte -> symbol -> chip -> modulation
 *
 *  datarate is the transmission speed after modulation
 */
uint64_t cc1100_get_tx_byte_duration(struct _cc1100_t *cc1100) 
{
  uint64_t drate_e, drate_m;

  uint64_t sym_rate;
  uint64_t sym_duration;
  uint64_t bit_sym_factor;

  uint64_t bit_rate;
  uint64_t bit_duration;
  uint64_t byte_duration;

  /* modem duration p50 */
  drate_e = cc1100->registers[CC1100_REG_MDMCFG4] & 0x0f;
  drate_m = cc1100->registers[CC1100_REG_MDMCFG3] & 0xff;

  /* bps */
  sym_rate     = (256 + drate_m) * (1 << drate_e);
  sym_rate     = sym_rate * ((double)CC1100_XOSC_FREQ_MHz * (double)1000000.0);
  sym_rate     = sym_rate / (1 << 28);
  sym_duration = (1000 * 1000 * 1000) / sym_rate;

  /* symbol duration */
  /* cc1100 modulation format are 1:1 // bit->symbol */
  switch (cc1100_get_modulation(cc1100)) 
    {
    case WSNET_MODULATION_2FSK:    bit_sym_factor = 1; break;
    case WSNET_MODULATION_GFSK:    bit_sym_factor = 1; break;
    case WSNET_MODULATION_ASK_OOK: bit_sym_factor = 1; break;
    case WSNET_MODULATION_MSK:     bit_sym_factor = 1; break;
    default:                       bit_sym_factor = 1; break;			
    }

  /* bit  */
  bit_rate      = sym_rate     / bit_sym_factor ;
  bit_duration  = sym_duration / bit_sym_factor ;

  /* byte */
  byte_duration = bit_duration * 8;

  /* Manchester and FEC are not available at the same time (page 27) */

  if (cc1100->registers[CC1100_REG_MDMCFG2] & 0x08) /* Manchester encoding p51 */
    {
      byte_duration *= 2;
    }
  else if (cc1100->registers[CC1100_REG_MDMCFG1] & 0x80)   /* FEC Encoding p52 */
    {
      byte_duration *= 2;
    }

  /*
  CC1100_DBG_TX("cc1100: bit rate %dbps %lfbkpd %lfkBps\n",bit_rate,(float)bit_rate/1000.0,(float)bit_rate/8000.0);
  CC1100_DBG_TX("cc1100: bit duration %"PRId64"ns %lfms\n",bit_duration,(double)bit_duration/1000000.0);
  CC1100_DBG_TX("cc1100: byte duration %"PRId64"ns %lfms\n",byte_duration,(double)byte_duration/1000000.0);
  */
  return byte_duration;
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_get_preamble_length(struct _cc1100_t *cc1100) 
{
  /* page 52 */
  uint8_t conf;      /*  0  1  2  3  4   5   6   7  */
  uint8_t plength[] = {  2, 3, 4, 6, 8, 12, 16, 24  };
  conf = ((cc1100->registers[CC1100_REG_MDMCFG1] >> 4) & 0x07);
  return plength[conf];
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_tx_preamble (struct _cc1100_t *cc1100) 
{
  uint8_t data = 0x55;
	
  cc1100->ioOffset++;
	
  if ((cc1100->ioOffset >= cc1100_get_preamble_length(cc1100)) && (cc1100->txBytes > 0)) 
    {
      /* Preamble over and TXFIFO byte available, go to next */
      if ((((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) == 0) || 
	  (((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) == 4)) 
	{
	  CC1100_TX_SEND_DATA(cc1100);	 // no preamble (p51)
	} 
      else
	{
	  CC1100_TX_SEND_SFD(cc1100);
	}
    }
  
  return data;
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_tx_sfd (struct _cc1100_t *cc1100) 
{
  uint8_t data;

  if (cc1100->ioOffset == 0) 
    {
      data = cc1100->registers[CC1100_REG_SYNC1];
      cc1100->ioOffset++;
    } 
  else if (cc1100->ioOffset == 1) 
    {
      data = cc1100->registers[CC1100_REG_SYNC0];

      if ((((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) != 3) && 
	  (((cc1100->registers[CC1100_REG_MDMCFG2]) & 0x07) != 7)) 
	{
	  CC1100_TX_SEND_DATA(cc1100);
	} 
      else 
	{
	  cc1100->ioOffset++;
	}
    } 
  else if (cc1100->ioOffset == 2) 
    {
      data = cc1100->registers[CC1100_REG_SYNC1];
      cc1100->ioOffset++;
    } 
  else if (cc1100->ioOffset == 3) 
    {
      data = cc1100->registers[CC1100_REG_SYNC0];
      CC1100_TX_SEND_DATA(cc1100);
    } 
  else 
    {
      CC1100_DBG_EXC("cc1100: (tx EXCEPTION): should never be here\n");
      data = 0;
    }
  
  return data;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_tx_state(struct _cc1100_t *cc1100) 
{
  /* cc1100 data sheet rev 1.1, page 68
   * MCSMx  Main Radio Control State Machine configuration

   * CC1100_REG_MCSM1 = Configuration 1
   * ===================================================================================

7:6 Reserved               R0
5:4 CCA_MODE[1:0]   3 (11) R/W Selects CCA_MODE; Reflected in CCA signal
3:2 RXOFF_MODE[1:0] 0 (00) R/W Select what should happen when a packet has been received
1:0 TXOFF_MODE[1:0] 0 (00) R/W Select what should happen when a packet has been sent (TX)
                                  Setting    Next state after finishing packet transmission
                                  0 (00)     IDLE
                                  1 (01)     FSTXON
                                  2 (10)     Stay in TX (start sending preamble)
                                  3 (11)     RX

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

  int tx_off  = (cc1100->registers[CC1100_REG_MCSM1] & 0x03) ;
  int fs_auto = (cc1100->registers[CC1100_REG_MCSM0] & 0x30) >> 4;

  switch (tx_off)
    {
    case 0x00: /* Idle */
      switch (fs_auto)
	{
	case 0x00: /* never */
	  CC1100_TX_END_ENTER(cc1100);
	  break;
	case 0x01: /* idle -> Rx/Tx */
	  CC1100_TX_END_ENTER(cc1100);
	  break;
	case 0x02: /* Rx/Tx -> Idle */
	  CC1100_CALIBRATE_ENTER(cc1100);
	  break;
	case 0x03: /* Rx/Tx -> Idle / 4 :: NOTE / TODO = div 4 */
	  CC1100_CALIBRATE_ENTER(cc1100);
	  break;
	}
      break;
    case 0x01: /* Fstxon */
      CC1100_FSTXON_ENTER(cc1100);
      break;
    case 0x02: /* Stay in Tx */
      CC1100_TX_SEND_PREAMBLE(cc1100);
      break;
    case 0x03: /* Rx */
      CC1100_TXRX_SETTLING_ENTER(cc1100);
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_tx_data (struct _cc1100_t *cc1100) {  
	uint8_t data;
	
	if (CC1100_FIXED_PKTLENGTH(cc1100)) {

		/* Fixed packet length */
		cc1100->ioLength = CC1100_PKTLENGTH(cc1100);
		if (cc1100->ioLength < 1)
			CC1100_DBG_EXC("cc1100: (tx EXCEPTION): 0 packet length\n");

	} else if (CC1100_VAR_PKTLENGTH(cc1100) && (cc1100->ioOffset == 0)) {

		/* Variable packet length: first byte is packet length */
		cc1100->ioLength = cc1100->txfifo[cc1100->txOffset] + 1; 
		if (CC1100_PKTLENGTH(cc1100) < (cc1100->ioLength - 1)) // ToCheck
		  {
		    CC1100_DBG_EXC("cc1100: (tx EXCEPTION): variable packet length %d exceeds max registered packet length %d\n", 
				   (cc1100->ioLength - 1), cc1100->registers[CC1100_REG_PKTLEN]);
		  }
		if (cc1100->ioLength < 1)
		  {
		    CC1100_DBG_EXC("cc1100: (tx EXCEPTION): 0 packet length\n");
		  }

	} else if (CC1100_INF_PKTLENGTH(cc1100)) {
		cc1100->ioLength = -1;
	}
	
	/* Get data from FIFO */
	data = cc1100_get_tx_fifo(cc1100);
	
	if (cc1100->ioLength != -1) {		
		cc1100->ioOffset++;
	}
	cc1100->ioCrc += data;
		
	if (cc1100->ioLength == cc1100->ioOffset) {
		/* Packet over */
		if (CC1100_COMPUTE_CRC(cc1100)) {
			CC1100_TX_SEND_CRC(cc1100);
		} else {
			/* Change state according to registers */
			CC1100_TX_END(cc1100);
		}
	}
	
	return data;
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_tx_crc (struct _cc1100_t *cc1100) 
{ 
  uint8_t data;
  
  if (cc1100->ioOffset == 0) 
    {
      data = (uint8_t) ((cc1100->ioCrc >> 8) & 0x00FF);
      cc1100->ioOffset++;
    } 
  else 
    {
      data = (uint8_t) (cc1100->ioCrc & 0x00FF);
      CC1100_TX_END(cc1100);
    }
  return data;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_tx (struct _cc1100_t *cc1100) 
{
  char     data;
  uint64_t duration;

  if (CC1100_IS_CALIBRATED(cc1100) == 0)
    {
      CC1100_DBG_EXC("cc1100:tx: TX while fs calibration not done\n");
      return;
    }
	
  if (MACHINE_TIME_GET_NANO()  < cc1100->tx_io_timer) 
    {
      /* Byte being sent, wait for termination */
      return;
    }
	
  switch (cc1100->fsm_ustate) 
    {
    case 1:
      data = cc1100_tx_preamble(cc1100);
      break;
    case 2:
      data = cc1100_tx_sfd(cc1100);
      break;
    case 3:
      /* Check for underflow */
      if (cc1100->txBytes == 0) 
	{
	  /* Set underflow byte */
	  cc1100->txUnderflow = 1;
	  cc1100->fsm_state = CC1100_STATE_TX_UNDERFLOW;
	  cc1100_assert_gdo(cc1100, 0x05, CC1100_PIN_ASSERT);
	  cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
	  CC1100_DBG_STATE("cc1100:state: TX_UNDERFLOW\n");
	  tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_TX_UNDERFLOW);
	  etracer_slot_event(ETRACER_PER_ID_CC1100,ETRACER_PER_EVT_MODE_CHANGED,ETRACER_CC1100_STARTUP,0);
	  return;
	}
      data = cc1100_tx_data(cc1100);
      break;
    case 4:
      data = cc1100_tx_crc(cc1100);
      break;
    case 5:
      return;
    default:
      CC1100_DBG_EXC("cc1100: (tx EXCEPTION): should never be here\n");
      data = 0;
      break;
    }
	
  /* Compute byte duration */
  duration = cc1100_get_tx_byte_duration(cc1100);
  
  /* Send byte */
  {
    struct wsnet_tx_info tx;
    tx.data       = data;
    tx.freq_mhz   = cc1100_get_frequency_mhz(cc1100);
    tx.modulation = cc1100_get_modulation(cc1100);
    tx.power_dbm  = cc1100_get_power_dbm(cc1100, tx.freq_mhz);
    tx.duration   = duration;
    tx.radio_id   = cc1100->worldsens_radio_id;
    worldsens_c_tx(&tx);
    
    CC1100_DBG_TX("cc1100:tx:node %d: data %02x, freq: %lfMHz, modulation: %d, "
		  "Power: %lfdBm, time: %" PRId64 " + %" PRId64 " = %" PRId64 " \n", 
		  machine_get_node_id(),
		  tx.data & 0xff, 
		  tx.freq_mhz,
		  tx.modulation,
		  tx.power_dbm,
		  MACHINE_TIME_GET_NANO(), 
		  tx.duration,
		  MACHINE_TIME_GET_NANO()  + tx.duration);
  }
  
  /* Set next emission and reception time */
  cc1100->tx_io_timer = MACHINE_TIME_GET_NANO() +     duration;
  cc1100->rx_io_timer = MACHINE_TIME_GET_NANO() + 2 * duration;
	
  return;
}

/***************************************************/
/***************************************************/
/***************************************************/

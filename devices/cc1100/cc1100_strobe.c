
/**
 *  \file   cc1100_strobe.c
 *  \brief  CC1100 strobe commands
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_strobe.c
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#include "cc1100_internals.h"

/**
 * Implemented command strobes:
 *   - SNOP
 *   - SCAL
 *   - SRES
 *   - SPWD
 *   - SXOFF
 *   - SFTX
 *   - SFRX
 *   - STX
 **/

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_mancal(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:mancal: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:mancal: refusing (0x%x) commands while calibrating\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_sleep(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    default:
      CC1100_DBG_IMPL("cc1100:strobe:sleep: refusing (0x%x) commands while sleeping\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_xoff(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {	
    default:
      CC1100_DBG_IMPL("cc1100:strobe:xoff: refusing (0x%x) commands while xoff\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_tx_underflow(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:tx_underflow: SNOP\n");
      return;
    case CC1100_STROBE_SFTX:
      CC1100_DBG_STROBE("cc1100:strobe:tx_underflow: SFTX\n");
      cc1100_flush_tx_fifo(cc1100);
      CC1100_IDLE_ENTER(cc1100);
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:tx_underflow: refusing (0x%x) commands while in tx_underflow\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_rx_overflow(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {		
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:rx_overflow: SNOP\n");
      return;
    case CC1100_STROBE_SFRX:
      CC1100_DBG_STROBE("cc1100:strobe:rx_overflow: SFRX\n");
      cc1100_flush_rx_fifo(cc1100);
      CC1100_IDLE_ENTER(cc1100);
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:rx_underflow: refusing (0x%x) commands while in rx_overflow\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_idle(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SCAL:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SCAL\n");
      CC1100_MANCAL_ENTER(cc1100);
      break;
    case CC1100_STROBE_SRES:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SRES\n");
      cc1100_reset_internal(cc1100);
      CC1100_IDLE_ENTER(cc1100);
      return;
    case CC1100_STROBE_SPWD:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SPWD\n");
      CC1100_SLEEP_ENTER(cc1100);
      return;
    case CC1100_STROBE_SXOFF:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SXOFF\n");
      CC1100_XOFF_ENTER(cc1100);
      return;
    case CC1100_STROBE_SIDLE:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SIDLE\n");
      return;
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SNOP\n");
      return;
    case CC1100_STROBE_STX:
      CC1100_DBG_STROBE("cc1100:strobe:idle: STX\n");
      cc1100->fsm_pending = CC1100_STATE_TX;
      CC1100_FS_WAKEUP_ENTER(cc1100);
      return;
    case CC1100_STROBE_SRX:
      CC1100_DBG_STROBE("cc1100:strobe:idle: SRX\n");
      cc1100->fsm_pending = CC1100_STATE_RX;
      CC1100_FS_WAKEUP_ENTER(cc1100);
      return;
    case CC1100_STROBE_SFSTXON:
      CC1100_DBG_STROBE("cc1100:strobe:idle: STX\n");
      cc1100->fsm_pending = CC1100_STATE_FSTXON;
      CC1100_FS_WAKEUP_ENTER(cc1100);
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:idle: strobe (0x%x) invalid or not implemented yet\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_fstxon(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SRES:
      CC1100_DBG_STROBE("cc1100:strobe:fstxon: SRES\n");
      cc1100_reset_internal(cc1100);
      CC1100_IDLE_ENTER(cc1100);
      return;
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:fstxon: SNOP\n");
      return;
    case CC1100_STROBE_STX:
      CC1100_DBG_STROBE("cc1100:strobe:fstxon: STX\n");
      CC1100_TX_ENTER(cc1100);
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:fstxon: strobe (0x%x) invalid or not implemented\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_fs_wakeup(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:fs_wakeup: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:fs_wakeup: refusing (0x%x) commands while waking up\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_calibrate(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:calibrate: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:calibrate: refusing (0x%x) commands while calibrating\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_fs_calibrate(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:fs_calibrate: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:fs_calibrate: refusing (0x%x) commands while fs_calibrate\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_settling(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:settling: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:settling: refusing (0x%x) commands while settling\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_txrx_settling(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {		
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:txrx_settling: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:txrx_settling: refusing (0x%x) commands while txrx_settling\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_rxtx_settling(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:rxtx_settling: SNOP\n");
      return;
    default:
      CC1100_DBG_IMPL("cc1100:strobe:rxtx_settling: refusing (0x%x) commands while rxtx_settling\n", cc1100->addr);
      return;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_state_rx(struct _cc1100_t *cc1100) 
{
  switch (cc1100->addr) 
    {
    case CC1100_STROBE_SNOP:
      CC1100_DBG_STROBE("cc1100:strobe:rx: SNOP\n");
      break;

    case CC1100_STROBE_SIDLE:
      CC1100_DBG_STROBE("cc1100:strobe:rx: SIDLE -> END_FORCED\n");
      CC1100_RX_END_FORCED(cc1100);
      /*
       * page 72: looking for fs_autocal
       *
       * 0 (00) Never (manually calibrate using SCAL strobe)
       * 1 (01) When going from IDLE to RX or TX (or FSTXON)
       * 2 (10) When going from RX or TX back to IDLE
       * 3 (11) Every 4th time when going from RX or TX to IDLE
       */
      switch ((cc1100->registers[CC1100_REG_MCSM0] & 0x30))
	{
	case 0x00:
	case 0x10: /* Idle */
	  CC1100_RX_END_ENTER(cc1100);
	  break;
	case 0x20:
	case 0x30: /* Calibrating */
	  CC1100_CALIBRATE_ENTER(cc1100);
	  break;
	}
      break;

    case CC1100_STROBE_STX:
      CC1100_DBG_STROBE("cc1100:strobe:rx: STX\n");
      CC1100_COMPUTE_CCA(cc1100);
      cc1100->fsm_pending = CC1100_STATE_TX;
      if (cc1100->registers[CC1100_REG_PKTSTATUS] & 0x10) 
	{
	  CC1100_RXTX_SETTLING_ENTER(cc1100);
	} 
      break;

    case CC1100_STROBE_SFSTXON:
      CC1100_DBG_STROBE("cc1100:strobe:rx: SFSTXON\n");
      CC1100_COMPUTE_CCA(cc1100);
      cc1100->fsm_pending = CC1100_STATE_FSTXON;
      if (cc1100->registers[CC1100_REG_PKTSTATUS] & 0x10) 
	{
	  CC1100_RXTX_SETTLING_ENTER(cc1100);
	} 
      break;

    case CC1100_STROBE_SRX:
      CC1100_DBG_STROBE("cc1100:strobe:rx: SRX issued while in RX\n");
      break;

    default:
      CC1100_DBG_IMPL("cc1100:strobe:rx: strobe command %s not implemented in rx state (0x%x)\n", 
		      cc1100_strobe_to_str(cc1100->addr), cc1100->addr);
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/
void cc1100_strobe_state_tx(struct _cc1100_t *cc1100) {
	switch (cc1100->addr) {		
		case CC1100_STROBE_SNOP:
			CC1100_DBG_STROBE("cc1100:strobe:tx: SNOP\n");
			return;
		default:
			CC1100_DBG_IMPL("cc1100:strobe:tx: strobe command not implemented in tx state (0x%x)\n", 
					cc1100_strobe_to_str(cc1100->addr), cc1100->addr);
			return;
	}
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_strobe_command(struct _cc1100_t *cc1100) {
	switch (cc1100->fsm_state) {
		case CC1100_STATE_IDLE:
			cc1100_strobe_state_idle(cc1100);
			return;
		case CC1100_STATE_MANCAL:
			cc1100_strobe_state_mancal(cc1100);
			return;
		case CC1100_STATE_SLEEP:
			cc1100_strobe_state_sleep(cc1100);
			return;
		case CC1100_STATE_XOFF:
			cc1100_strobe_state_xoff(cc1100);
			return;
		case CC1100_STATE_TX_UNDERFLOW:
			cc1100_strobe_state_tx_underflow(cc1100);
			return;
		case CC1100_STATE_RX_OVERFLOW:
			cc1100_strobe_state_rx_overflow(cc1100);
			return;
		case CC1100_STATE_FS_WAKEUP:
			cc1100_strobe_state_fs_wakeup(cc1100);
			return;
		case CC1100_STATE_CALIBRATE:
			cc1100_strobe_state_calibrate(cc1100);
			return;
		case CC1100_STATE_FS_CALIBRATE:
			cc1100_strobe_state_fs_calibrate(cc1100);
			return;
		case CC1100_STATE_SETTLING:
			cc1100_strobe_state_settling(cc1100);
			return;
		case CC1100_STATE_TXRX_SETTLING:
			cc1100_strobe_state_txrx_settling(cc1100);
			return;
		case CC1100_STATE_RXTX_SETTLING:
			cc1100_strobe_state_rxtx_settling(cc1100);
			return;
		case CC1100_STATE_RX:
			cc1100_strobe_state_rx(cc1100);
			return;
		case CC1100_STATE_TX:
			cc1100_strobe_state_tx(cc1100);
			return;
		case CC1100_STATE_FSTXON:
			cc1100_strobe_state_fstxon(cc1100);
			return;
		default:
			CC1100_DBG_IMPL("cc1100:strobe: state %d not implemented yet\n", cc1100->fsm_state);
			return;
	}
}

/***************************************************/
/***************************************************/
/***************************************************/


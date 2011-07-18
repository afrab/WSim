
/**
 *  \file   cc2420_rx.c
 *  \brief  CC2420 Rx methods
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_rx.c
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *  Modified by Loic Lemaitre 2009
 *
 */

#include <string.h>
#include <math.h>
#include <ctype.h>

#include "cc2420.h"
#include "cc2420_ram.h"
#include "cc2420_registers.h"
#include "cc2420_fifo.h"
#include "cc2420_tx.h"
#include "cc2420_rx.h"
#include "cc2420_macros.h"
#include "cc2420_internals.h"
#include "cc2420_debug.h"
#include "cc2420_crc_ccitt.h"
#include "cc2420_mux.h"


/**
 * update rssi value
 */

void cc2420_record_rssi(struct _cc2420_t * cc2420, double dBm) {

    uint8_t cca_mode;
    uint8_t cca_threshold;
    uint8_t cca_hyst;
    int16_t rx_rssi_value = 0;
    int i;

    /* if dBm = -100 it means that cc2420 didn't  receive any value during sync time. Skip this value for rssi average calculation */
    if (dBm == -100) {
        cc2420->rx_rssi_value = dBm - CC2420_RSSI_OFFSET;
    }
    else {
        /* we replace the oldest value of rssi in the table by the new we got */
        cc2420->rx_rssi_samples[cc2420->rx_rssi_sample_index] = dBm - CC2420_RSSI_OFFSET;

        /* update samples table index */
        cc2420->rx_rssi_sample_index = (cc2420->rx_rssi_sample_index + 1) % 8;

        /* compute new rssi average value */
        for (i = 1 ; i <= cc2420->rx_rssi_values ; i++) {
            rx_rssi_value = (rx_rssi_value * (i-1) + cc2420->rx_rssi_samples[i-1]) / i;
        }
        cc2420->rx_rssi_value = (int8_t)rx_rssi_value;

	/* rssi average value for fcs is only conputed with the first 8 symbols received after SFD */
        if (cc2420->rx_rssi_values == 7) {
            cc2420->rx_rssi_value_for_fcs = cc2420->rx_rssi_value;
        }

        if (cc2420->rx_rssi_values < 8) {			      
	    cc2420->rx_rssi_values++;
        }			     
    }

    
    /* update RSSI value register */
    cc2420->registers[CC2420_REG_RSSI] = (cc2420->registers[CC2420_REG_RSSI] & 0xff00) | cc2420->rx_rssi_value;


    /* 
     * update CCA 
     */

    cca_mode      = CC2420_REG_MDMCTRL0_CCA_MODE(cc2420->registers[CC2420_REG_MDMCTRL0]) >> 6;
    cca_hyst      = CC2420_REG_MDMCTRL0_CCA_HYST(cc2420->registers[CC2420_REG_MDMCTRL0]) >> 8;
    cca_threshold = CC2420_REG_RSSI_CCA_THR (cc2420->registers[CC2420_REG_RSSI]) >> 8;

    uint8_t old_cca_value = cc2420->cca_internal_value;
	
    switch (cca_mode) {
    case CC2420_CCA_MODE_RESERVED :
	CC2420_DBG_MUX("cc2420_record_rssi : bad CCA mode %d (reserved)\n", cca_mode);
        cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	cc2420->cca_internal_value = 0xFF;
	break;

    case CC2420_CCA_MODE_THRESHOLD :
	if (!cc2420->rx_rssi_valid) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	if (cc2420->rx_rssi_value >= cca_threshold) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	if (cc2420->rx_rssi_value < (cca_threshold - cca_hyst) ) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	    cc2420->cca_internal_value = 0xFF;
	}
	break;
    
    case CC2420_CCA_MODE_DATA :
	if (!cc2420->rx_rssi_valid) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	if (cc2420->rx_data_bytes == 0) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	    cc2420->cca_internal_value = 0xFF;
	}
	else {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	}
	break;
    
    case CC2420_CCA_MODE_BOTH :
	if (!cc2420->rx_rssi_valid) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	/* valid data first */
	if (cc2420->rx_data_bytes != 0) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	/* and check threshold */
	if (cc2420->rx_rssi_value >= cca_threshold) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    cc2420->cca_internal_value = 0x00;
	    break;
	}
	if (cc2420->rx_rssi_value < (cca_threshold - cca_hyst) ) {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	    cc2420->cca_internal_value = 0xFF;
	}
	break;

    default : 
	CC2420_DBG_MUX("cc2420_record_rssi : bad CCA mode %d\n", cca_mode);
	cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	cc2420->cca_internal_value = 0xFF;
	break;
    }

    if (old_cca_value != cc2420->cca_internal_value) {
      CC2420_DBG_RX("cc2420_record_rssi : CCA internal value changed from 0x%02x to 0x%02x\n", old_cca_value, cc2420->cca_internal_value);
    }

    return;
}


/*
 * update LQI value
 */

void cc2420_record_corr_lqi(struct _cc2420_t *cc2420, double snr)
{
  if (cc2420->corr_lqi_count >= 8) {
    return;
  }

  cc2420->corr_lqi_count++;

  if (snr > 110)
      cc2420->corr_lqi_value += 110;
  else
      cc2420->corr_lqi_value += snr;

  CC2420_DBG_RX("cc2420:rx: correlation value %d: snr = %g, corr = %d\n",
                cc2420->corr_lqi_count, snr, cc2420->corr_lqi_value / cc2420->corr_lqi_count);

  if (cc2420->corr_lqi_count == 8) {
    cc2420->corr_lqi_value /= 8;
  }
}


/**
 * check if CCA is OK
 * returns 1 if OK, 0 else
 */

int cc2420_check_cca(struct _cc2420_t * cc2420) {

    /* we just read the internal value of CCA since CCA is calculated at RX time with RSSI */
  int cc = cc2420->cca_internal_value;

  if (cc)
    return 1;
  else
    return 0;
}


/**
 * check if we can receive bytes or not
 */

int cc2420_rx_filter(struct _cc2420_t * cc2420, double frequency, int modulation,
                     double dBm, double UNUSED snr, uint8_t UNUSED rx)
{
  double freq_cc;

  /* used to calculate the time between reception of two symbols */
  int64_t sync_delta;

  /* if we're not in RX mode, drop byte */
  if ( (cc2420->fsm_state != CC2420_STATE_RX_SFD_SEARCH) &&
       (cc2420->fsm_state != CC2420_STATE_RX_FRAME) ) 
    {
      CC2420_DBG_RX("cc2420:rx: dropping received data [0x%02x,%c], not in Rx mode\n",
                    rx, isprint(rx) ? rx:'.');
      return -1;
    }

  /* check sensitivity */
  if ( dBm < -95) 
    {
      CC2420_DBG_RX("cc2420:rx: dropping received data, below sensibility\n");
      return -1;
    }

  /* check frequency */
#define FREQ_FILTER_THRESHOLD 1.0

  freq_cc = cc2420_get_frequency_mhz(cc2420);
  if ((freq_cc - frequency > FREQ_FILTER_THRESHOLD) || 
      (frequency - freq_cc > FREQ_FILTER_THRESHOLD)) /* fabs */
    {
      CC2420_DBG_RX("cc2420:rx: dropping received data, wrong frequency\n");
      return -1;
    }

  /* Verify cc2420 modulation */
  if (cc2420_get_modulation(cc2420) != modulation) 
    {
      CC2420_DBG_RX("cc2420:rx: dropping received data, wrong modulation\n");
      return -1;
    }

  /* record RSSI twice since we have two bytes / symbol */
  cc2420_record_rssi(cc2420, dBm);
  cc2420_record_rssi(cc2420, dBm);

  /* if not 1st byte of frame check synchronisation */
  /* todo : check sync_delta limit values */
#define SYNC_DELTA 33333
  if (cc2420->rx_data_bytes > 0) 
    {
      sync_delta = MACHINE_TIME_GET_NANO() - cc2420->rx_sync_timer;
      if (sync_delta < -SYNC_DELTA) 
	{
	  CC2420_DBG_RX("cc2420_callback_rx : bad sync, got byte too early : sync_delta is %"PRId64"d\n", sync_delta);
	  CC2420_RX_SFD_SEARCH_ENTER(cc2420);
	  return -1;
	}
      if (sync_delta > SYNC_DELTA) {
	CC2420_DBG_RX("cc2420_callback_rx : bad sync, got byte too late\n");
	CC2420_RX_SFD_SEARCH_ENTER(cc2420);
	return -1;
      }
    }
    
  cc2420->rx_sync_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
  
  return 0;
}


/**
 * calculate the number of required zero symbols during synchronisation
 * cf [1] p 37
 */

int cc2420_sync_zero_symbols(struct _cc2420_t * cc2420) {
    uint16_t syncword;
    uint8_t  zero_symbols = 1;

    syncword = cc2420->registers[CC2420_REG_SYNCWORD];

    if (syncword & 0x00F0) {
	zero_symbols ++;
	if (syncword & 0x000F)
	    zero_symbols ++;
    }

    return zero_symbols;
}


/**
 * check frame crc
 */

int cc2420_rx_check_crc(struct _cc2420_t * cc2420) {
    uint16_t fcs;
    uint16_t received_fcs;
    uint8_t  buffer[CC2420_RAM_RXFIFO_LEN];

    /* copy last received frame to a buffer to compute and check crc */
    cc2420_rx_fifo_get_buffer(cc2420, cc2420->rx_frame_start + 1, buffer, cc2420->rx_len);

    if (cc2420->rx_len - 2 < 0) {
	CC2420_DBG_RX("invalid len %d, can't check crc\n", cc2420->rx_len);
	return -1;
    }

    /* calculate fcs */
    fcs = cc2420_icrc(buffer, cc2420->rx_len - 2);

    /* get fcs received within frame */
    received_fcs = *((uint16_t *)(buffer + cc2420->rx_len - 2));

    /* and compare them */
    if (fcs != received_fcs) {
	CC2420_DBG_RX("cc2420_rx_check_frame : bad crc\n");
	return -1;
    }

    return 0;
}


/**
 * swap bits within a byte
 */

uint8_t swapbits(uint8_t c,int count) {
    uint8_t result=0;
    int     i;
 
    for(i = 0; i < count; i++) {
	result  = result << 1;
	result |= (c & 1);
	c = c >> 1;
    }
    return result;
}


/**
 * process frame control field to determine addressing modes etc...
 */

int cc2420_rx_process_fcf(struct _cc2420_t * cc2420) {

    CC2420_DBG_RX("cc2420_process_fcf: got fcf %.2x\n", cc2420->rx_fcf);

    cc2420->rx_frame_type    = swapbits( CC2420_FCF_FRAME_TYPE   (cc2420->rx_fcf), CC2420_FCF_FRAME_TYPE_LENGTH );
    cc2420->rx_sec_enabled   = swapbits( CC2420_FCF_SEC_ENABLED  (cc2420->rx_fcf), CC2420_FCF_SEC_ENABLED_LENGTH );
    cc2420->rx_frame_pending = swapbits( CC2420_FCF_FRAME_PENDING(cc2420->rx_fcf), CC2420_FCF_FRAME_PENDING_LENGTH );
    cc2420->rx_ack_req       = swapbits( CC2420_FCF_ACK_REQUEST  (cc2420->rx_fcf), CC2420_FCF_ACK_REQUEST_LENGTH );
    cc2420->rx_intra_pan     = swapbits( CC2420_FCF_INTRA_PAN    (cc2420->rx_fcf), CC2420_FCF_INTRA_PAN_LENGTH );
    cc2420->rx_dst_addr_mode = swapbits( CC2420_FCF_DST_ADDR_MODE(cc2420->rx_fcf), CC2420_FCF_DST_ADDR_MODE_LENGTH );
    cc2420->rx_src_addr_mode = swapbits( CC2420_FCF_SRC_ADDR_MODE(cc2420->rx_fcf), CC2420_FCF_SRC_ADDR_MODE_LENGTH );

    CC2420_DBG_RX("cc2420_process_fcf: ack_req is %d\n", cc2420->rx_ack_req);
    CC2420_DBG_RX("cc2420_process_fcf: frame_type is %d\n", cc2420->rx_frame_type);

    /* process addressing modes to determine addresses lengths and positions */

    switch (cc2420->rx_dst_addr_mode) {
    case CC2420_ADDR_MODE_EMPTY :
	cc2420->rx_dst_pan_len     = 0;
	cc2420->rx_dst_addr_len    = 0;
	break;
    case CC2420_ADDR_MODE_RESERVED :
	cc2420->rx_dst_pan_len     = 0;
	cc2420->rx_dst_addr_len    = 0;
	CC2420_DBG_RX("cc2420_process_fcf: using reserved addressing mode !\n");
	break;
    case CC2420_ADDR_MODE_16_BITS :
	cc2420->rx_dst_pan_len     = 2;
	cc2420->rx_dst_addr_len    = 2;
	break;
    case CC2420_ADDR_MODE_64_BITS :
	cc2420->rx_dst_pan_len     = 2;
	cc2420->rx_dst_addr_len    = 8;
	break;
    }

    switch (cc2420->rx_src_addr_mode) {
    case CC2420_ADDR_MODE_EMPTY :
	cc2420->rx_src_pan_len     = 0;
	cc2420->rx_src_addr_len    = 0;
	break;
    case CC2420_ADDR_MODE_RESERVED :
	cc2420->rx_src_pan_len     = 0;
	cc2420->rx_src_addr_len    = 0;
	CC2420_DBG_RX("cc2420_process_fcf: using reserved addressing mode !\n");
	break;
    case CC2420_ADDR_MODE_16_BITS :
	cc2420->rx_src_pan_len     = 2;
	cc2420->rx_src_addr_len    = 2;
	break;
    case CC2420_ADDR_MODE_64_BITS :
        cc2420->rx_src_pan_len     = 2;
	cc2420->rx_src_addr_len    = 8;
	break;
    }

    /* if intra-pan bit set, dest and src pan id are the same, so src pan id is not in the frame */
    if (cc2420->rx_intra_pan)
        cc2420->rx_src_pan_len = 0;

    cc2420->rx_dst_pan_offset  = 4; /* 4 for length field, FCF and sequence field */
    cc2420->rx_dst_addr_offset = cc2420->rx_dst_pan_offset  + cc2420->rx_dst_pan_len;
    cc2420->rx_src_pan_offset  = cc2420->rx_dst_addr_offset + cc2420->rx_dst_addr_len;
    cc2420->rx_src_addr_offset = cc2420->rx_src_pan_offset  + cc2420->rx_src_pan_len;
    
    CC2420_DBG_RX("cc2420_process_fcf: dst addr len is %d/%d, src addr len is %d/%d\n",
		 cc2420->rx_src_addr_len, cc2420->rx_src_pan_len, cc2420->rx_dst_addr_len, cc2420->rx_dst_pan_len);

    return 0;
}


/**
 * print a buffer, for debug
 */

void print_buf(char * msg, uint8_t * buf, uint8_t len) {

    char debug_str[256];
    char tmp      [256];
    int  i;

    strcpy(debug_str, msg);
    for (i = 0; i < len; i++) {
	sprintf(tmp, "%.2x:", buf[i]);
	strcat(debug_str, tmp);
    }
    strcat(debug_str, "\n");
    CC2420_DBG_RX(debug_str);
}


/**
 * check address recognition (nutshell)
 * returns 0 if address test passes, -1 otherwise
 */

int cc2420_rx_check_address(struct _cc2420_t * cc2420 UNUSED) {

    uint8_t buffer[256];
    uint8_t offset = 0;

    uint8_t src_addr[8];
    uint8_t src_pan [2];
    uint8_t dst_addr[8];
    uint8_t dst_pan [2];

    uint8_t broadcast_addr [8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


    /* no address recognition for acknoledge frames */
    if(cc2420->rx_frame_type == CC2420_FRAME_TYPE_ACK) {
        CC2420_DBG_RX("cc2420:rx:check_address: Frame type = ACK, no adress recognition\n");
        return 0;
    }
  
  
    /* calculate size corresponding to addresses */

    uint8_t addr_len = cc2420->rx_src_addr_offset + cc2420->rx_src_addr_len - cc2420->rx_dst_pan_offset;
    CC2420_DBG_RX("cc2420:rx:check_address: address len is %d\n", addr_len);

    CC2420_DBG_RX("cc2420:rx:check_address: in check_address, dst addr len is %d/%d, src addr len is %d/%d\n", cc2420->rx_src_addr_len, cc2420->rx_src_pan_len, 
			 cc2420->rx_dst_addr_len, cc2420->rx_dst_pan_len);


    if (addr_len == 0) {
	CC2420_DBG_RX("cc2420:rx:check_address: no address, check what to do here\n");
	return 0;
    }
    
    /* get buffer corresponding to src and dst addresses */
    cc2420_rx_fifo_get_buffer(cc2420, cc2420->rx_frame_start + cc2420->rx_dst_pan_offset, buffer, addr_len);

    uint8_t * ptr = &buffer[0];


    if (cc2420->rx_dst_pan_len > 0) {
	memcpy(dst_pan, ptr + offset, cc2420->rx_dst_pan_len);
	offset += cc2420->rx_dst_pan_len;
	print_buf("dst_pan  is ", dst_pan, cc2420->rx_dst_pan_len);
    }

    if (cc2420->rx_dst_addr_len > 0) {
	memcpy(dst_addr, ptr + offset, cc2420->rx_dst_addr_len);
	offset += cc2420->rx_dst_addr_len;
	print_buf("dst_addr is ", dst_addr, cc2420->rx_dst_addr_len);
    }

    if (cc2420->rx_src_pan_len > 0) {
	memcpy(src_pan, ptr + offset, cc2420->rx_src_pan_len);
	offset += cc2420->rx_src_pan_len;
	print_buf("src_pan  is ", src_pan, cc2420->rx_src_pan_len);
    }

    if (cc2420->rx_src_addr_len > 0) {
	memcpy(src_addr, ptr + offset, cc2420->rx_src_addr_len);
	offset += cc2420->rx_src_addr_len;
	print_buf("src_addr is ", src_addr, cc2420->rx_src_addr_len);
    }

    /* cf [1] p. 31, 41 and [2] p. 139 for address recognition */

    if (cc2420->rx_frame_type == CC2420_FRAME_TYPE_BEACON) {
        if (cc2420->rx_src_pan_len != 2) {
            CC2420_DBG_RX("cc2420:rx:check_address: no src pan ID for a beacon frame, dropping\n");
            return -1;
        }
        /* get pan id */

        /* if pan id is 0xFFFF accept any source pan id */
        if (!memcmp(cc2420->ram + CC2420_RAM_PANID, broadcast_addr, 2)) {
            CC2420_DBG_RX("cc2420:rx:check_address: local pan id is 0xFFFF, won't check src pan id\n");
        }
        else {
            if (memcmp(cc2420->ram + CC2420_RAM_PANID, src_pan, 2)) {
                CC2420_DBG_RX("cc2420:rx:check_address: local pan id and src pan id are different on a beacon frame, dropping\n");
                return -1;
            }
        }
    }

    /* check dst pan id */
    if (cc2420->rx_dst_pan_len > 0) {
        /* if not broadcast */
        if (!memcmp(dst_pan, broadcast_addr, 2)) {
            CC2420_DBG_RX("cc2420:rx:check_address: dst pan id is broadcast, not checking\n");
        }
        else {
            if (memcmp(cc2420->ram + CC2420_RAM_PANID, dst_pan, 2)) {
	      CC2420_DBG_RX("cc2420:rx:check_address: local pan id (%x,%x) and dst pan id (%x,%x) are different, dropping\n", *(cc2420->ram + CC2420_RAM_PANID), *(cc2420->ram + CC2420_RAM_PANID + 1), dst_pan[0], dst_pan[1]);
                return -1;
            }
        }
    }

    /* check short dst address */
    if (cc2420->rx_dst_addr_len == 2) {
        /* if not broadcast */
        if (!memcmp(dst_addr, broadcast_addr, 2)) {
            CC2420_DBG_RX("cc2420:rx:check_address: dst short address is broadcast, not checking\n");
        }
        else {
            if (memcmp(cc2420->ram + CC2420_RAM_SHORTADR, dst_addr, 2)) {
                CC2420_DBG_RX("cc2420:rx:check_address: dst short address and local short addresses are different, dropping\n");
                return -1;
            }
        }
    }

    /* check extended dst address */
    else if (cc2420->rx_dst_addr_len == 8) {
        /* if not broadcast */
        if (!memcmp(dst_addr, broadcast_addr, 8)) {
            CC2420_DBG_RX("cc2420:rx:check_address: dst short address is broadcast, not checking\n");
        }
        else {
            if (memcmp(cc2420->ram + CC2420_RAM_IEEEADR, dst_addr, 8)) {
                CC2420_DBG_RX("cc2420:rx:check_address: dst short address and local extended addresses are different, dropping\n");
                return -1;
            }
        }
    }

    /* if there only source addresses, check if we are coordinator and pan ID */
    if ( (cc2420->rx_dst_pan_len == 0) && (cc2420->rx_dst_addr_len == 0) ) {

        if (!CC2420_REG_MDMCTRL0_PAN_COORDINATOR(cc2420->registers[CC2420_REG_MDMCTRL0])) {
            CC2420_DBG_RX("cc2420:rx:check_address: only source addressing fields, and I'm not a coordinator, dropping\n");
            return -1;
        }

        /* if there is no pan id, drop */
        if (cc2420->rx_src_pan_len == 0) {
            CC2420_DBG_RX("cc2420:rx:check_address: only source addressing fields, but no src pan id, dropping\n");
            return -1;
        }
        /* if source pan id doesn't match , drop */
        if (memcmp(cc2420->ram + CC2420_RAM_PANID, src_pan, 2)) {
            CC2420_DBG_RX("cc2420:rx:check_address:only source addressing fields, but bad pan id, dropping\n");
            return -1;
        }
    }

    return 0;
}


/**
 * flush current rx frame
 */

void cc2420_rx_flush_current_frame(struct _cc2420_t * cc2420) {
    
  int current_frame_end = (cc2420->rx_frame_start + cc2420->rx_len) % CC2420_RAM_RXFIFO_LEN;
   
    /* reset write pointer to the first byte of the current rx frame */
    cc2420->rx_fifo_write = cc2420->rx_frame_start;
    
    /* check if this frame was being read */
    if (cc2420->nb_rx_frames == 0) {
        cc2420->rx_fifo_read = cc2420->rx_frame_start;
	cc2420->rx_frame_end = 0;
    }
    else if ( ((cc2420->rx_frame_start < cc2420->rx_fifo_read) && (cc2420->rx_fifo_read< current_frame_end))   ||
	      ((cc2420->rx_fifo_read < current_frame_end)      && (current_frame_end < cc2420->rx_frame_start))||
	      ((current_frame_end < cc2420->rx_frame_start)    && (cc2420->rx_frame_start < cc2420->rx_fifo_read)) ) {
        cc2420->rx_fifo_read = cc2420->rx_frame_start;
    }
   
  return;
}


/**
 * rx callback
 */

uint64_t cc2420_callback_rx(void *arg, struct wsnet_rx_info *wrx)
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *) arg;

  uint8_t rx       = wrx->data;
  double frequency = wrx->freq_mhz;
  int modulation   = wrx->modulation;
  double dBm       = wrx->power_dbm;
  double snr       = wrx->SiNR;


    /* check if we are able to receive data */
    if (cc2420_rx_filter(cc2420, frequency, modulation, dBm, snr, rx)) {
	return 0;
    }
    uint16_t addr_decode = CC2420_REG_MDMCTRL0_ADR_DECODE(cc2420->registers[CC2420_REG_MDMCTRL0]);
    uint16_t autocrc = CC2420_REG_MDMCTRL0_AUTOCRC(cc2420->registers[CC2420_REG_MDMCTRL0]);

    CC2420_DBG_RX("cc2420:rx:callback: entering RX Callback, rx data 0x%02x [%c]\n", rx,
                  isprint(rx) ? rx:'.');

    /* log rx byte */
    logpkt_rx_byte(cc2420->worldsens_radio_id, rx);

    /* according to current state, deal with data */
    switch (cc2420->fsm_state) {
    case CC2420_STATE_RX_SFD_SEARCH :
	/* one more sync byte */
	if (rx == 0) {
	    cc2420->rx_zero_symbols += 2;
	    return 0;
	}

	/* byte is not 0, and we got enough sync zeros : compare received byte with expected syncword */
	if (cc2420->rx_zero_symbols >= cc2420_sync_zero_symbols(cc2420) ) {
	    if (rx == CC2420_HIBYTE(cc2420->registers[CC2420_REG_SYNCWORD]) ) {
		CC2420_RX_FRAME_ENTER(cc2420);
		return 0;
	    }
	}

	CC2420_DBG_RX("cc2420:rx:callback: non 0 byte not expected, dropping\n");

	/* not a zero, not syncword -> drop and resynchronize */
	logpkt_rx_abort_pkt(cc2420->worldsens_radio_id, "bad sync word");
	CC2420_RX_SFD_SEARCH_ENTER(cc2420);
	return 0;

    case CC2420_STATE_RX_FRAME :
	/* we are synchronized, receive data */

	/* if this is the first RX byte, it is length field */
	if (cc2420->rx_data_bytes == 0) {
	    /* just keep the 7 low bits */
	    cc2420->rx_len = CC2420_TX_LEN_FIELD(rx);
	    CC2420_DBG_RX("cc2420:rx:callback: got rx_len %d\n", cc2420->rx_len);

	    /* set FIFO pin since we have data in fifo */
	    cc2420->FIFO_pin    = 0xFF;
	    cc2420->FIFO_set    = 1;

	    /* reset correlation for LQI */
	    cc2420->corr_lqi_value = 0;
	    cc2420->corr_lqi_count = 0;

	    /* if this is the only frame in fifo, calculate rx_frame_end */
	    /* rx_frame_end will be used in rx_fifo_pop to update the state of FIFOP */
	    if (cc2420->nb_rx_frames == 0) {
		cc2420->rx_frame_end = cc2420->rx_frame_start + cc2420->rx_len;
		if (cc2420->rx_frame_end >= CC2420_RAM_RXFIFO_LEN) {
		  cc2420->rx_frame_end -= CC2420_RAM_RXFIFO_LEN;
		}
	    }
	}

	/* update number of received bytes */
	cc2420->rx_data_bytes ++;

	/* update correlation / LQI for 8 first bytes */
	cc2420_record_corr_lqi(cc2420,snr);

	/* if first byte of data and no other pending frame in rx fifo, save firts byte position in RX FIFO */
	if (cc2420->rx_data_bytes == 1 && cc2420->nb_rx_frames == 0) {
	    /* if we don't already have data bytes in RX FIFO */
	    cc2420->rx_first_data_byte = cc2420->rx_fifo_write;
	}

	/* write byte to RX FIFO */
	if (cc2420_rx_fifo_push(cc2420, rx) < 0) {
	    logpkt_rx_abort_pkt(cc2420->worldsens_radio_id, "rx overflow");
	    CC2420_DBG_RX("cc2420:rx:callback: failed in push\n");
	    CC2420_RX_OVERFLOW_ENTER(cc2420);
	    cc2420->FIFOP_pin = 0xFF;
	    cc2420->FIFOP_set = 1;
	    cc2420->FIFO_pin  = 0x00;
	    cc2420->FIFO_set  = 1;
	    return 0;
	}


	/* Warning! cc2420 chipset is compliant with 802.15.4, but doesn't check if it is the case for user frames.
	   Thus if address decoding isn't enabled, user can choose to send data instead fcf for instance, cc2420 
	   doesn't care about that, frame will be sent all the same. */

	/* first byte of FCF */
	if (cc2420->rx_data_bytes == 2) {
	    CC2420_DBG_RX("cc2420:rx:callback: 1st byte of fcf is %.1x, swapped %.1x\n", rx, swapbits(rx,8));
	    /*
	      CC2420_DBG_RX("cc2420:rx:callback: swapping byte, todo : check that it's not 
	      a bug in cc2420 in cc2420.c/com_send driver\n");
	    */
	    cc2420->rx_fcf = swapbits(rx,8);
	}

	/* second byte of FCF */
	if (cc2420->rx_data_bytes == 3) {
	    CC2420_DBG_RX("cc2420:rx:callback: 2nd byte of fcf is %.1x, swapped %.1x\n", rx, swapbits(rx,8));
	    cc2420->rx_fcf = (cc2420->rx_fcf << 8) | (swapbits(rx,8)) ; 
	}

	/* sequence field */
	if (cc2420->rx_data_bytes == 4) {
	    cc2420->rx_sequence = rx;
	    CC2420_DBG_RX("cc2420:rx:callback: seq is %d\n", rx);
	}


	/* if address recognition is set, and addressing fields were received, deal with address recognition */
	if (addr_decode) {

	    if (cc2420->rx_data_bytes == 3) {
	        /* the FCF was fully received, process it */
	        cc2420_rx_process_fcf(cc2420);
	    }

	    /* if addressing fields were received, deal with address recognition */
	    if (cc2420->rx_data_bytes == cc2420->rx_src_addr_offset + cc2420->rx_src_addr_len) {
	        CC2420_DBG_RX("cc2420:rx:callback: got last addressing byte, data bytes is %d\n", cc2420->rx_data_bytes);
		/* if address checking recognition fails, set flag to indicate that frame has to be flushed */
		if (cc2420_rx_check_address(cc2420)) {
		    CC2420_DBG_RX("cc2420:rx:callback: address recognition failed, will flush received frame when complete\n");
		    cc2420->rx_addr_decode_failed = 1;
		    return 0;
		}
		else {
		    uint8_t rx_threshold = CC2420_REG_IOCFG0_FIFOP_THR(cc2420->registers[CC2420_REG_IOCFG0]);
		    CC2420_DBG_RX("cc2420:rx:callback: address recognition OK, checking threshold (%d)\n", rx_threshold);
		    if (cc2420->rx_data_bytes >= rx_threshold && cc2420->FIFOP_pin == 0x00) {
		      CC2420_DBG_RX("cc2420:rx:callback: rx_threshold (%d) reached, setting up FIFOP\n", rx_threshold);
			cc2420->FIFOP_pin = 0xFF;
			cc2420->FIFOP_set =    1;
		    }
		}
	    }
	} /* End of addressing recognition part */
	else
	  {
	    uint8_t rx_threshold = CC2420_REG_IOCFG0_FIFOP_THR(cc2420->registers[CC2420_REG_IOCFG0]);
	    if (cc2420->rx_data_bytes >= rx_threshold && cc2420->FIFOP_pin == 0x00) {
	      CC2420_DBG_RX("cc2420:rx:callback: rx_threshold (%d) reached, setting up FIFOP\n", rx_threshold);
	      cc2420->FIFOP_pin = 0xFF;
	      cc2420->FIFOP_set =    1;
	    }
	  }


	/* if we got a complete frame, (+1 for length field) */
	if ( cc2420->rx_data_bytes == (cc2420->rx_len + 1) ) {

	    /* check if hardware CRC is enabled */
	    if (autocrc) {
	        /* check that received frame is ok, ie crc and address recognition 
	        else flush the frame */
	        if (cc2420_rx_check_crc(cc2420)) {
		    logpkt_rx_abort_pkt(cc2420->worldsens_radio_id, "crc check failed");
		    CC2420_DBG_RX("cc2420:rx:callback: crc check failed, flushing received frame\n");
		    cc2420_rx_flush_current_frame(cc2420);
		    CC2420_RX_SFD_SEARCH_ENTER(cc2420);
		    return 0;
		}
		/* CRC is ok, so replace the 2 FCS bytes by RSSI and CRC/Corr (see 16.4 p38) */
		else {
		    cc2420_fcs_replace(cc2420);
		}
	    }

            if (cc2420->rx_addr_decode_failed) {
	        logpkt_rx_abort_pkt(cc2420->worldsens_radio_id, "addr recognition failed");
                CC2420_DBG_RX("cc2420:rx:callback: at end of frame, address recog was bad, flushing frame\n");
                cc2420_rx_flush_current_frame(cc2420);
                CC2420_RX_SFD_SEARCH_ENTER(cc2420);
                return 0;
            }

	    /* update number of fully received frames in RX FIFO */
	    cc2420->nb_rx_frames ++;

	    CC2420_DBG_RX("cc2420:rx:callback: got a new frame, nb_rx_frames is now %d\n", cc2420->nb_rx_frames);

	    /* end of packet -> packet ready to be dump */
	    logpkt_rx_complete_pkt(cc2420->worldsens_radio_id);

	    /* we are at the end of a new frame, if address recognition is disabled set FIFOP */
	    cc2420->FIFOP_pin = 0xFF;
	    cc2420->FIFOP_set = 1;

            /* if ack is requested */
            if ( (cc2420_read_register(cc2420, CC2420_REG_MDMCTRL0) & 0x10)
		 && cc2420->rx_ack_req 
		 && cc2420->rx_frame_type != CC2420_FRAME_TYPE_ACK) {
                CC2420_TX_ACK_CALIBRATE_ENTER(cc2420);
                return 0;
            }

	    CC2420_RX_SFD_SEARCH_ENTER(cc2420);
	    return 0;
	}
	
	break;

    default :
        ERROR("cc2420:rx:callback: bad state for RX\n");
	return 0;
    }

    return 0;
}




/**
 * Set CRC bit to 1 of the last received frame
 */
void cc2420_fcs_replace(struct _cc2420_t *cc2420)
{
    /* Set the significant bit in the last byte of the frame to 1 */
    cc2420->ram[CC2420_RAM_RXFIFO_START + cc2420->rx_frame_start + cc2420->rx_len] = CC2420_CRC_OK |
      (cc2420->corr_lqi_value & 0x7f);
  
    /* Replace the first byte of FCS by RSSI average value */
    cc2420->ram[CC2420_RAM_RXFIFO_START + cc2420->rx_frame_start + cc2420->rx_len - 1] = cc2420->rx_rssi_value_for_fcs;

    return;
}

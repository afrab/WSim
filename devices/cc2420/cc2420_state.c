
/**
 *  \file   cc2420_state.c
 *  \brief  CC2420 state machine
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_state.c
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc2420_internals.h"
#include "cc2420_dev.h"
#include "cc2420_state.h"
#include "cc2420_debug.h"
#include "cc2420_registers.h"
#include "cc2420_ram.h"
#include "cc2420_macros.h"
#include "cc2420_tx.h"
#include "cc2420_rx.h"
#include "cc2420_crc_ccitt.h"
#include "cc2420_mux.h"


uint8_t cc2420_read_pin  (struct _cc2420_t * cc2420, uint8_t pin);
int     cc2420_io_pins   (struct _cc2420_t * cc2420);

/**
 * check if a reset was asked by hardware (pin) or SPI
 * returns 1 if true, 0 otherwise
 */

int cc2420_reset_required(struct _cc2420_t * cc2420) {

    uint16_t value;

    /* check reset pin (active low) */
    if ( (cc2420->RESET_set) && (cc2420->RESET_pin == 0x00) ) {
	CC2420_DEBUG("cc2420_update : reset pin is set\n");
	return 1;
    }

    /* check configuration register */
    value = cc2420_read_register(cc2420, CC2420_REG_MAIN);
    if (CC2420_REG_MAIN_RESETn(value) == 0x0000) {
	CC2420_DEBUG("cc2420_update : reset bit of main register is set\n");
	return 1;
    }

    return 0;	
}


/**
 * update state VREG OFF (voltage regulator is off)
 */

void cc2420_update_state_vreg_off(struct _cc2420_t * cc2420) {

    /* if VREG_EN is high, enter vreg starting mode */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0xFF) {
	    CC2420_VREG_STARTING_ENTER(cc2420);
	}
    }

    return;
}


/**
 * update state VREG STARTING (waiting until voltage regulator is on)
 */

void cc2420_update_state_vreg_starting(struct _cc2420_t * cc2420) {

    /* if VREG_EN is low, go back to VREG_OFF state */
    if ( cc2420->VREG_EN_set && (cc2420->VREG_EN_pin == 0x00) ) {
	CC2420_VREG_OFF_ENTER(cc2420);
	return;
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* check if voltage regulator is running */
    if (MACHINE_TIME_GET_NANO() > cc2420->fsm_timer) {
	CC2420_POWER_DOWN_ENTER(cc2420);
	return;
    }
}


/**
 * update state RESET
 */

void cc2420_update_state_reset(struct _cc2420_t * cc2420) {
    uint16_t value;

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* check main configuration register and reset pin */
    value = cc2420_read_register(cc2420, CC2420_REG_MAIN);
    if ( (cc2420->RESET_pin != 0x00) && (CC2420_REG_MAIN_RESETn(value) != 0x0000) ) {
	CC2420_POWER_DOWN_ENTER(cc2420);
	return;
    }

    return;
}


/**
 * update state POWER DOWN (voltage reg is on)
 */

void cc2420_update_state_power_down(struct _cc2420_t * cc2420) {
    
    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    return;
}


/**
 * update state xosc starting
 */

void cc2420_update_state_xosc_starting(struct _cc2420_t * cc2420) {
    
    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* check if oscillator is running */
    if (MACHINE_TIME_GET_NANO() >= cc2420->fsm_timer) {
	CC2420_IDLE_ENTER(cc2420);
	return;
    }
}


/**
 * update state idle
 *
 */

void cc2420_update_state_idle(struct _cc2420_t * cc2420 UNUSED) {

    cc2420_pll_register_update(cc2420);

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* going to rx / tx modes is done by command strobes */

    return;
}


/**
 * update state rx calibrate
 */

void cc2420_update_state_rx_calibrate(struct _cc2420_t * cc2420 UNUSED) {

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* PLL is locked */
    if (MACHINE_TIME_GET_NANO() >= cc2420->pll_lock_timer) {
        cc2420->pll_locked = 1;
	cc2420_pll_register_update(cc2420);
    }

    /* RSSI becomes valid */
    if (MACHINE_TIME_GET_NANO() >= cc2420->rx_rssi_timer) {
      cc2420->rx_rssi_valid = 1;
    }

    /* calibration is over */
    if (MACHINE_TIME_GET_NANO() >= cc2420->fsm_timer) {
	CC2420_RX_SFD_SEARCH_ENTER(cc2420);
	return;
    }

    return;
}


/**
 * update state rx sfd search
 */

void cc2420_update_state_rx_sfd_search(struct _cc2420_t * cc2420) {

    int prev_read = cc2420->rx_fifo_read;

    /* if no bytes were received, update RSSI each symbol period */
    if (cc2420->rx_data_bytes != 0) {
	return;
    }

    if (MACHINE_TIME_GET_NANO() >= cc2420->rx_sync_timer) {
	cc2420_record_rssi(cc2420, -100);
	cc2420->rx_sync_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
    }

    if (cc2420->rx_fifo_read != prev_read) {
	CC2420_DEBUG("************** rx_fifo_read changed : %d -> %d\n", prev_read, cc2420->rx_fifo_read);
	if ( (cc2420->rx_fifo_read > (prev_read + 1) ) || ( (cc2420->rx_fifo_read < prev_read) && (cc2420->rx_fifo_read != 0) ) ) {
	    CC2420_DEBUG("*********************** GOT THE BUG ****************\n");
	    CC2420_DEBUG("*********************** GOT THE BUG ****************\n");
	    CC2420_DEBUG("*********************** GOT THE BUG ****************\n");
	    CC2420_DEBUG("************** rx_fifo_read changed : %d -> %d\n", prev_read, cc2420->rx_fifo_read);
	}
    }

    return;
}


/**
 * update state rx frame
 */

void cc2420_update_state_rx_frame(struct _cc2420_t * cc2420 UNUSED) {

    return;
}


/**
 * update state tx ack calibrate
 */

void cc2420_update_state_tx_ack_calibrate(struct _cc2420_t * cc2420 UNUSED) {

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }


   /* PLL is locked */
    if (MACHINE_TIME_GET_NANO() >= cc2420->pll_lock_timer) {
        cc2420->pll_locked = 1;
	cc2420_pll_register_update(cc2420);
    }

    /* calibration is over */
    if (MACHINE_TIME_GET_NANO() >= cc2420->fsm_timer) {
	CC2420_TX_ACK_PREAMBLE_ENTER(cc2420);
	return;
    }

    return;
}


/**
 * update state tx ack preamble
 */

void cc2420_update_state_tx_ack_preamble(struct _cc2420_t * cc2420 UNUSED) {

    uint8_t preamble_bytes;
    uint8_t tx_byte;
    uint8_t low_symbol;
    uint8_t high_symbol;

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* if we're stil sending the last byte, just wait */
    if (MACHINE_TIME_GET_NANO() < cc2420->tx_timer) {
	return;
    }

    /* calculate the number of bytes we have to send in preamble */
    preamble_bytes = cc2420_tx_preamble_symbols(cc2420) / 2;

    /* preamble symbols */
    if (cc2420->tx_bytes < preamble_bytes) {
	/* send a 0 for synchronisation */
	cc2420_tx_byte(cc2420, 0);
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
    }
    /* first byte of sync word */
    else if (cc2420->tx_bytes == preamble_bytes) {
	tx_byte = cc2420->registers[CC2420_REG_SYNCWORD];
	/* check if 1st 4 bits are 1, then this part to 0 (cf [1], p. 37) */
	low_symbol = CC2420_LOBYTE(cc2420->registers[CC2420_REG_SYNCWORD]) & 0x0F;
	if (low_symbol == 0x0F)
	    low_symbol = 0;
	high_symbol = CC2420_LOBYTE(cc2420->registers[CC2420_REG_SYNCWORD]) & 0xF0;
	if (high_symbol == 0xF0)
	    high_symbol = 0;
	tx_byte = high_symbol | low_symbol;
	cc2420_tx_byte(cc2420, tx_byte);
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
    }
    /* 2nd byte of sync word */
    else {
	cc2420_tx_byte(cc2420, CC2420_HIBYTE(cc2420->registers[CC2420_REG_SYNCWORD]));
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
	CC2420_TX_ACK_ENTER(cc2420);
    }
    
    return;
}


/**
 * update state tx ack
 */

void cc2420_update_state_tx_ack(struct _cc2420_t * cc2420) {

    /* ack frame format is : (cf [2] p 22) */
    /* LEN (1), FC (2), SEQ (1), FCS (2) */
        
    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* if we're stil sending the previous byte, just wait */
    if (MACHINE_TIME_GET_NANO() < cc2420->tx_timer) {
	return;
    }

    /* if we're sending the length field */
    if (cc2420->tx_bytes == 0) {
        cc2420_tx_byte(cc2420, 5);
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        cc2420->tx_bytes ++;
        return;
    }

    /* frame control is : */

    /* if no TX frame pending (ie SACK command strobe or auto ACK ) */
    /* for frame pending field, cf [1] p.42 */
    
    /* 010 (frame type : ACK)  0 (no sec) 0 (no frame pending) 0 (no ack req) 0 (no pan id comp) */
    /* 000 (reserved) 00 (no dst address) 0 (frame version) 00 (no src address) */
    /* --> 0100 0000 0000 0000  field swapped --> 0100 0000 0000 0000 bytes swapped --> 0000 0010 0000 0000 */
    /* --> 0x0200 */

    /* if TX frame pending (ie SACKPEND command strobe) */
    /* 010 (frame type : ACK)  0 (no sec) 1 (frame pending) 0 (no ack req) 0 (no pan id comp) */
    /* 000 (reserved) 00 (no dst address) 0 (frame version) 00 (no src address) */
    /* --> 0100 1000 0000 0000  field swapped --> 0100 1000 0000 0000 bytes swapped --> 0001 0010 0000 0000 */
    /* --> 0x1200 */

    uint8_t fc1;
    uint8_t fc2;
    
    if (!cc2420->tx_frame_pending) {
        fc1 = 0x02;
        fc2 = 0x00;
    }
    else {
        fc1 = 0x12;
        fc2 = 0x00;
    }

    /* 1st byte of FC */
    if (cc2420->tx_bytes == 1) {
        cc2420_tx_byte(cc2420, fc1);
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        cc2420->tx_bytes ++;    
	return;
    }

    /* 2nd byte of FC */
    if (cc2420->tx_bytes == 2) {
        cc2420_tx_byte(cc2420, fc2);
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        cc2420->tx_bytes ++;
	return;
    }

    /* seq number */
    if (cc2420->tx_bytes == 3) {
        cc2420_tx_byte(cc2420, cc2420->rx_sequence);
	CC2420_DEBUG("cc2420_update_state_tx_ack:rx_sequence=%x\n",cc2420->rx_sequence);
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        cc2420->tx_bytes ++;
	return;
    }

    /* 1st byte of fcs */
    if (cc2420->tx_bytes == 4) {
        /* calculate fcs (we need a buffer) */
        uint8_t ack_data[3] = {fc1, fc2, cc2420->rx_sequence};
        cc2420->tx_fcs = cc2420_icrc(ack_data, 3);
        cc2420_tx_byte(cc2420, CC2420_LOBYTE(cc2420->tx_fcs));
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        cc2420->tx_bytes ++;
	return;
    }
    
    /* 2nd byte of fcs */
    if (cc2420->tx_bytes == 5) {
        cc2420_tx_byte(cc2420, CC2420_HIBYTE(cc2420->tx_fcs));
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
        CC2420_RX_CALIBRATE_ENTER(cc2420);
	return;
    }

    /* buggy */
    CC2420_DEBUG(" !!! BUG : bad tx_bytes in tx_ack state !!! \n");

    return;
}


/**
 * update state rx overflow
 */

void cc2420_update_state_rx_overflow(struct _cc2420_t * cc2420 UNUSED) {

    return;
}


/**
 * update state rx wait
 */

void cc2420_update_state_rx_wait(struct _cc2420_t * cc2420 UNUSED) {

    return;
}


/**
 * update state tx calibrate
 */

void cc2420_update_state_tx_calibrate(struct _cc2420_t * cc2420) {

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }


   /* PLL is locked */
    if (MACHINE_TIME_GET_NANO() >= cc2420->pll_lock_timer) {
        cc2420->pll_locked = 1;
	cc2420_pll_register_update(cc2420);
    }

    /* wait for 12 symbol periods */
    if (MACHINE_TIME_GET_NANO() >= cc2420->fsm_timer) {
	CC2420_TX_PREAMBLE_ENTER(cc2420);
    }

    return;
}


/**
 * update state tx preamble
 */

void cc2420_update_state_tx_preamble(struct _cc2420_t * cc2420 UNUSED) {

    uint8_t preamble_bytes;
    uint8_t tx_byte;
    uint8_t low_symbol;
    uint8_t high_symbol;

    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* if we're stil sending the last byte, just wait */
    if (MACHINE_TIME_GET_NANO() < cc2420->tx_timer) {
	return;
    }

    /* calculate the number of bytes we have to send in preamble */
    preamble_bytes = cc2420_tx_preamble_symbols(cc2420) / 2;

    /* preamble symbols */
    if (cc2420->tx_bytes < preamble_bytes) {
	/* send a 0 for synchronisation */
	cc2420_tx_byte(cc2420, 0);
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
    }
    /* first byte of sync word */
    else if (cc2420->tx_bytes == preamble_bytes) {
	tx_byte = cc2420->registers[CC2420_REG_SYNCWORD];
	/* check if 1st 4 bits are 1, then this part to 0 (cf [1], p. 37) */
	low_symbol = CC2420_LOBYTE(cc2420->registers[CC2420_REG_SYNCWORD]) & 0x0F;
	if (low_symbol == 0x0F)
	    low_symbol = 0;
	high_symbol = CC2420_LOBYTE(cc2420->registers[CC2420_REG_SYNCWORD]) & 0xF0;
	if (high_symbol == 0xF0)
	    high_symbol = 0;
	tx_byte = high_symbol | low_symbol;
	cc2420_tx_byte(cc2420, tx_byte);
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
    }
    /* 2nd byte of sync word */
    else {
	cc2420_tx_byte(cc2420, CC2420_HIBYTE(cc2420->registers[CC2420_REG_SYNCWORD]));
	cc2420->tx_bytes ++;
	cc2420->fsm_timer += MACHINE_TIME_GET_NANO();
	CC2420_TX_FRAME_ENTER(cc2420);
    }
    
    return;
}


/**
 * update state tx frame
 */

void cc2420_update_state_tx_frame(struct _cc2420_t * cc2420) {

    uint8_t bytes_to_send;
    uint8_t bytes_from_fifo;

    uint16_t autocrc = CC2420_REG_MDMCTRL0_AUTOCRC(cc2420->registers[CC2420_REG_MDMCTRL0]);
    
    /* if VREG_EN is low, go back to VREG_OFF state */
    if (cc2420->VREG_EN_set) {
	if (cc2420_read_pin(cc2420, CC2420_INTERNAL_VREG_EN_PIN) == 0x00) {
	    CC2420_VREG_OFF_ENTER(cc2420);
	    return;
	}
    }

    /* if reset required, go back to RESET state */
    if (cc2420_reset_required(cc2420)) {
	CC2420_RESET_ENTER(cc2420);
	return;
    }

    /* if we're stil sending the previous byte, just wait */
    if (MACHINE_TIME_GET_NANO() < cc2420->tx_timer) {
	return;
    }

    /* number of bytes that have to be sent (tx_frame_len + 1 : length field is not included in length) */
    bytes_to_send   = cc2420->tx_frame_len + 1;


    /* evaluate number of bytes we need from TX FIFO */
    if (autocrc) {
      /* if hardware CRC is enabled, number of bytes we need from TX FIFO = bytes_to_send - 2 (we add CRC) */
      bytes_from_fifo = cc2420->tx_frame_len - 1;
    }
    else {
      bytes_from_fifo = bytes_to_send;
    }


    /* transmit data from FIFO */
    if (cc2420->tx_bytes < bytes_from_fifo) {
	/* check TX underflow */
        if (cc2420->tx_bytes >= cc2420->tx_fifo_len) {
	    CC2420_DEBUG("cc2420_update_state_tx : TX UNDERFLOW\n");
	    CC2420_TX_UNDERFLOW_ENTER(cc2420);
	    return;
	}
	cc2420_tx_byte(cc2420, cc2420->ram[CC2420_RAM_TXFIFO_START + cc2420->tx_bytes]);
	CC2420_DBG("cc2420:update_state_tx_frame: val [0x%02x] pop from tx fifo\n", cc2420->ram[CC2420_RAM_TXFIFO_START + cc2420->tx_bytes);
	cc2420->tx_bytes++;
	cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
	return;
    }


    /* if autocrc is disabled TX is over, else transmit CRC*/
    if (autocrc) {

      /* transmit 1st byte of CRC */
      if (cc2420->tx_bytes == bytes_from_fifo) {     
      cc2420->tx_fcs = cc2420_icrc(&cc2420->ram[CC2420_RAM_TXFIFO_START] + 1, cc2420->tx_frame_len - 2);
      cc2420_tx_byte(cc2420, CC2420_LOBYTE(cc2420->tx_fcs));
      CC2420_DBG("cc2420:update_state_tx_frame: val [0x%02x] pop from tx fifo\n", CC2420_LOBYTE(cc2420->tx_fcs));
      cc2420->tx_bytes++;
      cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;
      return;
      }

      /* transmit 2nd byte of CRC, TX is over */
      cc2420_tx_byte(cc2420, CC2420_HIBYTE(cc2420->tx_fcs));
      CC2420_DBG("cc2420:update_state_tx_frame: val [0x%02x] pop from tx fifo\n", CC2420_HIBYTE(cc2420->tx_fcs));
    }

    CC2420_RX_CALIBRATE_ENTER(cc2420);

    return;
}


/**
 * update state tx underflow
 * nothing to do since transition to tx_underflow -> rx_calibrate is automatic
 */

void cc2420_update_state_tx_underflow(struct _cc2420_t * cc2420 UNUSED) {
    
    return;
}


/**
 * update FSM (finished state machine)
 */

int cc2420_update(int dev_num) 
{
  struct _cc2420_t    * cc2420    = (struct _cc2420_t *)machine.device[dev_num].data;


    switch (cc2420->fsm_state) {

    case CC2420_STATE_VREG_OFF :
	cc2420_update_state_vreg_off(cc2420);
	break;

    case CC2420_STATE_VREG_STARTING :
	cc2420_update_state_vreg_starting(cc2420);
	break;

    case CC2420_STATE_XOSC_STARTING :
	cc2420_update_state_xosc_starting(cc2420);
	break;

    case CC2420_STATE_RESET :
	cc2420_update_state_reset(cc2420);
	break;

    case CC2420_STATE_POWER_DOWN :
	cc2420_update_state_power_down(cc2420);
	break;

    case CC2420_STATE_IDLE :
	cc2420_update_state_idle(cc2420);
	break;

    case CC2420_STATE_RX_CALIBRATE :
	cc2420_update_state_rx_calibrate(cc2420);
	break;

    case CC2420_STATE_RX_SFD_SEARCH :
	cc2420_update_state_rx_sfd_search(cc2420);
	break;

    case CC2420_STATE_RX_FRAME :
	cc2420_update_state_rx_frame(cc2420);
	break;

    case CC2420_STATE_TX_ACK_CALIBRATE :
	cc2420_update_state_tx_ack_calibrate(cc2420);
	break;

    case CC2420_STATE_TX_ACK_PREAMBLE :
	cc2420_update_state_tx_ack_preamble(cc2420);
	break;

    case CC2420_STATE_TX_ACK :
	cc2420_update_state_tx_ack(cc2420);
	break;

    case CC2420_STATE_RX_OVERFLOW :
	cc2420_update_state_rx_overflow(cc2420);
	break;

    case CC2420_STATE_RX_WAIT :
	cc2420_update_state_rx_wait(cc2420);
	break;

    case CC2420_STATE_TX_CALIBRATE :
	cc2420_update_state_tx_calibrate(cc2420);
	break;

    case CC2420_STATE_TX_PREAMBLE :
	cc2420_update_state_tx_preamble(cc2420);
	break;

    case CC2420_STATE_TX_FRAME :
	cc2420_update_state_tx_frame(cc2420);
	break;

    case CC2420_STATE_TX_UNDERFLOW :
	cc2420_update_state_tx_underflow(cc2420);
	break;

    default : 
	CC2420_DEBUG("cc2420_update : unknown state\n");
	break;
    }

    cc2420_io_pins(cc2420);

    /* reset input pins 'set' values */
    cc2420->SI_set      = 0;
    cc2420->VREG_EN_set = 0;
    cc2420->RESET_set   = 0;

    return 0;
}




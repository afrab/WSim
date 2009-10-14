
/**
 *  \file   cc2420_mux.c
 *  \brief  CC2420 test output signals
 *  \author Loic Lemaitre
 *  \date   2009
 **/

/*
 *  cc2420_dev.c
 *  
 *
 *  Created by Loic Lemaitre
 *  Inspired from cc1100_2500_gdo.c file
 *  Copyright 2009 __WorldSens__. All rights reserved.
 *
 */


/* Test output signals implemented:
   -CCA : 0x00, 0x18
   -SFD : 0x00                         */

#include "cc2420_internals.h"
#include "cc2420_registers.h"
#include "cc2420_debug.h"
#include "cc2420_rx.h"
#include "cc2420_mux.h"

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_assert_ccamux(struct _cc2420_t *cc2420, int event, int assert) {
    uint8_t old_CCA_pin = cc2420->CCA_pin;
	
    if (event == (cc2420->registers[CC2420_REG_IOCFG1] & 0x1F)) {
        if (assert == CC2420_PIN_ASSERT) {
	    cc2420->CCA_pin = 0xFF;
	    CC2420_DBG_MUX("cc2420_assert_ccamux: cca pin aserted 0xFF (event 0x%x)\n", event);

	} else {
	    cc2420->CCA_pin = 0x00;
	    CC2420_DBG_MUX("cc2420_assert_ccamux: cca pin aserted 0x00 (event 0x%x)\n", event);
	}
    }

    if (cc2420->CCA_pin != old_CCA_pin) {
	cc2420->CCA_set = 1;
    }

}


void cc2420_assert_sfdmux(struct _cc2420_t *cc2420, int event, int assert) {	
    uint8_t old_SFD_pin = cc2420->SFD_pin;

    if (event == (cc2420->registers[CC2420_REG_IOCFG1] & (0x1F << 5))) {
        if (assert == CC2420_PIN_ASSERT) {
	    cc2420->SFD_pin = 0xFF;
	    CC2420_DBG_MUX("cc2420_assert_sfdmux: sfd pin aserted 0xFF (event 0x%x)\n", event);

	} else {
	    cc2420->SFD_pin = 0x00;
	    CC2420_DBG_MUX("cc2420_assert_sfdmux: sfd pin aserted 0x00 (event 0x%x)\n", event);
	}
    }

    if (cc2420->SFD_pin != old_SFD_pin) {
	cc2420->SFD_set = 1;
    }

}


/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_update_mux(struct _cc2420_t *cc2420, uint16_t val) {
    uint8_t event;

    /* CCAMUX */	
    event = val & 0x1F;
    switch (event) {
    case 0x00: /* cca normal operation */
        if (cc2420_check_cca(cc2420)){
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_ASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: cca pin aserted 0xFF (event 0x%x)\n", event);
	}
	else {
	    cc2420_assert_ccamux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: cca pin aserted 0x00 (event 0x%x)\n", event);
	}
	break;
    case 0x18: /* xosc stable */
        if (cc2420->xosc_stable) {
	    cc2420_assert_ccamux(cc2420, 0x18, CC2420_PIN_ASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: cca pin aserted 0xFF (event 0x%x)\n", event);
	}
	else {
	    cc2420_assert_ccamux(cc2420, 0x18, CC2420_PIN_DEASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: cca pin aserted 0x00 (event 0x%x)\n", event);
	}
	break;   
    }

    /* SFDMUX */
    event = val & (0x1F <<5);
    switch (event) {
    case 0x00: /* sfd normal operation */
        if (cc2420->fsm_state  == CC2420_STATE_TX_FRAME || cc2420->fsm_state == CC2420_STATE_RX_FRAME
	    || cc2420->fsm_state == CC2420_STATE_TX_ACK) {
	    cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_ASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: sfd pin aserted 0xFF (event 0x%x)\n", event);
	}
	else {
	    cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_DEASSERT);
	    CC2420_DBG_MUX("cc2420_update_mux: sfd pin aserted 0x00 (event 0x%x)\n", event);
	}
	break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

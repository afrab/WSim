
/**
 *  \file   cc2420_state.h
 *  \brief  CC2420 state machine
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_state.h
 *  
 *
 *  Created by Nicolas Boulicault on 31/05/07.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_STATE_H
#define _CC2420_STATE_H


/*
 * CC2420 state machine constants
 */

enum {
  CC2420_STATE_POWER_DOWN       = 0,     /* 0x00 */
  CC2420_STATE_IDLE             = 1,     /* 0x01 */
  CC2420_STATE_RX_CALIBRATE     = 2,     /* 0x02 */
  CC2420_STATE_RX_SFD_SEARCH    = 3,     /* 0x03 */
  CC2420_STATE_RX_FRAME         = 16,    /* 0x10 */
  CC2420_STATE_TX_ACK_CALIBRATE = 48,    /* 0x30 */
  CC2420_STATE_TX_ACK_PREAMBLE  = 49,    /* 0x31 */
  CC2420_STATE_TX_ACK           = 52,    /* 0x34 */
  CC2420_STATE_RX_WAIT          = 14,    /* 0x0D */
  CC2420_STATE_RX_OVERFLOW      = 17,    /* 0x11 */
  CC2420_STATE_TX_CALIBRATE     = 32,    /* 0x20 */
  CC2420_STATE_TX_PREAMBLE      = 34,    /* 0x22 */
  CC2420_STATE_TX_FRAME         = 37,    /* 0x25 */
  CC2420_STATE_TX_UNDERFLOW     = 56,    /* 0x38 */

  CC2420_STATE_VREG_OFF         = 100,   /* 0x64 */
  CC2420_STATE_VREG_STARTING    = 101,   /* 0x65 */
  CC2420_STATE_RESET            = 102,   /* 0x66 */
  CC2420_STATE_XOSC_STARTING    = 103,   /* 0x67 */
};

/* internal tx_frame states */

enum {
    CC2420_USTATE_TX_FRAME_DATA     = 0,
    CC2420_USTATE_TX_FRAME_FCS      = 1,
};

/* internal rx_frame states */

enum {
    CC2420_USTATE_RX_FRAME_PREAMBLE = 0,
    CC2420_USTATE_RX_FRAME_SFD      = 1,
    CC2420_USTATE_RX_FRAME_DATA     = 2,
    CC2420_USTATE_RX_FRAME_FCS      = 3,
};


/*
 * FSM macros
 */

/*
 * FSM macros
 */

#define CC2420_VREG_OFF_ENTER(cc2420)					\
    do {								\
        cc2420->fsm_state   = CC2420_STATE_VREG_OFF;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_VREG_OFF;	\
	cc2420->mem_access  = CC2420_ACCESS_NONE;			\
	cc2420->xosc_stable = 0;					\
	cc2420->tx_active   = 0;					\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_pll_register_update(cc2420);				\
	cc2420_assert_ccamux(cc2420, 0x18, CC2420_PIN_DEASSERT);	\
	CC2420_DBG_STATE("cc2420:state: ENTERING VREG_OFF\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_VREG_OFF);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_VREG_OFF,0);			\
    } while (0)								\


#define CC2420_VREG_STARTING_ENTER(cc2420)				\
    do {								\
        cc2420->fsm_state = CC2420_STATE_VREG_STARTING;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_VREG_STARTING; \
        cc2420->mem_access = CC2420_ACCESS_REGISTERS_ONLY;		\
        cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + CC2420_VREG_STARTUP_TIME; \
	CC2420_DBG_STATE("cc2420:state: ENTERING VREG_STARTING\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_VREG_STARTING);		\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_STARTUP,0);			\
    } while (0)								\


#define CC2420_XOSC_STARTING_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state = CC2420_STATE_XOSC_STARTING;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_XOSC_STARTING; \
	cc2420->mem_access = CC2420_ACCESS_REGISTERS_ONLY;		\
	cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + CC2420_XOSC_STARTUP_TIME; \
	CC2420_DBG_STATE("cc2420:state: ENTERING XOSC_STARTING\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_XOSC_STARTING);		\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_STARTUP,0);			\
    } while (0);							\
	


#define CC2420_RESET_ENTER(cc2420)					\
  do {									\
        cc2420->fsm_state  = CC2420_STATE_RESET;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RESET;	\
        cc2420->mem_access = CC2420_ACCESS_REGISTERS_ONLY;		\
	cc2420->tx_active  = 0;						\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_pll_register_update(cc2420);				\
        CC2420_DBG_STATE("cc2420:state: ENTERING RESET\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RESET);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_STARTUP,0);			\
  } while (0)								\


#define CC2420_POWER_DOWN_ENTER(cc2420)					\
    do {								\
	cc2420->fsm_state   = CC2420_STATE_POWER_DOWN;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_POWER_DOWN; \
        cc2420->mem_access  = CC2420_ACCESS_REGISTERS_ONLY;		\
	cc2420->xosc_stable = 0;					\
	cc2420->tx_active   = 0;					\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_pll_register_update(cc2420);				\
	cc2420_assert_ccamux(cc2420, 0x18, CC2420_PIN_DEASSERT);	\
	CC2420_DBG_STATE("cc2420:state: ENTERING POWER_DOWN\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_POWER_DOWN);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_STARTUP,0);			\
    } while (0)								\


#define CC2420_IDLE_ENTER(cc2420)					\
  do {									\
        cc2420->fsm_state     = CC2420_STATE_IDLE;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_IDLE;	\
        cc2420->mem_access    = CC2420_ACCESS_ALL;			\
	cc2420->rx_fifo_read  = 0;					\
	cc2420->rx_fifo_write = 0;					\
	cc2420->xosc_stable   = 1;					\
	cc2420->tx_active     = 0;					\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_assert_ccamux(cc2420, 0x18, CC2420_PIN_ASSERT);		\
	cc2420_pll_register_update(cc2420);				\
	CC2420_DBG_STATE("cc2420:state: ENTERING IDLE\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_IDLE);				\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_IDLE,0);			\
  } while (0)								\


#define CC2420_TX_CALIBRATE_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state  = CC2420_STATE_TX_CALIBRATE;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_CALIBRATE; \
	cc2420->fsm_timer  = MACHINE_TIME_GET_NANO() + 12 * CC2420_SYMBOL_PERIOD; \
	/* PLL lock time set */						\
	if ( CC2420_REG_FSTST1_VCO_ARRAY_CAL_LONG(cc2420->registers[CC2420_REG_FSTST1]) >> 14 ) { \
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_LONG; \
	}								\
	else{								\
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_SHORT; \
	}								\
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_CALIBRATE\n");	\
	cc2420->tx_active = 1;						\
	cc2420->tx_bytes  = 0;						\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_pll_register_update(cc2420);				\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_TX_CALIBRATE);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0)								\


#define CC2420_RX_CALIBRATE_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state = CC2420_STATE_RX_CALIBRATE;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RX_CALIBRATE; \
	cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + 12 * CC2420_SYMBOL_PERIOD; \
	cc2420->tx_active = 0;						\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	/* RSSI valid as soon as the receiver has been enabled for at least 8 symbol periods (cf 23 p48) */ \
	cc2420->rx_rssi_timer = MACHINE_TIME_GET_NANO() + 8 * CC2420_SYMBOL_PERIOD; \
	/* PLL lock time set */						\
	if ( CC2420_REG_FSTST1_VCO_ARRAY_CAL_LONG(cc2420->registers[CC2420_REG_FSTST1]) >> 14 ) { \
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_LONG; \
	}								\
	else{								\
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_SHORT; \
	}								\
	cc2420_pll_register_update(cc2420);				\
	/* update SFD pin (cf [1] p.34) */				\
	if (cc2420->SFD_pin == 0xFF) {					\
	    cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_DEASSERT);	\
	}								\
	CC2420_DBG_STATE("cc2420:state: ENTERING RX_CALIBRATE\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RX_CALIBRATE);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_RX,0);			\
    } while (0)								\


#define CC2420_TX_PREAMBLE_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state = CC2420_STATE_TX_PREAMBLE;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_PREAMBLE; \
	cc2420->tx_start_time = MACHINE_TIME_GET_NANO();		\
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_PREAMBLE\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_TX_PREAMBLE);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0)								\


#define CC2420_TX_FRAME_ENTER(cc2420)					\
  do {									\
        cc2420->fsm_state  = CC2420_STATE_TX_FRAME;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_FRAME; \
	cc2420->fsm_timer  = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD; \
	cc2420->tx_bytes   = 0;						\
	cc2420->fsm_ustate = CC2420_USTATE_TX_FRAME_DATA;		\
	/* update SFD pin (cf [1] p.34) */				\
	cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_ASSERT);		\
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_FRAME\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_TX_FRAME);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
  } while (0);								\


#define CC2420_TX_UNDERFLOW_ENTER(cc2420)				\
    do {								\
        cc2420->fsm_state = CC2420_STATE_TX_UNDERFLOW;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_UNDERFLOW; \
	cc2420->tx_underflow = 1;					\
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_UNDERFLOW\n");	\
	/* transition to RX_CALIBRATE is automatic */			\
	CC2420_RX_CALIBRATE_ENTER(cc2420);				\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_TX_UNDERFLOW);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0)								\


#define CC2420_RX_SFD_SEARCH_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state       = CC2420_STATE_RX_SFD_SEARCH;		\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RX_SFD_SEARCH; \
	cc2420->rx_sync_timer   = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD;\
	cc2420->rx_rssi_value   = 0;					\
	cc2420->rx_rssi_values  = 0;					\
	cc2420->rx_rssi_value_for_fcs = 0;                              \
	cc2420->rx_rssi_sample_index = 0;                               \
	cc2420->rx_zero_symbols = 0;					\
	cc2420->rx_len          = 0;					\
	cc2420->rx_data_bytes   = 0;					\
	cc2420->rx_fcf          = 0;					\
	cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_DEASSERT);	\
	cc2420->rx_addr_decode_failed = 0;				\
	CC2420_DBG_STATE("cc2420:state: ENTERING RX_SFD_SEARCH\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RX_SFD_SEARCH);		\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_RX,0);			\
    } while (0)								\


#define CC2420_RX_FRAME_ENTER(cc2420)					\
    do {								\
	cc2420->fsm_state      = CC2420_STATE_RX_FRAME;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RX_FRAME; \
	cc2420->rx_data_bytes  = 0;					\
	cc2420->rx_frame_start = cc2420->rx_fifo_write;			\
	cc2420->rx_sync_timer  = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD; \
	cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_ASSERT);		\
	CC2420_DBG_STATE("cc2420:state: ENTERING RX_FRAME\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RX_FRAME);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_RX,0);			\
    } while (0)								\


#define CC2420_RX_WAIT_ENTER(cc2420)					\
    do {								\
	cc2420->fsm_state = CC2420_STATE_RX_WAIT;			\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RX_WAIT;	\
	CC2420_DBG_STATE("cc2420:state: ENTERING RX_WAIT\n");		\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RX_WAIT);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_RX,0);			\
    } while (0)								\


#define CC2420_RX_OVERFLOW_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state       = CC2420_STATE_RX_OVERFLOW;		\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_RX_OVERFLOW; \
	cc2420->rx_overflow     = 1;					\
	CC2420_DBG_STATE("cc2420:state: ENTERING RX_OVERFLOW\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
                            CC2420_STATE_RX_OVERFLOW);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_RX,0);			\
    } while (0)								\


#define CC2420_TX_ACK_CALIBRATE_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state       = CC2420_STATE_TX_ACK_CALIBRATE;	\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_ACK_CALIBRATE; \
	cc2420->fsm_timer       = MACHINE_TIME_GET_NANO() + 12 * CC2420_SYMBOL_PERIOD; \
	/* PLL lock time set */						\
	if ( CC2420_REG_FSTST1_VCO_ARRAY_CAL_LONG(cc2420->registers[CC2420_REG_FSTST1]) >> 14 ) { \
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_LONG; \
	}								\
	else{								\
	    cc2420->pll_lock_timer = MACHINE_TIME_GET_NANO() + CC2420_PLL_CALIBRATION_TIME_SHORT; \
	}								\
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_ACK_CALIBRATE\n");	\
	cc2420->tx_active = 1;						\
	cc2420->tx_bytes  = 0;						\
	cc2420->rx_rssi_valid = 0;					\
	cc2420->pll_locked = 0;						\
	cc2420_pll_register_update(cc2420);				\
	tracer_event_record(TRACER_CC2420_STATE,			\
			    CC2420_STATE_TX_ACK_CALIBRATE);		\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0)								\


#define CC2420_TX_ACK_PREAMBLE_ENTER(cc2420)				\
    do {								\
	cc2420->fsm_state       = CC2420_STATE_TX_ACK_PREAMBLE;		\
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_ACK_PREAMBLE; \
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_ACK_PREAMBLE\n");	\
	tracer_event_record(TRACER_CC2420_STATE,			\
			    CC2420_STATE_TX_ACK_PREAMBLE);		\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0)								\


#define CC2420_TX_ACK_ENTER(cc2420)                                     \
    do {                                                                \
	cc2420->fsm_state       = CC2420_STATE_TX_ACK;                  \
	cc2420->registers[CC2420_REG_FSMSTATE] = CC2420_STATE_TX_ACK; \
	CC2420_DBG_STATE("cc2420:state: ENTERING TX_ACK\n");		\
	cc2420->fsm_timer  = MACHINE_TIME_GET_NANO() + 2 * CC2420_SYMBOL_PERIOD; \
	cc2420->tx_bytes   = 0;                                         \
	cc2420->fsm_ustate = CC2420_USTATE_TX_FRAME_DATA;               \
	/* update SFD pin (cf [1] p.34) */                              \
	cc2420_assert_sfdmux(cc2420, 0x00, CC2420_PIN_ASSERT);		\
	tracer_event_record(TRACER_CC2420_STATE,			\
			    CC2420_STATE_TX_ACK);			\
	etracer_slot_event(ETRACER_PER_ID_CC2420,			\
			   ETRACER_PER_EVT_MODE_CHANGED,		\
			   ETRACER_CC2420_TX,0);			\
    } while (0);							\
	

#endif

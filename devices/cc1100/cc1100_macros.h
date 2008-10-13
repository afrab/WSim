
/**
 *  \file   cc1100_macros.h
 *  \brief  CC1100 macros
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_macros.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _CC1100_MACROS_H
#define _CC1100_MACROS_H


/***************************************************/
/***************************************************/
/***************************************************/
#define CC1100_INIT_PQT(cc1100)			\
  do {						\
    cc1100->pqt = 0;				\
  } while (0) // TODO

#define CC1100_GET_PQT(cc1100) 	(cc1100->pqt >= ((cc1100->registers[CC1100_REG_PKTCTRL1] >> 5) & 0x07))

#define CC1100_UPDATE_PQT(cc1100, rx)		\
  do {						\
    cc1100->pqt += 8;				\
    if (cc1100->pqt > 31)			\
      cc1100->pqt = 31;				\
    if (cc1100->pqt < 0)			\
      cc1100->pqt = 0;				\
  } while (0) // TODO


#define CC1100_INIT_CS(cc1100)			               \
  do {							       \
  } while (0)


#define CC1100_GET_CS(cc1100)                                  1


#define CC1100_SET_CS(cc1100, dBm)		               \
  do {							       \
  } while (0)


#define CC1100_INIT_RSSI(cc1100)			       \
  do {							       \
    /* ToCheck: value  */				       \
    /* ToCheck: 0 -> -110dBm */				       \
    /* (int8_t) */ cc1100->registers[CC1100_REG_RSSI] = 0;     \
  } while (0)


#define CC1100_RECORD_RSSI(cc1100, dBm)					\
  do {									\
    if ((cc1100->fsm_state == CC1100_STATE_RX)) {			\
      /* ToCheck: 0 -> -110dBm */					\
      if (dBm < -110) {							\
	cc1100->registers[CC1100_REG_RSSI] = 0;				\
      } else {								\
	cc1100->registers[CC1100_REG_RSSI] = dBm + 110;			\
      }									\
      CC1100_SET_CS(cc1100, dBm);					\
    }									\
  } while (0)


#define CC1100_INIT_LQI(cc1100)					\
  do {								\
    /* ToCheck: 0 -> -110dBm */					\
    /* (int8_t) */ cc1100->registers[CC1100_REG_RSSI] = 0;	\
  } while (0)

#define CC1100_RECORD_LQI(cc1100, lqi)				     \
  do {								     \
    cc1100->registers[CC1100_REG_LQI] =				     \
      (cc1100->registers[CC1100_REG_LQI] & 0x80) | (lqi & 0x7F);     \
  } while (0)

#define CC1100_COMPUTE_LQI(cc1100,snr)		\
  do {						\
    if (cc1100->lqi_cnt >= 8) {			\
      break;					\
    }						\
    cc1100->lqi_cnt++;				\
    if (snr > 127)				\
      cc1100->lqi += 127;			\
    else					\
      cc1100->lqi += snr;			\
    if (cc1100->lqi_cnt == 8) {			\
      uint8_t val;				\
      cc1100->lqi /= 8;				\
      val = (uint8_t) cc1100->lqi;		\
      CC1100_RECORD_LQI(cc1100, val);		\
    }						\
  } while (0)


/***************************************************/
/***************************************************/
/***************************************************/
#define CC1100_COMPUTE_CRC(cc1100)      (cc1100->registers[CC1100_REG_PKTCTRL0] & 0x04)
#define CC1100_APPEND_CRC(cc1100)       (cc1100->registers[CC1100_REG_PKTCTRL1] & 0x04)

/***************************************************/
/***************************************************/
/***************************************************/
#define CC1100_FIXED_PKTLENGTH(cc1100)	((cc1100->registers[CC1100_REG_PKTCTRL0] & 0x03) == 0)
#define CC1100_VAR_PKTLENGTH(cc1100)	((cc1100->registers[CC1100_REG_PKTCTRL0] & 0x03) == 1)
#define CC1100_INF_PKTLENGTH(cc1100)	((cc1100->registers[CC1100_REG_PKTCTRL0] & 0x03) == 2)

#define CC1100_PKTLENGTH(cc1100)	(cc1100->registers[CC1100_REG_PKTLEN])

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_IS_CALIBRATED(cc1100)	(cc1100->fs_cal)

#define CC1100_UNCALIBRATE(cc1100)				        \
  do {									\
    CC1100_DBG_STATE("cc1100:state:calibration: lost at %"PRIu64"\n",	\
		     MACHINE_TIME_GET_NANO());				\
    cc1100->fs_cal = 0;							\
  } while (0)

#define CC1100_CALIBRATE(cc1100)				        \
  do {									\
    CC1100_DBG_STATE("cc1100:state:calibration: done at %"PRIu64"\n",	\
		     MACHINE_TIME_GET_NANO());				\
    cc1100->fs_cal = 1;							\
  } while (0)


/***************************************************/
/***************************************************/
/***************************************************/


#define CC1100_SET_CRC_TRUE(cc1100)			    \
  do {							    \
    cc1100_assert_gdo(cc1100, 0x07, CC1100_PIN_ASSERT);	    \
    cc1100->registers[CC1100_REG_LQI] |= 0x80;		    \
  } while (0)

#define CC1100_SET_CRC_FALSE(cc1100)		   \
  do {						   \
    cc1100->registers[CC1100_REG_LQI] &= 0x7F;	   \
  } while (0)


/***************************************************/
/***************************************************/
/***************************************************/


#define CC1100_TX_END(cc1100)						\
  do {									\
    CC1100_DBG_PKT("cc1100:tx:state: TX (end)\n");			\
    cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);               \
    cc1100_tx_state(cc1100);						\
  } while (0)

#define CC1100_TX_SEND_CRC(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (crc)\n");	                \
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->fsm_ustate = 4;						\
  } while (0)

#define CC1100_TX_SEND_DATA(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (data)\n");	                \
    cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_ASSERT);			\
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->ioCrc      = 0;						\
    cc1100->fsm_ustate = 3;						\
  } while (0)

#define CC1100_TX_SEND_SFD(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (sfd)\n");                 	\
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->ioCrc      = 0;						\
    cc1100->fsm_ustate = 2;						\
  } while (0)

#define CC1100_TX_SEND_PREAMBLE(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (preamble)\n");	        \
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->fsm_ustate = 1;						\
  } while (0)

#define CC1100_TX_ENTER(cc1100)						\
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (enter)\n");			\
    cc1100->fsm_state = CC1100_STATE_TX;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_TX);		\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_TX,0);				\
    CC1100_TX_SEND_PREAMBLE(cc1100);					\
    cc1100_tx(cc1100);							\
  } while (0)

#define CC1100_TX_END_ENTER(cc1100)                                     \
  do {									\
    CC1100_DBG_STATE("cc1100:tx:state: TX (end)\n");			\
    cc1100->fsm_ustate = 5;						\
    cc1100->fsm_timer =  MACHINE_TIME_GET_NANO()			\
      + CC1100_IDLE_NOCAL_DELAY_NS;					\
  } while (0)


/***************************************************/
/***************************************************/
/***************************************************/


#define CC1100_RX_SEND_PREAMBLE   1
#define CC1100_RX_SEND_SFD        2
#define CC1100_RX_SEND_DATA       3
#define CC1100_RX_SEND_CRC        4
#define CC1100_RX_SEND_END        5

#define CC1100_RX_END_ENTER(cc1100)					\
  do {									\
  cc1100->fsm_ustate = 5;						\
  cc1100->fsm_timer  =  MACHINE_TIME_GET_NANO()				\
    + CC1100_IDLE_NOCAL_DELAY_NS;					\
 } while (0)

#define CC1100_RX_END_FORCED(cc1100)					\
  do {									\
    cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);		\
    cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_ASSERT);			\
    CC1100_DBG_PKT("cc1100:rx:packet: RX FORCED\n");			\
  } while (0)

#define CC1100_RX_END(cc1100)						\
  do {									\
    cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);		\
    cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_ASSERT);			\
    CC1100_DBG_PKT("cc1100:rx:packet: RX END\n");			\
    CC1100_DBG_STATE("cc1100:rx:state: RX END [%s():%d]\n",		\
		     __FUNCTION__, __LINE__);				\
    cc1100_rx_state(cc1100);						\
  } while (0)

#define CC1100_RX_EXPECT_PREAMBLE(cc1100)				\
  do {									\
    CC1100_DBG_STATE("cc1100:rx:state: changed to expect PREAMBLE "	\
		     "[%s():%d]\n", __FUNCTION__, __LINE__);		\
    cc1100->ioOffset    = 0;						\
    cc1100->ioLength    = 0;						\
    cc1100->fsm_ustate  = 1;						\
    cc1100->rx_io_timer = 0;					        \
    CC1100_INIT_PQT(cc1100);						\
  } while (0)

#define CC1100_RX_EXPECT_SFD(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:rx:state: changed to expect SFD\n");	\
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->fsm_ustate = 2;						\
  } while (0)

#define CC1100_RX_EXPECT_DATA(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:rx:state: changed to DATA [%s():%d]\n",	\
		     __FUNCTION__, __LINE__);				\
    cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_ASSERT);			\
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->fsm_ustate = 3;						\
    cc1100->ioCrc      = 0;						\
    cc1100->addressChk = 0;						\
    CC1100_INIT_LQI(cc1100);						\
  } while (0)

#define CC1100_RX_EXPECT_CRC(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:rx:state: changed to expect CRC\n");       \
    cc1100->ioOffset   = 0;						\
    cc1100->ioLength   = 0;						\
    cc1100->fsm_ustate = 4;						\
  } while (0)

#define CC1100_RX_ENTER(cc1100)						\
  do {									\
    CC1100_DBG_STATE("cc1100:state: RX (enter)\n");			\
    CC1100_SET_CRC_TRUE(cc1100);					\
    cc1100->fsm_state   = CC1100_STATE_RX;				\
    cc1100->fsm_pending = CC1100_STATE_IDLE;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_RX);		\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_RX,0);				\
    CC1100_RX_EXPECT_PREAMBLE(cc1100);					\
    CC1100_INIT_CS(cc1100);						\
    CC1100_INIT_RSSI(cc1100);						\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_CALIBRATE_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: CALIBRATE (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_CALIBRATE;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_CALIBRATE);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO() 			\
      + CC1100_CALIBRATE_DELAY_NS;					\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_FS_CALIBRATE_ENTER(cc1100)				\
  do {									\
    CC1100_DBG_STATE("cc1100:state: FS_CALIBRATE (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_FS_CALIBRATE;			\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_FS_CALIBRATE); \
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO()				\
      + CC1100_CALIBRATE_DELAY_NS;					\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_MANCAL_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: MANCAL (enter)\n");			\
    cc1100->fsm_state = CC1100_STATE_MANCAL;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_MANCAL);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO()				\
      + CC1100_MANCAL_DELAY_NS;						\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_FSTXON_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: FSTXON (enter)\n");			\
    cc1100->fsm_state = CC1100_STATE_FSTXON;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_FSTXON);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_SETTLING_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: SETTLING (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_SETTLING;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_SETTLING);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer =  MACHINE_TIME_GET_NANO()			\
      + CC1100_SETTLING_DELAY_NS;					\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_IDLE_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: IDLE (enter) at %"PRId64"\n",	\
		     MACHINE_TIME_GET_NANO());				\
    cc1100->fsm_state = CC1100_STATE_IDLE;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_IDLE);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_IDLE,0);				\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_SLEEP_ENTER(cc1100)					\
  do {									\
    cc1100->fsm_pending = CC1100_STATE_SLEEP;				\
  } while (0)

#define CC1100_SLEEP_REALLY_ENTER(cc1100)				\
  do {									\
    CC1100_DBG_STATE("cc1100:state: SLEEP (enter) at %"PRId64"\n",	\
		     MACHINE_TIME_GET_NANO());				\
    CC1100_UNCALIBRATE(cc1100);						\
    cc1100->fsm_state = CC1100_STATE_SLEEP;				\
    cc1100->fsm_pending = CC1100_STATE_IDLE;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_SLEEP);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_SLEEP,0);				\
    /* Configuration registers set to default in sleep state (p68) */	\
    cc1100->registers[CC1100_REG_FSTEST]  = CC1100_REG_FSTEST_DEFAULT;	\
    cc1100->registers[CC1100_REG_PTEST]   = CC1100_REG_PTEST_DEFAULT;	\
    cc1100->registers[CC1100_REG_AGCTEST] = CC1100_REG_AGCTEST_DEFAULT; \
    cc1100->registers[CC1100_REG_TEST2]   = CC1100_REG_TEST2_DEFAULT;	\
    cc1100->registers[CC1100_REG_TEST1]   = CC1100_REG_TEST1_DEFAULT;	\
    cc1100->registers[CC1100_REG_TEST0]   = CC1100_REG_TEST0_DEFAULT;	\
    /* PA Table reset except for PA[0] when going sleep state (p20) */	\
    cc1100->patable[1] = 0;						\
    cc1100->patable[2] = 0;						\
    cc1100->patable[3] = 0;						\
    cc1100->patable[4] = 0;						\
    cc1100->patable[5] = 0;						\
    cc1100->patable[6] = 0;						\
    cc1100->patable[7] = 0;						\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_XOFF_ENTER(cc1100)					\
  do {									\
    cc1100->fsm_pending = CC1100_STATE_XOFF;				\
  } while (0) 

#define CC1100_XOFF_REALLY_ENTER(cc1100)				\
  do {									\
    CC1100_UNCALIBRATE(cc1100);						\
    cc1100->fsm_state = CC1100_STATE_XOFF;				\
    cc1100->fsm_pending = CC1100_STATE_IDLE;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_XOFF);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_XOFF,0);				\
    CC1100_DBG_STATE("cc1100:state: XOFF (enter)\n");			\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_FS_WAKEUP_ENTER(cc1100)					\
  do {									\
    CC1100_DBG_STATE("cc1100:state: FS_WAKEUP (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_FS_WAKEUP;				\
    tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_FS_WAKEUP);	\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO()				\
      + CC1100_FS_WAKEUP_DELAY_NS;					\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_RXTX_SETTLING_ENTER(cc1100)				\
  do {									\
    CC1100_DBG_STATE("cc1100:state: RXTX_SETTLING (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_RXTX_SETTLING;			\
    tracer_event_record(TRACER_CC1100_STATE,CC1100_STATE_RXTX_SETTLING);\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO()				\
      + CC1100_RX_TX_DELAY_NS;						\
  } while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_TXRX_SETTLING_ENTER(cc1100)				\
  do {									\
    CC1100_DBG_STATE("cc1100:state: TXRX_SETTLING (enter)\n");		\
    cc1100->fsm_state = CC1100_STATE_TXRX_SETTLING;			\
    tracer_event_record(TRACER_CC1100_STATE,CC1100_STATE_TXRX_SETTLING);\
    etracer_slot_event(ETRACER_PER_ID_CC1100,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_CC1100_STARTUP,0);			\
    cc1100->fsm_timer = MACHINE_TIME_GET_NANO()				\
      + CC1100_TX_RX_DELAY_NS;						\
} while (0)

/***************************************************/
/***************************************************/
/***************************************************/

#endif

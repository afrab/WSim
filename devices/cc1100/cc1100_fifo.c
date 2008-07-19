
/**
 *  \file   cc1100_fifo.c
 *  \brief  CC1100 Data RX/TX fifo 
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_fifo.c
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */
#include "cc1100_internals.h"

/* Tx/Rx Fifo threshold, page 46
 * value   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
 * TX     61 57 53 49 45 41 37 33 29 25 21 17 13  9  5  1
 * RX      4  8 12 16 20 24 28 32 36 40 44 48 52 56 60 64
 *
 * 10, 11, 44, 43
 */

static int tx_fifo_threshold[] = {
  61, 57, 53, 49, 45, 41, 37, 33, 29, 25, 21, 17, 13,  9,  5,  1
};

/*
static int rx_fifo_threshold[] = {
   4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64
};
*/

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_put_tx_fifo(struct _cc1100_t *cc1100, uint8_t val)
{ 
  if (cc1100->txBytes == CC1100_TXFIFO_LENGTH) 
    {
      CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo already Full [1]. Assert GDO.\n");
      cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_ASSERT);
    } 
  else 
    {
      uint8_t offset = cc1100->txOffset + cc1100->txBytes;
      
      if (offset >= CC1100_TXFIFO_LENGTH)
	{
	  offset -= CC1100_TXFIFO_LENGTH;
	}

      cc1100->txfifo[offset] = val;
      cc1100->txBytes++;

      /* FIFO is full */
      if (cc1100->txBytes == CC1100_TXFIFO_LENGTH) 
	{
	  CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo Full [2]. Assert GDO.\n");
	  cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_ASSERT);
	} 
      
      /* FIFO above Tx threshold */
      if (cc1100->txBytes >= tx_fifo_threshold[cc1100->registers[CC1100_REG_FIFOTHR]])
	{
	  CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo filled above threshold. Assert GDO.\n");
	  cc1100_assert_gdo(cc1100, 0x02, CC1100_PIN_ASSERT);
	}

      CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo written: 0x%02x (size: %d, threshold: %d)\n",
		      val & 0xff, cc1100->txBytes, tx_fifo_threshold[cc1100->registers[CC1100_REG_FIFOTHR]]);
      /* spi -> tx fifo */
      etracer_slot_event(ETRACER_PER_ID_CC1100,
			 ETRACER_PER_EVT_WRITE_COMMAND,
			 ETRACER_PER_ARG_WR_DST_FIFO | ETRACER_ACCESS_LVL_SPI0, 0);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_get_tx_fifo(struct _cc1100_t *cc1100) 
{
  uint8_t data;

  if (cc1100->txBytes == 0)
    {
      CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo underrun\n");
      cc1100_assert_gdo(cc1100, 0x05, CC1100_PIN_ASSERT);
    }
  
  data = cc1100->txfifo[cc1100->txOffset];
  cc1100->txBytes--;
  cc1100->txOffset++;
  
  if (cc1100->txOffset >= CC1100_TXFIFO_LENGTH)
    {
      cc1100->txOffset -= CC1100_TXFIFO_LENGTH;
    }

  /* FIFO below threshold */
  if (cc1100->txBytes < tx_fifo_threshold[cc1100->registers[CC1100_REG_FIFOTHR]]) 
    {
      CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo below threshold\n");
      cc1100_assert_gdo(cc1100, 0x02, CC1100_PIN_DEASSERT);
      cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_DEASSERT);
    }

  CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo internally read: 0x%02x (size: %d, threshold: %d)\n", 
		  data, cc1100->txBytes, tx_fifo_threshold[cc1100->registers[CC1100_REG_FIFOTHR]]);

  /* tx fifo -> outside */
  etracer_slot_event(ETRACER_PER_ID_CC1100, 
		     ETRACER_PER_EVT_WRITE_COMMAND, 
		     ETRACER_PER_ARG_WR_SRC_EXT | ETRACER_ACCESS_LVL_OUT, 0);
  return data;
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_flush_tx_fifo(struct _cc1100_t *cc1100) 
{
  
  cc1100->txBytes = 0;
  cc1100->txOffset = 0;
  cc1100->txUnderflow = 0;
  
  cc1100_assert_gdo(cc1100, 0x05, CC1100_PIN_DEASSERT);
  if (cc1100->txBytes < tx_fifo_threshold[cc1100->registers[CC1100_REG_FIFOTHR]])
    {
      cc1100_assert_gdo(cc1100, 0x02, CC1100_PIN_DEASSERT);
      cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_DEASSERT);
    }
	
  CC1100_DBG_FIFO("cc1100: (fifo DEBUG): TX fifo flushed (size: %d)\n", cc1100->txBytes);
  etracer_slot_event(ETRACER_PER_ID_CC1100, 
		     ETRACER_PER_EVT_WRITE_COMMAND, 
		     ETRACER_PER_ARG_WR_FLUSH | ETRACER_ACCESS_LVL_SPI0, 0);
}


/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_put_rx_fifo(struct _cc1100_t *cc1100, uint8_t val) {
	int offset;	

	/* outside -> rx fifo */
        etracer_slot_event(ETRACER_PER_ID_CC1100, 
			   ETRACER_PER_EVT_WRITE_COMMAND, 
			   ETRACER_PER_ARG_WR_DST_EXT | ETRACER_ACCESS_LVL_OUT, 0);
	
	if ((cc1100->rxBytes) == CC1100_RXFIFO_LENGTH) {
		cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
		cc1100_assert_gdo(cc1100, 0x04, CC1100_PIN_ASSERT);

		cc1100->rxOverflow = 1;
		cc1100->fsm_state = CC1100_STATE_RX_OVERFLOW;
		CC1100_DBG_FIFO("cc1100:fifo: RX fifo overflow\n");
		CC1100_DBG_STATE("cc1100:state: RX_OVERFLOW\n");
		ERROR("cc1100: RX_FIFO overflow\n");
		tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_RX_OVERFLOW);
		etracer_slot_event(ETRACER_PER_ID_CC1100,ETRACER_PER_EVT_MODE_CHANGED,ETRACER_CC1100_STARTUP,0);
		return -1;
	}
	
	offset = cc1100->rxOffset + cc1100->rxBytes;	
	if (offset >= CC1100_RXFIFO_LENGTH)
		offset -= CC1100_RXFIFO_LENGTH;
	cc1100->rxBytes++;
	cc1100->rxfifo[offset] = val;
	
	if (cc1100->rxBytes >= (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
		cc1100_assert_gdo(cc1100, 0x00, CC1100_PIN_ASSERT);
		cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_ASSERT);
	}			
	
	CC1100_DBG_FIFO("cc1100: (fifo DEBUG): RX fifo internally written: 0x%02x (size: %d)\n",  
			 val, cc1100->rxBytes);

	return 0;
}


/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_get_rx_fifo(struct _cc1100_t *cc1100) {
	uint8_t val = 0;
	
	if (cc1100->rxBytes == 0) {
		CC1100_DBG_IMPL("cc1100: (fifo IMPLEMENTATION): RX fifo empty. Unspecified behavior.\n");
	} else {
		
		cc1100_assert_gdo(cc1100, 0x07, CC1100_PIN_DEASSERT);
		
		val = cc1100->rxfifo[cc1100->rxOffset]; 
		
		cc1100->rxOffset++;
		cc1100->rxBytes--;
		
		if (cc1100->rxOffset >= CC1100_RXFIFO_LENGTH)
			cc1100->rxOffset -= CC1100_RXFIFO_LENGTH;

		if (cc1100->rxBytes == 0) {
			cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_DEASSERT);
		} 
		if (cc1100->rxBytes < (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
			cc1100_assert_gdo(cc1100, 0x00, CC1100_PIN_DEASSERT);
		}			
	}
	
	CC1100_DBG_FIFO("cc1100: (fifo DEBUG): RX fifo read: 0x%x (size: %d)\n", val, cc1100->rxBytes);
	/* rx fifo -> spi */
        etracer_slot_event(ETRACER_PER_ID_CC1100, 
			   ETRACER_PER_EVT_WRITE_COMMAND, 
			   ETRACER_PER_ARG_WR_SRC_FIFO | ETRACER_ACCESS_LVL_SPI0, 0);
	return val;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_flush_rx_fifo(struct _cc1100_t *cc1100) {
	cc1100->rxBytes = 0;
	cc1100->rxOffset = 0;
	cc1100->rxOverflow = 0;
	
	cc1100_assert_gdo(cc1100, 0x04, CC1100_PIN_DEASSERT);

	if (cc1100->rxBytes == 0) {
		cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_DEASSERT);
	} 
	if (cc1100->rxBytes < (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
		cc1100_assert_gdo(cc1100, 0x00, CC1100_PIN_DEASSERT);
	}				
	
	CC1100_DBG_FIFO("cc1100: (fifo DEBUG): RX fifo flushed (size: %d)\n", cc1100->rxBytes);
	etracer_slot_event(ETRACER_PER_ID_CC1100, 
			   ETRACER_PER_EVT_WRITE_COMMAND, 
			   ETRACER_PER_ARG_WR_FLUSH | ETRACER_ACCESS_LVL_OUT, 0);
}

/***************************************************/
/***************************************************/
/***************************************************/

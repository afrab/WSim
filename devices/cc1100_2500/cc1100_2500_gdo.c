
/**
 *  \file   cc1100_2500_gdo.c
 *  \brief  CC1100/CC2500 GDOx pins handling
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_2500_gdo.c
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified by Antoine Fraboulet 2007
 */

/*
 * Implemented signals: 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x29, 0x2E, 0x3F
 */

#include "cc1100_2500_internals.h"


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_spi_output(struct _cc1100_t *cc1100, uint8_t val) {

	if ((cc1100->registers[CC1100_REG_IOCFG0] & 0x3F) == 0x2E) {
		CC1100_DBG_SPI("cc1100: (spi DEBUG): GDO0 pin value %x\n", val);

		cc1100_write_pin(cc1100, CC1100_INTERNAL_GO0_PIN, val);	
	}
	if ((cc1100->registers[CC1100_REG_IOCFG1] & 0x3F) == 0x2E) {
		CC1100_DBG_SPI("cc1100: (spi DEBUG): GDO1 pin value %x\n", val);
		
		cc1100_write_pin(cc1100, CC1100_INTERNAL_GO1_PIN, val);	
	}
	if ((cc1100->registers[CC1100_REG_IOCFG2] & 0x3F) == 0x2E) {
		CC1100_DBG_SPI("cc1100: (spi DEBUG): GDO2 pin value %x\n", val);
		
		cc1100_write_pin(cc1100, CC1100_INTERNAL_GO2_PIN, val);	
	}
}
	
/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_assert_gdo(struct _cc1100_t *cc1100, int event, int assert) {
	
	if (event == (cc1100->registers[CC1100_REG_IOCFG0] & 0x3F)) {
		if (assert == CC1100_PIN_ASSERT) {

			if (cc1100->registers[CC1100_REG_IOCFG0] & 0x40) {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO0_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO0 pin aserted 0x00 (event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO0_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO0 pin asserted 0xFF (event %x)\n", event);
			}
		} else {
			
			if (cc1100->registers[CC1100_REG_IOCFG0] & 0x40) {	
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO0_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO0 pin deasserted 0xFF (event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO0_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO0 pin deasserted 0x00 (event %x)\n", event);
			}
		}
	}
	
	if (event == (cc1100->registers[CC1100_REG_IOCFG1] & 0x3F)) {
		if (assert == CC1100_PIN_ASSERT) {
			
			if (cc1100->registers[CC1100_REG_IOCFG1] & 0x40) {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO1_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO1 pin asserted 0x00 (event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO1_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO1 pin asserted 0xFF (event %x)\n", event);
			}
		} else {
			
			if (cc1100->registers[CC1100_REG_IOCFG1] & 0x40) {	
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO1_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO1 pin deasserted 0xFF(event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO1_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO1 pin deasserted 0x00(event %x)\n", event);
			}
		}
	}
	
	if (event == (cc1100->registers[CC1100_REG_IOCFG2] & 0x3F)) {
		if (assert == CC1100_PIN_ASSERT) {
			
			if (cc1100->registers[CC1100_REG_IOCFG2] & 0x40) {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO2_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO2 pin asserted 0x00 (event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO2_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO2 pin asserted 0xFF (event %x)\n", event);
			}
		} else {
			
			if (cc1100->registers[CC1100_REG_IOCFG2] & 0x40) {	
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO2_PIN, 0xFF);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO2 pin deasserted 0xFF (event %x)\n", event);
			} else {
				cc1100_write_pin (cc1100, CC1100_INTERNAL_GO2_PIN, 0x00);
				CC1100_DBG_GDO("cc1100: (gdo DEBUG): GDO2 pin deasserted 0x00 (event %x)\n", event);
			}
		}
	}
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_update_xosc(struct _cc1100_t *cc1100) 
{
  if ((cc1100->clk_timeref + CC1100_XOSC_PERIOD_NS) >= MACHINE_TIME_GET_NANO()) 
    {
      return;
    }
	
  cc1100->clk_tick   += (MACHINE_TIME_GET_NANO() - cc1100->clk_timeref) / CC1100_XOSC_PERIOD_NS;
  cc1100->clk_timeref =  MACHINE_TIME_GET_NANO();
		
  if (cc1100->clk_tick % 192) /* NOTE: TODO / VERIFY / CHANGE */
    {
      if (cc1100->clk_tick % 2) /* NOTE: TODO / VERIFY / CHANGE */
	{
	  cc1100_assert_gdo(cc1100, 0x3F, CC1100_PIN_ASSERT);
	} 
      else 
	{
	  cc1100_assert_gdo(cc1100, 0x3F, CC1100_PIN_ASSERT);
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_update_gdo (struct _cc1100_t *cc1100, uint8_t val) {
	uint8_t event = val & 0x3F;
	
	switch (event) {
		case 0x00:
			if (cc1100->rxBytes >= (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
				cc1100_assert_gdo(cc1100, 0x00, CC1100_PIN_ASSERT);
			} else if (cc1100->rxBytes < (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
				cc1100_assert_gdo(cc1100, 0x00, CC1100_PIN_DEASSERT);
			}			
			break;
		case 0x01:
			if (cc1100->rxBytes >= (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4)) {
				cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_ASSERT);
			} else if (cc1100->rxBytes == 0) {
				cc1100_assert_gdo(cc1100, 0x01, CC1100_PIN_DEASSERT);
			} 
			break;
		case 0x02:
			if (cc1100->txBytes >= (64 - (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4 - 1))) {
				cc1100_assert_gdo(cc1100, 0x02, CC1100_PIN_ASSERT);
			} else if (cc1100->txBytes < (64 - (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4 - 1))) {
				cc1100_assert_gdo(cc1100, 0x02, CC1100_PIN_DEASSERT);
			}
			
			break;
		case 0x03:
			if (cc1100->txBytes == CC1100_TXFIFO_LENGTH) {
				cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_ASSERT);
			} else if (cc1100->txBytes < (64 - (((cc1100->registers[CC1100_REG_FIFOTHR] & 0x0F) + 1) * 4 - 1))) {
				cc1100_assert_gdo(cc1100, 0x03, CC1100_PIN_DEASSERT);
			}
			break;
		case 0x04:
			if (cc1100->rxOverflow) {
				cc1100_assert_gdo(cc1100, 0x04, CC1100_PIN_ASSERT);
			} else {
				cc1100_assert_gdo(cc1100, 0x04, CC1100_PIN_DEASSERT);
			}
			break;
		case 0x05:
			if (cc1100->txUnderflow) {
				cc1100_assert_gdo(cc1100, 0x05, CC1100_PIN_ASSERT);
			} else {
				cc1100_assert_gdo(cc1100, 0x05, CC1100_PIN_DEASSERT);
			}
			break;
		case 0x06: //ToCheck
			cc1100_assert_gdo(cc1100, 0x06, CC1100_PIN_DEASSERT);
			break;
		case 0x07: //ToCheck
#if defined(CC2500)
		        if( (cc1100_read_register(cc1100, CC1100_REG_PKTCTRL0)) & 0x08 ) {
			    cc1100_assert_gdo(cc1100, 0x07, CC1100_PIN_DEASSERT);
			}
#elif defined(CC1100)
			cc1100_assert_gdo(cc1100, 0x07, CC1100_PIN_DEASSERT);
#endif
			break;
		case 0x29:
			cc1100_assert_gdo(cc1100, 0x29, CC1100_PIN_ASSERT);
			break;
		case 0x3F:
			cc1100_update_xosc(cc1100);
			break;
		}
	
}

/***************************************************/
/***************************************************/
/***************************************************/

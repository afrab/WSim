
/**
 *  \file   cc1100_dev.c
 *  \brief  CC1100 device model entry point
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_dev.c
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "machine/machine.h"

#include "cc1100_internals.h"

/***************************************************/
/***************************************************/
/***************************************************/

/* Global Variables (not backtracked) */
int  CC1100_XOSC_FREQ_MHz;
int  CC1100_XOSC_PERIOD_NS;

tracer_id_t TRACER_CC1100_STATE;
tracer_id_t TRACER_CC1100_STROBE;
tracer_id_t TRACER_CC1100_CS;
tracer_id_t TRACER_CC1100_SO;

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_device_size (void) 
{
  return sizeof(struct _cc1100_t);
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_write_pin(struct _cc1100_t *cc1100, uint8_t pin, uint8_t val) 
{
  /*	TODO: voir si on conserve le "if () return" */
  switch (pin) 
    {
    case CC1100_INTERNAL_GO0_PIN:
      if (cc1100->GO0_pin == val) /* reduce activity */
	return;
      cc1100->GO0_pin = val;
      cc1100->GO0_set = 1;
      return;
    case CC1100_INTERNAL_GO1_PIN:
      cc1100->GO1_pin = val;
      cc1100->GO1_set = 1;
      return;
    case CC1100_INTERNAL_GO2_PIN:
      if (cc1100->GO2_pin == val) /* reduce activity */
	return;
      cc1100->GO2_pin = val;
      cc1100->GO2_set = 1;
      return;
    default:
      return;
    }
}


/***************************************************/
/***************************************************/
/***************************************************/

uint8_t cc1100_read_pin (struct _cc1100_t *cc1100, uint8_t pin) 
{
  switch (pin) 
    {
    case CC1100_INTERNAL_SI_PIN:
      return cc1100->SI_pin;
    case CC1100_INTERNAL_CSn_PIN:
      return cc1100->CSn_pin;
    default:
      return 0;
    }
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_write(int dev_num, uint32_t  mask, uint32_t  value) 
{
  struct _cc1100_t *cc1100 = (struct _cc1100_t *) machine.device[dev_num].data;
  
  if ((mask & CC1100_CSn_MASK) != 0)
    {
      tracer_event_record(TRACER_CC1100_CS, (value & CC1100_CSn_MASK) ? 0 : 1);

      if ((value & CC1100_CSn_MASK) != 0)
	{
	  if (cc1100->CSn_pin == 0) /* CSn == 1, was 0 --> deselect */
	    {
	      CC1100_DBG_PINS ("cc1100:pins: from mcu CSn = 1 **  chip deselected, SO high\n");
	      // desactivated, SO must go highZ, we do this on the 
	      // same cycle
	      cc1100->SO_set = 1;
	      cc1100->SO_pin = 0xff;
	      tracer_event_record(TRACER_CC1100_SO, 1);
	      // will need to change state and go to SLEEP or XOFF 
	      if ((cc1100->fsm_state   == CC1100_STATE_IDLE) && 
		  (cc1100->fsm_pending == CC1100_STATE_SLEEP))
		{
		  CC1100_DBG_STATE("cc1100:state: going from IDLE to SLEEP state\n");
		  CC1100_SLEEP_REALLY_ENTER(cc1100);
		}
	      else if ((cc1100->fsm_state   == CC1100_STATE_IDLE) && 
		       (cc1100->fsm_pending == CC1100_STATE_XOFF))
		{
		  CC1100_DBG_STATE("cc1100:state: going from IDLE to XOFF state\n");
		  CC1100_XOFF_REALLY_ENTER(cc1100);
		}
	    }
	  cc1100->CSn_pin = 0xFF;
	} 
      else 
	{
	  if (cc1100->CSn_pin != 0) /* CSn == 0, was != 0 --> select */
	    {
	      CC1100_DBG_PINS ("cc1100:pins: from mcu CSn = 0 ** chip selected, SO low\n");
	      // activated, SO must go low when crystal is stable, this device is stable
	      // on the same cycle.
	      cc1100->SO_set = 1;
	      cc1100->SO_pin = 0x00;
	      tracer_event_record(TRACER_CC1100_SO, 0);
	    }
	  cc1100->CSn_pin = 0x00;
	}
    }
	
  if ((mask & CC1100_DATA_MASK) != 0) 
    {
      cc1100->SI_pin = value & CC1100_DATA_MASK;
      cc1100->SI_set = 1;
      CC1100_DBG_PINS("cc1100:pins: from mcu SI = 0x%x\n", value & CC1100_DATA_MASK);
    } 
  else 
    {
      cc1100->SI_set = 0;
    }
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_read(int dev_num, uint32_t  *mask, uint32_t  *value) 
{
  struct _cc1100_t *cc1100 = (struct _cc1100_t *) machine.device[dev_num].data;
  
  *mask  = 0;
  *value = 0;

  if (cc1100->SO_set)
    {
      *mask  |= CC1100_SO_MASK;
      *value |= cc1100->SO_pin ? CC1100_SO_MASK : 0;
      CC1100_DBG_PINS("cc1100:pins: to mcu SO = 0x%02x\n", cc1100->SO_pin & 0xff);
      cc1100->SO_set = 0;
    }

  if (cc1100->GO1_set) /* data */
    {
      *mask  |= CC1100_DATA_MASK;
      *value |= cc1100->GO1_pin & 0x00ff;
      CC1100_DBG_PINS("cc1100:pins: to mcu GDO1 = 0x%02x\n", cc1100->GO1_pin & 0xff);
      cc1100->GO1_set = 0;
    }

	
  if (cc1100->GO2_set) 
    {
      *mask  |= CC1100_GDO2_MASK;
      *value |= cc1100->GO2_pin ? CC1100_GDO2_MASK : 0;
      CC1100_DBG_PINS("cc1100:pins: to mcu GDO2 = 0x%02x\n", cc1100->GO2_pin & 0xff);
      cc1100->GO2_set = 0;
    }
	
  if (cc1100->GO0_set) 
    {
      *mask  |= CC1100_GDO0_MASK;
      *value |= cc1100->GO0_pin ? CC1100_GDO0_MASK : 0; 
      CC1100_DBG_PINS("cc1100:pins: to mcu GDO0 = 0x%02x\n", cc1100->GO0_pin & 0xff);
      cc1100->GO0_set = 0;
    }

}


/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_device_create (int dev_num, int fxosc_mhz)
{	
  struct _cc1100_t *cc1100 = (struct _cc1100_t *) machine.device[dev_num].data;

  machine.device[dev_num].reset         = cc1100_reset;
  machine.device[dev_num].delete        = cc1100_delete;
  
  machine.device[dev_num].power_up      = cc1100_power_up;
  machine.device[dev_num].power_down    = cc1100_power_down;
  
  machine.device[dev_num].update        = cc1100_update;
  
  machine.device[dev_num].read          = cc1100_read;
  machine.device[dev_num].write         = cc1100_write;
  
  machine.device[dev_num].state_size    = cc1100_device_size();
  machine.device[dev_num].name          = "cc1100 radio device";

  CC1100_XOSC_FREQ_MHz  = fxosc_mhz;
  CC1100_XOSC_PERIOD_NS = 1000 / fxosc_mhz;

  cc1100->worldsens_radio_id = worldsens_c_rx_register((void*)cc1100, cc1100_callback_rx);

  TRACER_CC1100_STATE  = tracer_event_add_id(8, "state",  "cc1100");
  TRACER_CC1100_STROBE = tracer_event_add_id(8, "strobe", "cc1100");
  TRACER_CC1100_CS     = tracer_event_add_id(1, "cs",     "cc1100");
  TRACER_CC1100_SO     = tracer_event_add_id(1, "so",     "cc1100");
  
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_delete (int UNUSED dev_num) 
{  
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_power_up (int dev_num) 
{  
  return cc1100_reset (dev_num);
}


/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_power_down (int dev_num) 
{  
  return cc1100_reset (dev_num);
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_reset_internal (struct _cc1100_t *cc1100) 
{
  cc1100->fsm_state		= CC1100_STATE_IDLE;
  cc1100->fsm_ustate		= 0;
  cc1100->fsm_timer		= 0;
  cc1100->fsm_pending		= 0;
	
  cc1100->fs_cal		= 0;
	
  cc1100->clk_timeref		= 0;
  cc1100->clk_tick		= 0;
	
  cc1100->tx_io_timer		= 0;
  cc1100->rx_io_timer		= 0;
	
  cc1100_write_pin(cc1100, CC1100_INTERNAL_GO2_PIN, 0x00);
  cc1100_write_pin(cc1100, CC1100_INTERNAL_GO1_PIN, 0x00);
  cc1100_write_pin(cc1100, CC1100_INTERNAL_GO0_PIN, 0x00);
  cc1100->SI_set		= 0;
	
  cc1100->SIType                = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
  cc1100->addr			= 0;
  cc1100->read			= 0;
  
  cc1100->txBytes               = 0;
  cc1100->txOffset		= 0;
  cc1100->txUnderflow		= 0;
  
  cc1100->rxBytes               = 0;
  cc1100->rxOffset		= 0;
  cc1100->rxOverflow		= 0;
   
  cc1100->ioOffset		= 0;	
  cc1100->ioLength		= 0;	
  cc1100->ioCrc			= 0;	
  
  cc1100_reset_registers(cc1100);
  
  tracer_event_record(TRACER_CC1100_STATE, CC1100_STATE_IDLE);
  etracer_slot_event(ETRACER_PER_ID_CC1100,ETRACER_PER_EVT_MODE_CHANGED,ETRACER_CC1100_IDLE,0);
  CC1100_DBG_STATE("cc1100:state: IDLE (internal reset)\n");
  
  cc1100_assert_gdo(cc1100, 0x29, CC1100_PIN_ASSERT);
  return;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_reset (int dev_num) 
{  
  struct _cc1100_t *cc1100 = (struct _cc1100_t *) machine.device[dev_num].data;
  cc1100_reset_internal(cc1100);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

#define ETRACER_SRC(n)  etracer_slot_event(ETRACER_PER_ID_CC1100, ETRACER_PER_EVT_WRITE_COMMAND, \
                                           ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI0, n)

#define ETRACER_DST(n)  etracer_slot_event(ETRACER_PER_ID_CC1100, ETRACER_PER_EVT_WRITE_COMMAND, \
                                           ETRACER_PER_ARG_WR_DST | ETRACER_ACCESS_LVL_SPI0, n)

int cc1100_io_pins (struct _cc1100_t *cc1100) 
{
	
  if (cc1100_read_pin(cc1100, CC1100_INTERNAL_CSn_PIN) == 0xFF) 
    {
      /* No communication with uproc */
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      /* PA Table count reset */
      cc1100->patable_cnt = 0;
      return 0;
    }
	
  if (cc1100->SI_set == 0) 
    {
      /* No communication through SPI */
      return 0;
    }
	
  cc1100->SI_set = 0; /* ack input data */

  if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR) 
    {
      uint8_t SIbyte = cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN);
      uint8_t burst  = CC1100_REG_ACCESS_TYPE(SIbyte);
		
      cc1100->read   = CC1100_REG_ACCESS_OP(SIbyte);    
      cc1100->addr   = CC1100_REG_ACCESS_ADDRESS(SIbyte);

      ETRACER_DST(1);
				
      if ((cc1100->addr >= 0x30) && (cc1100->addr <= 0x3D) &&
	  (burst == CC1100_REG_ACCESS_NOBURST)) 
	{
	  /* Command Strobe (cf [1] p19) */
	  cc1100_strobe_command(cc1100);
	  cc1100_write_status(cc1100);
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Strobe command 0x%x\n", cc1100->addr);
	  return 0;
	}

      if ((cc1100->addr >= 0x30) && (cc1100->addr <= 0x3B) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_READ) &&
	  (burst == CC1100_REG_ACCESS_BURST)) 
	{
	  /* Status register access (cf [1] p19) */
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_NOBURST;
	  cc1100_write_status(cc1100);
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read No Burst RO address %x\n", cc1100->addr);
	  return 0;
	}
		
      if ((cc1100->addr < 0x30) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_READ) &&
	  (burst == CC1100_REG_ACCESS_NOBURST)) 
	{
	  /* Read single byte register access (cf [1] p19) */
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_NOBURST;
	  cc1100_write_status(cc1100);
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read No Burst address %x\n", cc1100->addr);
	  return 0;
	}
		
      if ((cc1100->addr < 0x30) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_READ) &&
	  (burst == CC1100_REG_ACCESS_BURST)) 
	{
	  /* Read burst register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read Burst address %x\n", cc1100->addr);
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		

      if ((cc1100->addr < 0x30) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_WRITE) &&
	  (burst == CC1100_REG_ACCESS_NOBURST)) 
	{
	  /* Write single byte register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write No Burst address %x\n", cc1100->addr);
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_NOBURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
      if ((cc1100->addr < 0x30) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_WRITE) &&
	  (burst == CC1100_REG_ACCESS_BURST)) 
	{
	  /* Write burst register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write Burst address %x\n", cc1100->addr);
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
      if (SIbyte == 0x3F) 
	{
	  /* TX FIFO single byte access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write No Burst TX FIFO\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_NOBURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
      if (SIbyte == 0x7F) 
	{
	  /* TX FIFO burst access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write Burst TX FIFO\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
      if (SIbyte == 0xBF) 
	{
	  /* RX FIFO single byte access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read No Burst RX FIFO\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_NOBURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
      
      if (SIbyte == 0xFF) 
	{
	  /* RX FIFO burst access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read Burst RX FIFO\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
      if ((cc1100->addr == 0x3E) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_READ) &&
	  (burst == CC1100_REG_ACCESS_NOBURST)) 
	{
	  /* Read single byte register access (cf [1] p19) */
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_NOBURST;
	  cc1100_write_status(cc1100);
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read No Burst PA Table\n");
	  return 0;
	}
		
      if ((cc1100->addr == 0x3E) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_READ) &&
	  (burst == CC1100_REG_ACCESS_BURST)) 
	{
	  /* Read burst register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Read Burst PA Table\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
		
      if ((cc1100->addr == 0x3E) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_WRITE) &&
	  (burst == CC1100_REG_ACCESS_NOBURST)) 
	{
	  /* Write single byte register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write No Burst PA Table\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_NOBURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
      
      if ((cc1100->addr == 0x3E) &&
	  (cc1100->read == CC1100_REG_ACCESS_OP_WRITE) &&
	  (burst == CC1100_REG_ACCESS_BURST)) 
	{
	  /* Write burst register access (cf [1] p19) */
	  CC1100_DBG_ACCESS("cc1100: (access DEBUG): Write Burst PA Table\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_BURST;
	  cc1100_write_status(cc1100);
	  return 0;
	}
		
		
      CC1100_DBG_EXC("cc1100: (access EXCEPTION): invalid access address\n");
      cc1100_write_status(cc1100);
      return 0;
      
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_NOBURST) 
    {
      ETRACER_DST(2);
      ETRACER_SRC(2);
      cc1100_spi_output(cc1100, cc1100_read_register(cc1100, cc1100->addr));
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_BURST) 
    {
      ETRACER_DST(3);
      ETRACER_SRC(3);
      cc1100_spi_output(cc1100, cc1100_read_register(cc1100, cc1100->addr));
      if ((++(cc1100->addr)) == 0x30) 
	{
	  CC1100_DBG_IMPL("cc1100: (access IMPLEMENTATION): reached ultimate register addresse in burst\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
	}
      return 0;
    }
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_NOBURST) 
    {
      uint8_t data = cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN);
      ETRACER_DST(4);
      cc1100_write_register(cc1100, cc1100->addr, data);
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      cc1100_write_status(cc1100);
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_BURST) 
    {
      uint8_t data = cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN);
      ETRACER_DST(5);
      cc1100_write_register(cc1100, cc1100->addr, data);
      if ((++(cc1100->addr)) == 0x30) 
	{
	  CC1100_DBG_IMPL("cc1100: (access IMPLEMENTATION): reached ultimate register addresse in burst\n");
	  cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
	}
      cc1100_write_status(cc1100);
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_NOBURST) 
    {
      cc1100_put_tx_fifo(cc1100, cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN));
      /* ETRACER_DST_FIFO */
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      cc1100_write_status(cc1100);
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_BURST) 
    {
      cc1100_put_tx_fifo(cc1100, cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN));
      /* ETRACER_DST_FIFO */
      cc1100_write_status(cc1100);
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_NOBURST) 
    {
      ETRACER_DST(6);
      /* ETRACER_SRC_FIFO */
      cc1100_spi_output(cc1100, cc1100_get_rx_fifo(cc1100));
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_BURST) 
    {
      ETRACER_DST(6);
      /* ETRACER_SRC_FIFO */
      cc1100_spi_output(cc1100, cc1100_get_rx_fifo(cc1100));
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_NOBURST) 
    {
      ETRACER_DST(7);
      ETRACER_SRC(7);
      cc1100_spi_output(cc1100, cc1100->patable[cc1100->patable_cnt]);
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;			
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_BURST) 
    {
      ETRACER_DST(8);
      ETRACER_SRC(8);
      cc1100_spi_output(cc1100, cc1100->patable[cc1100->patable_cnt]);
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;			
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_NOBURST) 
    {
      uint8_t data = cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN);
      ETRACER_DST(9);
      cc1100->patable[cc1100->patable_cnt] = data;
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;			
      cc1100->SIType = CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR;
      cc1100_write_status(cc1100);
      return 0;
    } 
  else if (cc1100->SIType == CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_BURST) 
    {
      uint8_t data = cc1100_read_pin(cc1100, CC1100_INTERNAL_SI_PIN);
      ETRACER_DST(10);
      cc1100->patable[cc1100->patable_cnt] = data;
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;			
      cc1100_write_status(cc1100);
      return 0;
    } 
  else 
    {
      CC1100_DBG_EXC("cc1100: (access EXCEPTION): should never be here\n");
      ETRACER_DST(11);
      cc1100_write_status(cc1100);
      return 0;
    }
  return 0;
}


/***************************************************/
/***************************************************/
/***************************************************/

void cc1100_write_status(struct _cc1100_t *cc1100) 
{
  uint8_t status = 0;

  switch (cc1100->fsm_state) 
    {
    case CC1100_STATE_SLEEP:   /* don't care, STATUS not available in SLEEP mode        */
      status = 0; /* (CC1100_STATUS_IDLE                << 4) & 0xF0;  */
      break;

    case CC1100_STATE_XOFF:    /* don't really care since XOFF is set when CSn goes low */
      status = 0; /* (CC1100_STATUS_IDLE                << 4) & 0xF0;  */
      break;

    case CC1100_STATE_IDLE:
      status = (CC1100_STATUS_IDLE                << 4) & 0xF0;
      break;
    case CC1100_STATE_MANCAL:
    case CC1100_STATE_CALIBRATE:
      status = (CC1100_STATUS_CALIBRATE           << 4) & 0xF0;
      break;
    case CC1100_STATE_TX:
      status = (CC1100_STATUS_TX                  << 4) & 0xF0;
      break;
    case CC1100_STATE_FSTXON:
      status = (CC1100_STATUS_FSTXON              << 4) & 0xF0;
      break;

    case CC1100_STATE_SETTLING:
    case CC1100_STATE_FS_WAKEUP:
    case CC1100_STATE_FS_CALIBRATE:
    case CC1100_STATE_TXRX_SETTLING:
    case CC1100_STATE_RXTX_SETTLING:
      status = (CC1100_STATUS_SETTLING            << 4) & 0xF0;
      break;

    case CC1100_STATE_RX:
      status = (CC1100_STATUS_RX                  << 4) & 0xF0;
      break;
    case CC1100_STATE_RX_OVERFLOW:
      status = (CC1100_STATUS_RXFIFO_OVERFLOW     << 4) & 0xF0;
      break;
    case CC1100_STATE_TX_UNDERFLOW:
      status = (CC1100_STATUS_TXFIFO_UNDERFLOW    << 4) & 0xF0;
      break;
    default:
      CC1100_DBG_IMPL("cc1100:write_status: unspecified value 0x%02x\n", cc1100->fsm_state & 0xff);
      status = 0x00;
      break;
    }
  
  
  if (cc1100->read) 
    {
      if ((cc1100->rxBytes) > 15) 
	{
	  status |= 0x0F;
	} 
      else 
	{
	  status |= (cc1100->rxBytes);
	}
    } 
  else 
    {
      if ((CC1100_TXFIFO_LENGTH - cc1100->txBytes) > 15) 
	{
	  status |= 0x0F;
	} 
      else 
	{
	  status |= (CC1100_TXFIFO_LENGTH - cc1100->txBytes);
	}
    }

  ETRACER_SRC(12);
  cc1100_spi_output(cc1100, status);
}

/***************************************************/
/***************************************************/
/***************************************************/

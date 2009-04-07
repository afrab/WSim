
/**
 *  \file   cc2420_dev.c
 *  \brief  CC2420 device model entry point
 *  \author Nicolas Boulicault
 *  \date   2006
 **/

/*
 *  cc2420_dev.c
 *  
 *
 *  Created by Nicolas Boulicault
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "arch/common/debug.h"

#include "cc2420.h"
#include "cc2420_registers.h"
#include "cc2420_spi.h"
#include "cc2420_fifo.h"
#include "cc2420_ram.h"
#include "cc2420_strobes.h"
#include "cc2420_tx.h"
#include "cc2420_internals.h"
#include "cc2420_macros.h"
#include "cc2420_debug.h"
#include "cc2420_dev.h"

/***************************************************/
/***************************************************/
/***************************************************/

/* Global Variables (not backtracked) */
int  CC2420_XOSC_FREQ_MHz;
int  CC2420_XOSC_PERIOD_NS;

tracer_id_t TRACER_CC2420_STATE;
tracer_id_t TRACER_CC2420_STROBE;
tracer_id_t TRACER_CC2420_CS;

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_device_size(void) 
{
  return sizeof(struct _cc2420_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

/* read a pin. Return 0xFF if high, 0x00 if low */
uint8_t cc2420_read_pin(struct _cc2420_t * cc2420, uint8_t pin) 
{
  switch (pin) 
    {
    case CC2420_INTERNAL_FIFO_PIN    :   return cc2420->FIFO_pin;
    case CC2420_INTERNAL_FIFOP_PIN   :   return cc2420->FIFOP_pin;
    case CC2420_INTERNAL_CCA_PIN     :   return cc2420->CCA_pin;
    case CC2420_INTERNAL_SFD_PIN     :   return cc2420->SFD_pin;
    case CC2420_INTERNAL_SI_PIN      :   return cc2420->SI_pin;
    case CC2420_INTERNAL_SO_PIN      :   return cc2420->SO_pin;
    case CC2420_INTERNAL_CSn_PIN     :   return cc2420->CSn_pin;
    case CC2420_INTERNAL_VREG_EN_PIN :   return cc2420->VREG_EN_pin;
    default :
      return 0;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_device_create (int dev_num, int fxosc_mhz)
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *) machine.device[dev_num].data;

  machine.device[dev_num].reset         = cc2420_reset;
  machine.device[dev_num].delete        = cc2420_delete;

  machine.device[dev_num].power_up      = cc2420_power_up;
  machine.device[dev_num].power_down    = cc2420_power_down;

  machine.device[dev_num].update        = cc2420_update;
  
  machine.device[dev_num].read          = cc2420_read;
  machine.device[dev_num].write         = cc2420_write; 
  
  machine.device[dev_num].state_size    = cc2420_device_size();
  machine.device[dev_num].name          = "cc2420 radio device";

  CC2420_XOSC_FREQ_MHz  = fxosc_mhz;
  CC2420_XOSC_PERIOD_NS = 1000 / fxosc_mhz;

  worldsens_c_rx_register((void*)cc2420,cc2420_callback_rx);

  cc2420->fsm_state = CC2420_STATE_POWER_DOWN;

  TRACER_CC2420_STATE  = tracer_event_add_id(8, "cc2420_state", "cc2420");
  TRACER_CC2420_STROBE = tracer_event_add_id(8, "cc2420_strobe", "cc2420");
  TRACER_CC2420_CS     = tracer_event_add_id(1, "cc2420_cs",     "cc2420");
  

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_delete (int UNUSED dev_num) 
{  
  return 0;
}


/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_power_up(int dev_num) 
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *) machine.device[dev_num].data;
  /* if power is already up, do nothing */
  if (cc2420->fsm_state != CC2420_STATE_VREG_OFF)
    {
      return 0;
    }
  
  /* update state machine timer */
  cc2420->fsm_timer = MACHINE_TIME_GET_NANO() + CC2420_VREG_STARTUP_TIME;
  
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_power_down(int dev_num)
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *) machine.device[dev_num].data; 
  CC2420_POWER_DOWN_ENTER(cc2420);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_reset_pins(struct _cc2420_t * cc2420) 
{
    cc2420->SFD_pin   = 0x00;
    cc2420->CCA_pin   = 0x00;
    cc2420->FIFOP_pin = 0x00;
    cc2420->FIFO_pin  = 0x00;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_reset_internal(struct _cc2420_t * cc2420) 
{
  cc2420_reset_registers(cc2420);                           /* reset internal registers   */
  cc2420_reset_pins(cc2420);                                /* reset output pins          */
  cc2420->SI_type       = CC2420_SPI_NEXT_ACCESS_TYPE_ADDR; /* reset next SPI access type */
  cc2420->xosc_stable   = 0;                                /* set xosc state to unstable */
  cc2420->tx_active     = 0;                                /* set tx active to false     */
  cc2420->pll_locked    = 0;                                /* set pll state to unlocked    */
  cc2420->encoding_busy = 0;                                /* set encoding module state  */
                                                            /*     to idle */

    /* update internal rx / tx variables */

  cc2420->tx_frame_pending    = 0;
  cc2420->tx_preamble_symbols = cc2420_tx_preamble_symbols(cc2420);
  cc2420->tx_fifo_len         = 0;
  cc2420->tx_underflow        = 0;
  cc2420->tx_needed_bytes     = 0;
  cc2420->tx_available_bytes  = 0;
  cc2420->rx_first_data_byte  = -1;
  cc2420->nb_rx_frames        = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_reset(int dev_num) 
{
  struct _cc2420_t * cc2420 = (struct _cc2420_t *) machine.device[dev_num].data;
  cc2420_reset_internal(cc2420);
  CC2420_VREG_OFF_ENTER(cc2420);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_write_status(struct _cc2420_t * cc2420) 
{
    cc2420->status_byte = 0;

    if (cc2420->rx_rssi_valid) {
	cc2420->status_byte |= (1 << 1);
    }

    if (cc2420->pll_locked) {
	cc2420->status_byte |= (1 << 2);
    }

    /* tx active */
    if (cc2420->tx_active) {
	cc2420->status_byte |= (1 << 3);
    }

    /* encoding module mode */
    /* never active since security is not implemented */
    if (cc2420->encoding_busy == 1) {
	cc2420->status_byte |= (1 << 4);
    }
	 
    if (cc2420->tx_underflow) {
	cc2420->status_byte |= (1 << 5);
    }

    if (cc2420->xosc_stable) {
	cc2420->status_byte |= (1 << 6);
    }

    cc2420_spi_output(cc2420, cc2420->status_byte);
}

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_io_pins(struct _cc2420_t * cc2420) 
{

    if (cc2420_read_pin(cc2420, CC2420_INTERNAL_CSn_PIN) == 0xFF) 
      {
	/* CSn not active */
	cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_ADDR;
	/* TODO:: check PA table count reset */
	return 0;
      }

    if (cc2420->SI_set == 0) 
      {
	/* no com through SPI */
	return 0;
      }


    cc2420->SI_set = 0; /* ack input data */
    /*
     * CC2420_SPI_NEXT_ACCESS_TYPE_ADDR
     *
     * deal with case where nothing happened on the SPI bus
     * we're waiting for an address byte
     * we're in the "address cycle"
     */

    if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_ADDR) 
      {
	uint8_t read_access  = 0;
	/* store the bit indicating whether we're accessing RAM or REGISTER */
	uint8_t ram_access   = CC2420_SPI_ACCESS_ZONE(cc2420->SI_pin);
	
	/* store the address :: register addresses are on 6 bits, RAM addresses on 7 bits */
	if (ram_access) 
	  {
	    cc2420->SPI_addr = CC2420_SPI_ACCESS_RAM_ADDR(cc2420->SI_pin);
	  }
	else 
	  {
	    cc2420->SPI_addr = CC2420_SPI_ACCESS_REG_ADDR(cc2420->SI_pin);
	    /* store the bit indicating if it's a read / write access */
	    read_access      = CC2420_SPI_ACCESS_RW(cc2420->SI_pin);
	  }
	

	if      ( (!ram_access) && (!read_access) && (cc2420->SPI_addr <= 0x0E) ) 
	  { 
	    /* writing a command strobe, address cycle (the only cycle for strobes) */
	    CC2420_DBG_ACCESS("cc2420:access: write address %x == Strobe\n", cc2420->SPI_addr & 0xff);
	    cc2420_strobe_command(cc2420);
	    cc2420_write_status(cc2420);
	    return 0;
	  }
	else if  ( (!ram_access) && (!read_access) && (cc2420->SPI_addr >= 0x10) && (cc2420->SPI_addr <= 0x30) ) 
	  {
	    /* writing to a conf register, address cycle */ /* wait for next two bytes */
	    CC2420_DBG_ACCESS("cc2420:access: write address %x == REG\n", cc2420->SPI_addr & 0xff);
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE1; 
	    cc2420_write_status(cc2420);
	    return 0;
	  }
	else if ( (!ram_access) && (!read_access) && (cc2420->SPI_addr == 0x3E) ) 
	  {
	    /* writing to the TX FIFO, address cycle */
	    CC2420_DBG_ACCESS("cc2420:access: write address %x == TXFIFO\n", cc2420->SPI_addr & 0xff);
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_TXFIFO_WRITE;
	    cc2420_write_status(cc2420);
	    return 0;
	}
	else if ( (!ram_access) && (read_access) && (cc2420->SPI_addr >= 0x10) && (cc2420->SPI_addr <= 0x30) ) 
	  {
	    /* reading a conf register, address cycle */
	    CC2420_DBG_ACCESS("cc2420:access: read address %x == REG\n", cc2420->SPI_addr & 0xff);
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE1;
	    cc2420_write_status(cc2420);
	    return 0;
	  }
	else if ( (!ram_access) && (read_access) && (cc2420->SPI_addr == 0x3F) ) 
	  {
	    /* reading RX FIFO, address cycle */
	    CC2420_DBG_ACCESS("cc2420:access: read address %x == RXFIFO\n", cc2420->SPI_addr & 0xff);
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_RXFIFO_READ;
	    cc2420_write_status(cc2420);
	    return 0;
	  }
	else if (ram_access) 
	  {
	    /* RAM, 1st address cycle (still don't know if it's read or write) */
	    CC2420_DBG_ACCESS("cc2420:access: write address %x == RAM\n", cc2420->SPI_addr & 0xff);
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_RAM_BANK_SELECT;
	    cc2420_write_status(cc2420);
	    return 0;
	  }
      }
    /* end of CC2420_SPI_NEXT_ACCESS_TYPE_ADDR */

    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE1) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE1
	 * we're writing to a register, get the first byte (registers are 16 bits long)
	 */
	CC2420_DBG_ACCESS("cc2420:access: write address 0x%x / REG byte1 = 0x%02x\n", 
			  cc2420->SPI_addr & 0xff, cc2420->SI_pin & 0xff);
	cc2420->SI_byte1 = cc2420->SI_pin;
	cc2420->SI_type  = CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE2;
	cc2420_spi_output(cc2420, 0x00); /*Send a dummy byte to the SPI. Value got by experimental test on hardware*/
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE2) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE2
	 * writing to a register, second byte
	 */
	CC2420_DBG_ACCESS("cc2420:access: write address 0x%x / REG byte2 = 0x%02x\n", 
			  cc2420->SPI_addr & 0xff, cc2420->SI_pin & 0xff);
	cc2420_write_register_h(cc2420, cc2420->SPI_addr,cc2420->SI_byte1);
	cc2420_write_register_l(cc2420, cc2420->SPI_addr,cc2420->SI_pin);
	cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_ADDR;
	cc2420_spi_output(cc2420, 0x00); /*Send a dummy byte to the SPI. Value got by experimental test on hardware*/ 
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE1) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE1
	 * reading a register, 1st byte: output the high part
	 */
	CC2420_DBG_ACCESS("cc2420:access: read address %x == REG byte 1\n", cc2420->SPI_addr & 0xff);
	cc2420_spi_output(cc2420, cc2420_read_register_h(cc2420, cc2420->SPI_addr));
	cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE2;
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE2) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE2
	 * reading a register, 2nd byte : output the low part
	 */
	CC2420_DBG_ACCESS("cc2420:access: read address %x == REG byte 2\n", cc2420->SPI_addr & 0xff);
	cc2420_spi_output(cc2420, cc2420_read_register_l(cc2420, cc2420->SPI_addr));
	cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_ADDR;
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_TXFIFO_WRITE) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_TXFIFO_WRITE
	 * writing a single byte into TXFIFO
	 * leave access type unchanged, multiple access
	 * increment SPI address
	 */
	CC2420_DBG_ACCESS("cc2420:access: TXFIFO write address %x\n", cc2420->SPI_addr & 0xff);
	cc2420_tx_fifo_write(cc2420, cc2420->SI_pin);
	cc2420_write_status(cc2420);
	cc2420->SPI_addr++;
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_RXFIFO_READ) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_RXFIFO_READ
	 * leave access type unchanged, multiple access
	 * increment SPI addr
	 */
	uint8_t val = 0;
	CC2420_DBG_ACCESS("cc2420:access: RXFIFO read address %x\n", cc2420->SPI_addr & 0xff);
	if (cc2420_rx_fifo_pop(cc2420, &val) < 0 ) 
	  {
	    CC2420_DEBUG("cc2420_io_pins : can't read, RX FIFO is empty\n");
	  }
	cc2420_spi_output(cc2420, val);
	cc2420->SPI_addr++;
	
	/* if no more data in RX FIFO, reset FIFO pin to 0 (cf [1] p. 33) */
	if (cc2420->rx_fifo_read == cc2420->rx_fifo_write) 
	  {
	    cc2420->FIFO_pin = 0x00;
	    cc2420->FIFO_set = 1;
	  }
	
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_RAM_BANK_SELECT) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_RAM_BANK_SELECT
	 * we'll get the bank and the R/W flag
	 */
	CC2420_DBG_ACCESS("cc2420:access: RAM Bank Select address %x\n", cc2420->SPI_addr & 0xff);
	cc2420->ram_bank = CC2420_RAM_BANK(cc2420->SI_pin);
	if (CC2420_RAM_READ_ACCESS(cc2420->SI_pin)) 
	  {
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_RAM_READ_BYTE;
	    return 0;
	  }
	else 
	  {
	    cc2420->SI_type = CC2420_SPI_NEXT_ACCESS_TYPE_RAM_WRITE_BYTE;
	    return 0;
	  }
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_RAM_READ_BYTE) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_RAM_READ_BYTE
	 * read a byte from RAM
	 * don't change the access type, but increment SPI address
	 */
	CC2420_DBG_ACCESS("cc2420:access: RAM read byte address %x\n", cc2420->SPI_addr & 0xff);
	cc2420_spi_output(cc2420, cc2420_ram_read_byte(cc2420, cc2420->ram_bank, cc2420->SPI_addr));
	cc2420->SPI_addr++;
	return 0;
      }
    else if (cc2420->SI_type == CC2420_SPI_NEXT_ACCESS_TYPE_RAM_WRITE_BYTE) 
      {
	/* CC2420_SPI_NEXT_ACCESS_TYPE_RAM_WRITE_BYTE
	 * write a byte to RAM
	 * don't change the access type, but increment SPI address
	 */
	CC2420_DBG_ACCESS("cc2420:access: RAM write byte address %x\n", cc2420->SPI_addr & 0xff);
	cc2420_spi_output(cc2420, cc2420_ram_write_byte(cc2420, cc2420->ram_bank, cc2420->SPI_addr, cc2420->SI_pin));
	cc2420->SPI_addr++;
	return 0;
      }

    /* *** SHOULD NEVER BE THERE *** */
    CC2420_DEBUG("cc2420_io_pins : BUG, should not be there\n");
    return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_read(int  dev_num, uint32_t  *mask, uint32_t  *value)
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *)machine.device[dev_num].data;
  
  *mask  = 0;
  *value = 0;

  /* used to check polarities (cf [1] p. 73) */
  uint16_t pol = 0;
  /* "real" output value */
  uint8_t hw_val;


  /* output data on SPI */
  if (cc2420->CSn_pin == 0x00) 
    {
      if (cc2420->SO_set) 
	{
	  *mask         |= CC2420_DATA_MASK;
	  *value        |= cc2420->SO_pin;
	  cc2420->SO_set = 0;
	  CC2420_DBG_PINS("cc2420:pins:read: data 0x%02x\n",*value);
	}
    }


  if (cc2420->FIFOP_set) 
    {
      *mask            |= CC2420_FIFOP_MASK;
      pol               = CC2420_REG_IOCFG0_FIFOP_POLARITY(cc2420->registers[CC2420_REG_IOCFG0]);
      hw_val            = (pol) ? !cc2420->FIFOP_pin : cc2420->FIFOP_pin;
      cc2420->FIFOP_set = 0;
      if (hw_val) 
	{
	  *value |= CC2420_FIFOP_MASK;
	  CC2420_DBG_PINS("cc2420:pins:read: setting FIFOP pin\n");
	}
      else 
	{
	  *value &= ~CC2420_FIFOP_MASK;
	  CC2420_DEBUG("cc2420:pins:read: unsetting FIFOP pin\n");
	}
    }


  if (cc2420->FIFO_set) 
    {
      *mask            |= CC2420_FIFO_MASK;
      pol               = CC2420_REG_IOCFG0_FIFO_POLARITY(cc2420->registers[CC2420_REG_IOCFG0]);
      hw_val            = (pol) ? !cc2420->FIFO_pin : cc2420->FIFO_pin;
      cc2420->FIFO_set  = 0;
      if (hw_val) 
	{
	  *value |= CC2420_FIFO_MASK;	    
	  CC2420_DBG_PINS("cc2420:pins:read: setting FIFO pin\n");
	}
      else {
	*value &= ~CC2420_FIFO_MASK;
	CC2420_DBG_PINS("cc2420:pins:read: unsetting FIFO pin\n");
      }
    }


    if (cc2420->CCA_set) 
      {
	*mask          |= CC2420_CCA_MASK;
	pol             = CC2420_REG_IOCFG0_CCA_POLARITY(cc2420->registers[CC2420_REG_IOCFG0]);
	hw_val          = (pol) ? !cc2420->CCA_pin : cc2420->CCA_pin;
	cc2420->CCA_set = 0;
	if (hw_val) 
	  {
	    *value |= CC2420_CCA_MASK;	    
	    CC2420_DBG_PINS("cc2420:pins:read: setting CCA pin\n");
	  }
	else 
	  {
	    *value &= ~CC2420_CCA_MASK;
	    CC2420_DBG_PINS("cc2420:pins:read: unsetting CCA pin\n");
	  }
      }


    if (cc2420->SFD_set) 
      {
	*mask          |= CC2420_SFD_MASK;
	pol             = CC2420_REG_IOCFG0_SFD_POLARITY(cc2420->registers[CC2420_REG_IOCFG0]);
	hw_val          = (pol) ? !cc2420->SFD_pin : cc2420->SFD_pin;
	cc2420->SFD_set = 0;
	if (hw_val) 
	  {
	    *value |= CC2420_SFD_MASK;
	    CC2420_DBG_PINS("cc2420:pins:read: setting SFD pin\n");
	  }
	else 
	  {
	    *value &= ~CC2420_SFD_MASK;
	    CC2420_DBG_PINS("cc2420:pins:read: unsetting SFD pin\n");
	  }
      }
}

/***************************************************/
/***************************************************/
/***************************************************/

void cc2420_write(int dev_num, uint32_t mask, uint32_t value)
{
  struct _cc2420_t *cc2420 = (struct _cc2420_t *)machine.device[dev_num].data;

  if ((mask & CC2420_CSn_MASK) != 0)
    {
      if ((value & CC2420_CSn_MASK) != 0)
	{
	  cc2420->CSn_pin = 0xFF;
	  tracer_event_record(TRACER_CC2420_CS, 1);
	  CC2420_DBG_PINS("cc2420:pins:write: from mcu CSn = 1\n");
	}
      else 
	{
	  if (cc2420->CSn_pin)
	    {
	      CC2420_DBG_PINS("cc2420:pins:write: from mcu CSn = 0\n");
	    }
	  cc2420->CSn_pin = 0x00;
	  tracer_event_record(TRACER_CC2420_CS, 0);
	}
    }


  if ((mask & CC2420_DATA_MASK) != 0) 
    {
      cc2420->SI_pin = value & CC2420_DATA_MASK;
      cc2420->SI_set = 1;
      CC2420_DBG_PINS("cc2420:pins:write: from mcu SI = 0x%x\n", value & CC2420_DATA_MASK);
    }
  else
    {
      cc2420->SI_set = 0;
    }


  if (mask & CC2420_VREG_EN_MASK) 
    {
      if (value & CC2420_VREG_EN_MASK)
	{
	  cc2420->VREG_EN_pin = 0xFF;
	  CC2420_DBG_PINS("cc2420:pins:write: from mcu VREG_EN = 1\n");
	}
      else 
	{
	  if (cc2420->VREG_EN_pin)
	    {
	      CC2420_DBG_PINS("cc2420:pins:write: from mcu VREG_EN = 1\n");
	    }
	  cc2420->VREG_EN_pin = 0x00;
	}
      cc2420->VREG_EN_set = 1;
    }


    if (mask & CC2420_RESET_MASK) 
      {
	if (value & CC2420_RESET_MASK)
	  {
	    cc2420->RESET_pin = 0xFF;
	    CC2420_DBG_PINS("cc2420:pins:write: from mcu RESET = 1\n");
	  }
	else 
	  {
	    if (cc2420->RESET_pin)
	      {
		CC2420_DBG_PINS("cc2420:pins:write: from mcu RESET = 0\n");
	      }
	    cc2420->RESET_pin = 0x00;
	  }
	cc2420->RESET_set = 1;
    }


  if (mask & CC2420_FIFO_MASK) 
    {
      WARNING("cc2420:pins:write: FIFO imposed from mcu.\n");
    }

  if (mask & CC2420_FIFOP_MASK) 
    {
      WARNING("cc2420:pins:write: FIFOP imposed from mcu.\n");
    }

  if (mask & CC2420_CCA_MASK) 
    {
      WARNING("cc2420:pins:write: CCA imposed from mcu.\n");
    }

  if (mask & CC2420_SFD_MASK) 
    {
      WARNING("cc2420:pins:write: SFD imposed from mcu.\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

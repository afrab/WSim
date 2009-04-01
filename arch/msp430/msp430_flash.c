/**
 *  \file   msp430_flash.c
 *  \brief  MSP430 Flash controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>

#include "arch/common/hardware.h"
#include "msp430.h"


/***********************************************************/
/* THIS MODEL IS NOT COMPLETE                              */
/* most of the parts are done and should behave correctly  */
/* if the flash controller is used with a correct software */
/***********************************************************/

#define F2L 2 /* log level */


#if defined(__msp430_have_flash)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* 
 * The information memory has two 128-byte segments (MSP430F1101 devices
 * have only one). The main memory has two or more 512-byte segments. See
 * the device-specific datasheet for the complete memory map of a device.
 * 
 * The segments are further divided into blocks. A block is 64 bytes, starting at
 * 0xx00h, 0xx40h, 0xx80h, or 0xxC0h, and ending at 0xx3Fh, 0xx7Fh, 0xxBFh,
 * or 0xxFFh.
 */

/*
 * The flash timing generator operating frequency, f(FTG), must be
 * in the range from ~ 257 kHz to ~ 476 kHz (see device-specific datasheet).
 */

#define MCUFLASH MCU.flash

static void msp430_flash_update_wait(void);
void (*msp430_flash_update_ptr)(void);
void msp430_flash_freq_eval(void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_reset()
{
  MCUFLASH.fctl1.s = 0x09600;
  MCUFLASH.fctl2.s = 0x09642;
  MCUFLASH.fctl3.s = 0x09618;
  msp430_flash_freq_eval();
  msp430_flash_update_ptr = NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_flash_update_wait(void)
{
  /* busy is set when a dummy write has been done in the flash */
  if (MCUFLASH.fctl3.b.busy == 1)
    {
      if (1 /* && timeout */)
	{
	  VERBOSE(F2L,"msp430:flash: Start erase\n");
	  msp430_io_set_flash_read_normal (ADDR_FLASH_START, ADDR_FLASH_STOP);
	  msp430_io_set_flash_write_normal(ADDR_FLASH_START, ADDR_FLASH_STOP);
	  msp430_flash_update_ptr = NULL;
	  MCUFLASH.fctl3.b.busy   = 0;
	}
      else
	{
	  /* waiting, decrease time to wait */
	}
    }
  else
    {
      /* we should not be here */
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_flash_read  (uint16_t addr)
{
  VERBOSE(F2L,"msp430:flash: read at address 0x%04x\n",addr);
  switch (addr)
    {
    case FLASH_FCTL1:
      return MCUFLASH.fctl1.s;
    case FLASH_FCTL2:
      return MCUFLASH.fctl2.s;
    case FLASH_FCTL3:
      return MCUFLASH.fctl3.s;
    default:
      ERROR("msp430:flash: bad read address 0x%04x\n",addr);
      break;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_write (uint16_t addr, int16_t val)
{
  /* use mcu_ramctrl_swrite during programming */

  VERBOSE(F2L,"msp430:flash: write at address 0x%04x val 0x%04x\n",addr, val);

  if (((val >> 8) & 0xff) != 0xA5)
    {
      ERROR("msp430:flash: ===========================================================\n");
      ERROR("msp430:flash: ==== THIS SHOULD TRIGGER A PUC ON A REAL PLATFORM !!!! ====\n");
      ERROR("msp430:flash: ==== bad KEY value, val = 0x%04x, (should be 0xa5)     ====\n");
      ERROR("msp430:flash: ===========================================================\n");
      MCUFLASH.fctl3.b.keyv = 1;
      return ;
    }
  
  switch (addr)
    {
      /* ======================= */
      /* ===== FLASH_FCTL1 ===== */
      /* ======================= */
    case FLASH_FCTL1:
      {
	union {
	  struct fctl1_t      b;
	  uint16_t            s;
	} fctl1;
	fctl1.s = val;

	if (MCUFLASH.fctl3.b.busy == 1)
	  {
	    ERROR("msp430:flash: write to controller FCTL1 while busy\n");
	    /* more tests needed here, details section 5.3.6 */
	    MCUFLASH.fctl3.b.accvifg = 1;
	    return ;
	  }

	if ((MCUFLASH.fctl1.b.ERASE != fctl1.b.ERASE) ||
	    (MCUFLASH.fctl1.b.MERAS != fctl1.b.MERAS))
	  {
	    /* start to wait the dummy write or clear */
	    HW_DMSG("msp430:flash: ERASE %d and MERAS %d bits modified\n",
		    fctl1.b.ERASE, fctl1.b.MERAS);
	  }
	
	if ((fctl1.b.ERASE == 1) || (fctl1.b.MERAS == 1))
	  {
	    /* start to wait the dummy write or clear */
	    VERBOSE(F2L,"msp430:flash: Erase prepared, waiting for dummy write\n");
	    msp430_io_set_flash_write_start_erase(ADDR_FLASH_START, ADDR_FLASH_STOP);
	  }

	MCUFLASH.fctl1.s = 0x09600 | (val & 0xff);
      }
      break;

      /* ======================= */
      /* ===== FLASH_FCTL2 ===== */
      /* ======================= */
    case FLASH_FCTL2:
      {
	union {
	  struct fctl2_t      b;
	  uint16_t            s;
	} fctl2;
	fctl2.s = val;

	if (MCUFLASH.fctl3.b.busy == 1)
	  {
	    ERROR("msp430:flash: write to controller FCTL2 while busy\n");
	    MCUFLASH.fctl3.b.accvifg = 1;
	    return ;
	  }

	if (MCUFLASH.fctl2.b.fnx != fctl2.b.fnx)
	  {
	    msp430_flash_freq_eval();
	  }
	if (MCUFLASH.fctl2.b.fsselx != fctl2.b.fsselx)
	  {
	    msp430_flash_freq_eval();
	  }
	MCUFLASH.fctl2.s = 0x09600 | (val & 0xff);
      }
      break;

      /* ======================= */
      /* ===== FLASH_FCTL3 ===== */
      /* ======================= */
    case FLASH_FCTL3:
      {
	union {
	  struct fctl3_t      b;
	  uint16_t            s;
	} fctl3;
	fctl3.s = val;

	if (fctl3.b.EMEX == 1)
	  {
	    MCUFLASH.fctl1.b.ERASE = 0;
	    MCUFLASH.fctl1.b.MERAS = 0;
	  }

	MCUFLASH.fctl3.s = 0x09600 | (val & 0xff);;
      }
      break;

      /* ======================= */
      /* ======================= */
      /* ======================= */
    default:
      ERROR("msp430:flash: bad write address 0x%04x val 0x%04x\n",addr,val);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

// Flash erase timings
// FLASH_ERASE_TIMING_MASS  
// FLASH_ERASE_TIMING_SEG   

void msp430_flash_freq_eval(void)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_start_erase (uint16_t addr)
{
  VERBOSE(F2L,"msp430:flash: start erase dummy write at 0x%04x\n",addr);
  msp430_io_set_flash_read_jump_pc(ADDR_FLASH_START, ADDR_FLASH_STOP);
  /* memset(region, 0xff, sizeof(region[addr])); */
  MCUFLASH.fctl3.b.busy   = 1;
  msp430_flash_update_ptr = msp430_flash_update_wait;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_flash_chkifg(void)
{
  int ret = 0;

  if (MCUFLASH.fctl3.b.keyv == 1)
    {
      ERROR("msp430:flash: =========================================\n");
      ERROR("msp430:flash: === KEYV set to 1, MSP430 reset (PUC) ===\n");
      ERROR("msp430:flash: === execution is unpredictable        ===\n");
      ERROR("msp430:flash: =========================================\n");
      mcu_reset();
      return 1;
    }

  if (MCU.sfr.ie1.b.accvie && MCUFLASH.fctl3.b.accvifg)
    {
      msp430_interrupt_set(INTR_NMI);
      ret = 1;
    }
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif

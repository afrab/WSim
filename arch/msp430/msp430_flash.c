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

void   msp430_flash_update_wait (void);
void (*msp430_flash_update_ptr) (void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_create(void)
{
  msp430_io_register_range16(FLASHCTL_START,FLASHCTL_END,msp430_flash_read,msp430_flash_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_reset(void)
{
  MCUFLASH.fctl1.s          = 0x09600;
  MCUFLASH.fctl2.s          = 0x09642;
  MCUFLASH.fctl3.s          = 0x09618;
  MCUFLASH.ticks_divider    = 0;
  MCUFLASH.flash_ticks_left = 0;
  MCUFLASH.flash_write_fst  = 0;
  msp430_flash_update_ptr   = NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_update_wait(void)
{
  /* busy is set when a dummy write has been done in the flash */
  if (MCUFLASH.fctl3.b.busy == 1)
    {
      if (MCUFLASH.flash_ticks_left <= 0) /* == timeout */
	{
	  HW_DMSG_FLASH("msp430:flash: == operation done, busy == 0\n");
	  msp430_io_set_flash_read_normal (ADDR_FLASH_START, ADDR_FLASH_STOP);
	  msp430_flash_update_ptr = NULL;
	  MCUFLASH.fctl3.b.busy   = 0;	
	  MCUFLASH.fctl3.b.wait   = 1;
	}
      else
	{
	  /* waiting, decrease time to wait */
	  uint32_t clock = 0;
	  uint32_t div, mod;
	  switch (MCUFLASH.fctl2.b.fsselx)
	    {
	    case 0:
	      clock = MCU_CLOCK.ACLK_increment;
	      break;
	    case 1:
	      clock = MCU_CLOCK.MCLK_increment;
	      break;
	    case 2: /* fall */
	    case 3: 
	      clock = MCU_CLOCK.SMCLK_increment;
	      break; 
	    }
	  MCUFLASH.ticks_divider    += clock;
	  div = MCUFLASH.ticks_divider / (MCUFLASH.fctl2.b.fnx + 1); 
	  mod = MCUFLASH.ticks_divider % (MCUFLASH.fctl2.b.fnx + 1);
	  MCUFLASH.ticks_divider     = mod;
	  MCUFLASH.flash_ticks_left -= div;
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
  switch (addr)
    {
    case FLASH_FCTL1:
      HW_DMSG_FLASH("msp430:flash: read  at FCTL1 val 0x%04x\n", MCUFLASH.fctl1.s);
      return MCUFLASH.fctl1.s;
    case FLASH_FCTL2:
      HW_DMSG_FLASH("msp430:flash: read  at FCTL2 val 0x%04x\n", MCUFLASH.fctl2.s);
      return MCUFLASH.fctl2.s;
    case FLASH_FCTL3:
      // read on busy flag
      // HW_DMSG_FLASH("msp430:flash: read  at FCTL3 val 0x%04x\n", MCUFLASH.fctl3.s);
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

	HW_DMSG_FLASH("msp430:flash: write at FCTL1 val 0x%04x\n",val & 0xffff);

	if (MCUFLASH.fctl3.b.busy == 1)
	  {
	    ERROR("msp430:flash: write to controller FCTL1 while busy\n");
	    /* more tests needed here, details section 5.3.6 */
	    MCUFLASH.fctl3.b.accvifg = 1;
	    return ;
	  }

	if ((MCUFLASH.fctl1.b.blkwrt != fctl1.b.blkwrt) ||
	    (MCUFLASH.fctl1.b.wrt    != fctl1.b.wrt))
	  {
	    HW_DMSG_FLASH("msp430:flash:    -- (blkwrt,wrt) bits modified (%d,%d)\n",
		    fctl1.b.blkwrt, fctl1.b.wrt);
	    MCUFLASH.flash_write_fst  = 0;
	    switch ((fctl1.b.blkwrt << 1) | fctl1.b.wrt)
	      {
	      case 0:
		HW_DMSG_FLASH("msp430:flash:    -- no write\n");
		break;
	      case 1:
		HW_DMSG_FLASH("msp430:flash:    -- wrt == 1, write bit/byte\n");
		break;
	      case 2:
		HW_DMSG_FLASH("msp430:flash:    -- wrtblk == 1 && wrt == 0, waiting wrt = 1\n");
		break;
	      case 3:
		HW_DMSG_FLASH("msp430:flash:    -- block write prepared\n");
		break;
	      }
	  }

	if ((MCUFLASH.fctl1.b.ERASE != fctl1.b.ERASE) ||
	    (MCUFLASH.fctl1.b.MERAS != fctl1.b.MERAS))
	  {
	    /* start to wait the dummy write or clear */
	    HW_DMSG_FLASH("msp430:flash:    -- ERASE and MERAS bits modified (%d,%d)\n",
		    fctl1.b.ERASE, fctl1.b.MERAS);
	    switch ((fctl1.b.MERAS << 1) | fctl1.b.ERASE)
	      {
	      case 0:
		HW_DMSG_FLASH("msp430:flash:    -- no erase\n");
		break;
	      case 1:
		HW_DMSG_FLASH("msp430:flash:    -- erase individual segment only\n");
		break;
	      case 2:
		HW_DMSG_FLASH("msp430:flash:    -- erase all main memory segments\n");
		break;
	      case 3:
		HW_DMSG_FLASH("msp430:flash:    -- erase all main and information memory segments\n");
		break;
	      }
	  }
	
	if ((fctl1.b.ERASE == 1) || (fctl1.b.MERAS == 1) || 
	    (fctl1.b.wrt == 1) || (fctl1.b.blkwrt == 1))
	  {
	    /* start to wait the dummy write or clear */
	    HW_DMSG_FLASH("msp430:flash:    -- write/erase prepared, waiting for dummy write\n");
	    msp430_io_set_flash_write_start_erase(ADDR_FLASH_START, ADDR_FLASH_STOP);
	    msp430_io_set_flash_write_start_erase(ADDR_NVM_START,   ADDR_NVM_STOP  );
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

	HW_DMSG_FLASH("msp430:flash: write at FCTL2 val 0x%04x\n",val & 0xffff);

	if (MCUFLASH.fctl3.b.busy == 1)
	  {
	    ERROR("msp430:flash: write to controller FCTL2 while busy\n");
	    MCUFLASH.fctl3.b.accvifg = 1;
	    return ;
	  }

	if (MCUFLASH.fctl2.b.fnx != fctl2.b.fnx)
	  {
	    HW_DMSG_FLASH("msp430:flash: clock divider set to %d (%d+1)\n",
		    fctl2.b.fnx + 1, fctl2.b.fnx);
	  }
	if (MCUFLASH.fctl2.b.fsselx != fctl2.b.fsselx)
	  {
	    switch (fctl2.b.fsselx)
	      {
	      case 0:
		HW_DMSG_FLASH("msp430:flash: clock source from ACLK\n");
		break;
	      case 1:
		HW_DMSG_FLASH("msp430:flash: clock source from MCLK\n");
		break;
	      case 2:
		HW_DMSG_FLASH("msp430:flash: clock source from SMCLK\n");
		break; 
	      case 3: 
		HW_DMSG_FLASH("msp430:flash: clock source from SMCLK\n");
		break; 
	      }
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

	HW_DMSG_FLASH("msp430:flash: write at FCTL3 val 0x%04x\n",val & 0xffff);

	if (fctl3.b.EMEX == 1)
	  {
	    HW_DMSG_FLASH("msp430:flash:    EMEX bit is set to 1, emergency exit\n");
	    msp430_io_set_flash_write_normal(ADDR_FLASH_START, ADDR_FLASH_STOP);
	    msp430_io_set_flash_write_normal(ADDR_NVM_START,   ADDR_NVM_STOP  );
	    msp430_flash_reset();
	  }
	else
	  {
	    if (MCUFLASH.fctl3.b.lock != fctl3.b.lock)
	      {
		HW_DMSG_FLASH("msp430:flash:   lock bit set to %d\n",fctl3.b.lock);
		if (fctl3.b.lock == 1)
		  {
		    msp430_io_set_flash_write_normal(ADDR_FLASH_START, ADDR_FLASH_STOP);
		    msp430_io_set_flash_write_normal(ADDR_NVM_START,   ADDR_NVM_STOP  );
		    msp430_flash_update_ptr = NULL;
		  }
	      }

	    // remove wait and busy from val
	    val &= 0xf6;
	    MCUFLASH.fctl3.s = 0x09600 | (val & 0xff);;
	  }
      }
      break;

      /* ======================= */
      /* ======================= */
      /* ======================= */
    default:
      ERROR("msp430:flash: bad write address 0x%04x val 0x%04x\n",addr,val & 0xffff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_flash_start_erase (uint16_t u16addr, int size, uint32_t val)
{
  uint32_t addr;
  uint32_t wptr;
  uint32_t region_start = 1;
  uint32_t region_stop  = 0;

  addr = u16addr;

  switch (size)
    {
    case 1: 
      HW_DMSG_FLASH("msp430:flash: == start erase dummy write8  at [0x%04x] = 0x%02x\n",addr,val & 0xff);
      break;
    case 2:
      HW_DMSG_FLASH("msp430:flash: == start erase dummy write16 at [0x%04x] = 0x%04x\n",addr,val & 0xffff);
      break;
    default:
      HW_DMSG_FLASH("msp430:flash: == start erase dummy write16 at [0x%04x] = 0x%04x\n",addr,val & 0xffff);
      break;
    }

  if (MCUFLASH.fctl1.b.wrt == 1)
    {
      if (MCUFLASH.fctl1.b.blkwrt == 1)
	{
	  if (MCUFLASH.flash_write_fst == 0)
	    {
	      HW_DMSG_FLASH("msp430:flash:   write block [0x%04x] = 0x%04x (first byte, %d)\n",
		      u16addr&0xffff,val&0xffff,size);
	      MCUFLASH.flash_ticks_left = FLASH_WRITE_TIMING_FSTBYTE;
	      MCUFLASH.flash_write_fst  = 1;
	    }
	  else
	    {
	      HW_DMSG_FLASH("msp430:flash:   write block [0x%04x] = 0x%04x (next byte, %d)\n",
		      u16addr&0xffff,val&0xffff,size);
	      if ((u16addr & 0x3f) != 0x3f) 
		{
		  MCUFLASH.flash_ticks_left = FLASH_WRITE_TIMING_NXTBYTE;
		}
	      else
		{
		  MCUFLASH.flash_ticks_left = FLASH_WRITE_TIMING_LSTBYTE;
		  MCUFLASH.flash_write_fst  = 2;
		}
	    }
	  switch (size)
	    {
	    case 1:  mcu_jtag_write_byte(u16addr,val&0xff); break;
	    case 2:
	    default: mcu_jtag_write_word(u16addr,val);      break;
	    }

	  MCUFLASH.fctl3.b.wait = 0;
	}
      else
	{
	  msp430_io_set_flash_read_jump_pc(ADDR_FLASH_START, ADDR_FLASH_STOP);
	  MCUFLASH.flash_ticks_left = FLASH_WRITE_TIMING_BYTE;
	  switch (size)
	    {
	    case 1:
	      HW_DMSG_FLASH("msp430:flash:   write byte [0x%04x] = 0x%02x\n",u16addr&0xffff,val&0xff);
	      mcu_jtag_write_byte(u16addr,val&0xff);
	      break;
	    case 2:
	    default:
	      HW_DMSG_FLASH("msp430:flash:   write word [0x%04x] = 0x%04x\n",u16addr&0xffff,val&0xffff);
	      mcu_jtag_write_word(u16addr,val);
	      break;
	    }
	}
    }

  /* memset(region, 0xff, sizeof(region[addr])); */
  /*
    msp430f1611 memory mapping

    #define ADDR_FLASH_STOP    0xFFFFu
    #define ADDR_FLASH_START   0x4000u
    #define ADDR_NVM_STOP      0x10ffu
    #define ADDR_NVM_START     0x1000u
  */

  if ((MCUFLASH.fctl1.b.MERAS == 1) || (MCUFLASH.fctl1.b.ERASE == 1))
    {
      msp430_io_set_flash_read_jump_pc(ADDR_FLASH_START, ADDR_FLASH_STOP);
      switch ((MCUFLASH.fctl1.b.MERAS << 1) | MCUFLASH.fctl1.b.ERASE)
	{
	case 0:
	  MCUFLASH.flash_ticks_left = 0;
	  region_start = 1;
	  region_stop  = 0;
	  HW_DMSG_FLASH("msp430:flash:    -- no erase \n");
	  break;
	  
	case 1:
	  MCUFLASH.flash_ticks_left = FLASH_ERASE_TIMING_SEG;
	  if ((addr >= ADDR_FLASH_START) && (addr <= ADDR_FLASH_STOP))
	    {
	      region_start = (addr & 0xfe00);       /* 512 bytes segments */
	      region_stop  = region_start | 0x1FF;  /* 512 bytes segments */
	    }
	  else if ((addr >= ADDR_NVM_START) && (addr <= ADDR_NVM_STOP))
	    {
	      region_start = (addr & 0xff80);       /* 128 bytes segments */
	      region_stop  = region_start | 0x7f;   /* 128 bytes segments */
	    }
	  else
	    {
	      ERROR("msp430:flash: wrong segment address range for erase\n");
	    }
	  HW_DMSG_FLASH("msp430:flash:    -- erase individual segment only [0x%04x-0x%04x]\n",
		  region_start, region_stop);
	  break;
	  
	case 2:
	  MCUFLASH.flash_ticks_left = FLASH_ERASE_TIMING_MASS;
	  region_start = ADDR_FLASH_START;
	  region_stop  = ADDR_FLASH_STOP;
	  HW_DMSG_FLASH("msp430:flash:    -- erase all main memory segments\n");
	  break;
	  
	case 3:
	  MCUFLASH.flash_ticks_left = FLASH_ERASE_TIMING_MASS;
	  region_start = ADDR_FLASH_START;
	  region_stop  = ADDR_FLASH_STOP;
	  HW_DMSG_FLASH("msp430:flash:    -- erase all main and information memory segments\n");
	  for(wptr=ADDR_NVM_START; wptr<=ADDR_NVM_STOP; wptr++)
	    {
	      mcu_jtag_write_byte(wptr,0xff);
	    }
	  
	  break;
	}
      
      for(wptr=region_start; wptr<=region_stop; wptr++)
	{
	  mcu_jtag_write_byte(wptr,0xff);
	}
    }

  MCUFLASH.fctl3.b.busy     = 1;
  msp430_flash_update_ptr   = msp430_flash_update_wait;
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

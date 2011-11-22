/**
 *  \file   msp430_pmm.c
 *  \brief  MSP430 PMM definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_pmm)
/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_pmm_create()
{
  msp430_io_register_range8(PMM_IOMEM_BEGIN, PMM_IOMEM_END, msp430_pmm_read8, msp430_pmm_write8);
  msp430_io_register_range16(PMM_IOMEM_BEGIN, PMM_IOMEM_END, msp430_pmm_read, msp430_pmm_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_pmm_reset()
{
  MCU.pmm.pmmctl0.s = 0x9600;
  MCU.pmm.pmmctl1.s = 0x0000;
  MCU.pmm.svsmhctl.s = 0x4400;
  MCU.pmm.svsmlctl.s = 0x4400;
  MCU.pmm.svsmio.s = 0x0020;
  MCU.pmm.pmmifg.s = 0x0000;
  MCU.pmm.pmmrie.s = 0x0000;
  MCU.pmm.pm5ctl0.s = 0x0000;
  MCU.pmm.unlocked = 0;
  return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_pmm_update()
{
  if (MCU.pmm.unlocked == 1) {
    MCU.pmm.pmmifg.b.svsmldlyifg = 1; /* always ready */
    MCU.pmm.pmmifg.b.svmlvlrifg = 1; /* vcore always reached */
    MCU.pmm.pmmifg.s = 5;
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_pmm_read8(uint16_t addr)
{
  int8_t res;
  HW_DMSG_PMM("msp430:pmm: read [0x%04x]\n", addr);
  if (addr == PMMCTL0) {
    res = (MCU.pmm.pmmctl0.s & 0x00ff);
  } else {
    ERROR("msp430:pmm: bad read address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t msp430_pmm_read(uint16_t addr)
{
  int16_t res;
  HW_DMSG_PMM("msp430:pmm: read [0x%04x]\n", addr);
  if (addr == PMMIFG) {
    //res = 5; //some flags for aostubs init
    res = MCU.pmm.pmmifg.s;
  } else {
    ERROR("msp430:pmm: bad read address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_pmm_write8(uint16_t addr, int8_t val)
{
  HW_DMSG_PMM("msp430:pmm: write : [0x%04x] = 0x%02x\n", addr, val & 0xff);
  if (addr == PMMCTL0) {
    MCU.pmm.pmmctl0.s = (MCU.pmm.pmmctl0.s & 0xff00) | (val);
  } else if (addr == (PMMCTL0 + 1)) {
    if ((val & 0xff) == 0xa5) {
      MCU.pmm.unlocked = 1;
      HW_DMSG_PMM("msp430:pmm: unlocked\n", addr, val & 0xff);
    } else {
      MCU.pmm.unlocked = 0;
      HW_DMSG_PMM("msp430:pmm: locked\n", addr, val & 0xff);
    }
  } else {
    ERROR("msp430:pmm: bad write address 0x%04x = 0x%02x\n", addr, val & 0xff);
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_pmm_write(uint16_t addr, int16_t val)
{
  HW_DMSG_PMM("msp430:pmm: write : [0x%04x] = 0x%04x\n", addr, val);
  if (addr == SVSMHCTL) {
    MCU.pmm.svsmhctl.s = val;
  } else if (addr == SVSMLCTL) {
    MCU.pmm.svsmhctl.s = val;
  } else if (addr == PMMIFG) {
    MCU.pmm.pmmifg.s = val;
  } else {
    ERROR("msp430:pmm: bad write address 0x%04x = 0x%04x\n", addr, val);
  }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // _have_pmm

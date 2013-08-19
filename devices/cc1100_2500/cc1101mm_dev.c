
/**
 *  \file   cc1101mm_dev.c
 *  \brief  MSP430 CC1101 definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "machine/machine.h"
#include "liblogpkt/pcap.h"
#include "arch/msp430/msp430.h"

#include "cc1100_2500_internals.h"

#include "cc1101mm_dev.h"

#if defined(CC1101MM)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
//used internal register function
uint8_t cc1100_read_ro_register                 (struct _cc1100_t *cc1100, uint8_t addr);

/* Global Variables (not backtracked) */
int CC1100_XOSC_FREQ_MHz;
int CC1100_XOSC_PERIOD_NS;
int CC1100_RCOSC_FREQ_KHz;
int CC1100_RCOSC_PERIOD_NS;

tracer_id_t TRACER_CC1100_STATE;
tracer_id_t TRACER_CC1100_STROBE;
tracer_id_t TRACER_CC1100_CS;
tracer_id_t TRACER_CC1100_SO;
tracer_id_t TRACER_CC1100_GDO0;
tracer_id_t TRACER_CC1100_GDO2;

#define cc1101 MCU_RFCC1101
#define cc1100 cc1101.cc1100

/* forward prototypes */
void cc1101mm_setstate         (int8_t state);
void cc1101mm_data             (int8_t data);
void cc1101mm_instruction      (uint8_t val);
void cc1101mm_writestatus      (int rxfifo);
void cc1101mm_writestatus_data (int rxfifo);

void
cc1101mm_create ()
{
  // Register memory range
  msp430_io_register_range8 (CC1101_IOMEM_OFFSET, CC1101_IOMEM_END, cc1101mm_read8, cc1101mm_write8);
  msp430_io_register_range16 (CC1101_IOMEM_OFFSET, CC1101_IOMEM_END, cc1101mm_read16, cc1101mm_write16);


  // Create own CC1101 struct
  //static struct _cc1100_t newcc1100;
  //cc1100 = &newcc1100;
  //
  // cc1100 structure is statically allocated in the
  // main msp430 stucture
  cc1100 = & MCU_RFCC1101.cc1100_struct;

  //TODO change this according to "no internal osc"
  CC1100_XOSC_FREQ_MHz = 27;
  CC1100_XOSC_PERIOD_NS = 1000 / 27;
  CC1100_RCOSC_FREQ_KHz = 2700 / 750;
  CC1100_RCOSC_PERIOD_NS = (750 * 1000) / 27;

  cc1100->worldsens_radio_id = worldsens_c_rx_register ((void*) cc1100, cc1100_callback_rx, "omnidirectional");
  cc1100->channel_busy_timer = 0;

  TRACER_CC1100_STATE = tracer_event_add_id (8, "state", "cc1100");
  TRACER_CC1100_STROBE = tracer_event_add_id (8, "strobe", "cc1100");
  TRACER_CC1100_CS = tracer_event_add_id (1, "cs", "cc1100");
  TRACER_CC1100_SO = tracer_event_add_id (1, "so", "cc1100");
  TRACER_CC1100_GDO0 = tracer_event_add_id (1, "gdo0", "cc1100");
  TRACER_CC1100_GDO2 = tracer_event_add_id (1, "gdo2", "cc1100");

  /* init packets log */
  logpkt_init_interface (cc1100->worldsens_radio_id, "cc1100", PCAP_DLT_USER0);
  

}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int
cc1101mm_reset ()
{
  cc1100_reset_internal (cc1100);
  cc1100->registers[CC1100_REG_VERSION] = 0x06; //version 0x06 instead of 0x03
  cc1100->registers[CC1100_REG_IOCFG0] = 0x2E; //The default setting for IOCFG0.GDO0_CFGx changed from 0x3F (RFCLK/192) to 0x2E (3-state).


  /* statemachine */
  cc1101.statemachine.data = 0x00;
  cc1101.statemachine.instruction = 0x00;
  cc1101.statemachine.parameter = 0x00;
  cc1101.statemachine.rxfifo = 0x00;
  cc1101mm_setstate (CC1101MM_WAIT_INSTR);

  /* registers */
  cc1101.rf1aifctl0.s = 0x0000;
  cc1101.rf1aifctl1.s = 0x0010;
  cc1101.rf1aiferr.s = 0x0000;
  cc1101.rf1aiferrv = 0x0000;
  cc1101.rf1aifiv = 0x0000;
  cc1101.rf1ainstrw = 0x0000;
  cc1101.rf1ainstr1b = 0x00;
  cc1101.rf1ainstr2b = 0x00;
  cc1101.rf1adinw = 0x0000;
  cc1101.rf1astatw = 0x0000;
  cc1101.rf1astat1w = 0x0000;
  cc1101.rf1astat2w = 0x0000;
  cc1101.rf1adoutw = 0x0000;
  cc1101.rf1adout1w = 0x0000;
  cc1101.rf1adout2w = 0x0000;
  cc1101.rf1ain = 0x0000;
  cc1101.rf1aifg = 0x0000;
  cc1101.rf1aies = 0x0000;
  cc1101.rf1aie = 0x0000;
  cc1101.rf1aiv = 0x0000;
  
  cc1101.last_GO0_pin = 0;
  cc1101.last_GO1_pin = 0;
  cc1101.last_GO2_pin = 0;
  
  cc1101.send_intr = 0;

  return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void
cc1101mm_update ()
{
  cc1100_update_state (cc1100);
  
  /* RF1AIFG */
  /* high->low / low->high opposite to datasheet (expected by simpliciti) */
  if(cc1100->GO0_pin != cc1101.last_GO0_pin) {
    if(((cc1100->GO0_pin) && ((cc1101.rf1aie & 0x01)==0x01)) || ((!cc1100->GO0_pin) && ((cc1101.rf1aie & 0x01)==0x00))) {
      cc1101.rf1aifg |= 0x01;
    }
  }
  
  if(cc1100->GO1_pin != cc1101.last_GO1_pin) {
    if((!cc1100->GO1_pin && ((cc1101.rf1aie & 0x02)==0x00)) || (cc1100->GO1_pin && ((cc1101.rf1aie & 0x02)==0x02))) {
      cc1101.rf1aifg |= 0x02;
    }
  }
  if(cc1100->GO2_pin != cc1101.last_GO2_pin) {
    if((!cc1100->GO2_pin && ((cc1101.rf1aie & 0x04)==0x00)) || (cc1100->GO2_pin && ((cc1101.rf1aie & 0x04)==0x04))) {
      cc1101.rf1aifg |= 0x04;
    }
  }
  
  cc1101.last_GO0_pin = cc1100->GO0_pin;
  cc1101.last_GO1_pin = cc1100->GO1_pin;
  cc1101.last_GO2_pin = cc1100->GO2_pin;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t
cc1101mm_read8 (uint16_t addr)
{
  HW_DMSG ("msp430:cc1101:  byte read at address 0x%04x\n", addr);
  int8_t res;
  switch (addr)
    {
    case RF1AIFCTL1:
      res = cc1101.rf1aifctl1.s >> 8;
      break;
    case RF1AIFCTL1 + 1:
      res = cc1101.rf1aifctl1.s;
      break;
    case RF1ASTATW:
      res = cc1101.rf1astatw;
      break;
    case RF1ASTATW + 1:
      res = (cc1101.rf1astatw >> 8);
      break;
    case RF1ASTAT1W:
      res = cc1101.rf1astat1w;
      break;
    case RF1ASTAT1W + 1:
      res = (cc1101.rf1astat1w >> 8);
      break;
    case RF1ASTAT2W + 1:
      res = (cc1101.rf1astat2w >> 8);
      break;
    default:
      ERROR ("msp430:cc1101: bad byte read at address 0x%04x\n", addr);
      res = 0;
    }
  HW_DMSG ("msp430:cc1101:  value 0x%02x\n", res & 0xff);
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t
cc1101mm_read16 (uint16_t addr)
{
  HW_DMSG ("msp430:cc1101:  word read at address 0x%04x\n", addr);
  int16_t res;
  switch (addr)
    {
    case RF1AIFCTL0:
      res = cc1101.rf1aifctl0.s;
      break;
    case RF1AIFCTL1:
      res = cc1101.rf1aifctl1.s;
      break;
    case RF1AIFERR:
      res = cc1101.rf1aiferr.s;
      break;
    case RF1AIFERRV:
      res = cc1101.rf1aiferrv;
      break;
    case RF1AIFIV:
      res = cc1101.rf1aifiv;
      break;
    case RF1AINSTRW:
      res = cc1101.rf1ainstrw;
      break;
    case RF1AINSTR1W:
      res = 0x00 + cc1101.rf1ainstr1b;
      break;
    case RF1AINSTR2W:
      res = 0x00 + cc1101.rf1ainstr2b;
      break;
    case RF1ADINW:
      res = cc1101.rf1adinw;
      break;
    case RF1ASTATW:
      res = cc1101.rf1astatw;
      break;
    case RF1ASTAT1W:
      res = cc1101.rf1astat1w;
      break;
    case RF1ASTAT2W:
      res = cc1101.rf1astat2w;
      break;
    case RF1ADOUTW:
      res = cc1101.rf1adoutw;
      cc1101.rf1aifctl1.b.rfdoutifg = 0;
      break;
    case RF1ADOUT1W:
      res = cc1101.rf1adout1w;
      cc1101.rf1aifctl1.b.rfdoutifg = 0;
      break;
    case RF1ADOUT2W:
      res = cc1101.rf1adout2w;
      cc1101.rf1aifctl1.b.rfdoutifg = 0;
      break;
    case RF1AIN:
      res = cc1101.rf1ain;
      if (cc1100->GO0_pin)
        {
          res |= 1;
        }
      if (cc1100->GO1_pin)
        {
          res |= (1 << 1);
        }
      if (cc1100->GO2_pin)
        {
          res |= (1 << 2);
        }
      if (cc1100->rxOverflow)
        {
          res |= (1 << 3);
          res |= (1 << 4);
        }
      HW_DMSG ("msp430:cc1101: word read RF1AIN 0x%04x = 0x%04x\n", addr, res & 0xffff);
      break;
    case RF1AIFG:
      res = cc1101.rf1aifg;
      HW_DMSG ("msp430:cc1101: word read RF1AIFG = 0x%04x\n", res & 0xffff);
      break;
    case RF1AIES:
      res = cc1101.rf1aies;
      break;
    case RF1AIE:
      res = cc1101.rf1aie;
      break;
    case RF1AIV:
      res = cc1101.rf1aiv;
      break;
    default:
      ERROR ("msp430:cc1101: bad word read at address 0x%04x\n", addr);
      res = 0;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void
cc1101mm_write8 (uint16_t addr, int8_t val)
{
  HW_DMSG ("msp430:cc1101: byte write at address 0x%04x = 0x%02x\n", addr, val & 0xff);
  switch (addr)
    {
    case RF1AINSTRW: /* RF1ADINB */
      cc1101.rf1ainstrw = (cc1101.rf1ainstrw & 0x00ff) | val;
      cc1101mm_data (val);
      break;
    case RF1AINSTRW + 1: /* RF1AINSTRB */
      cc1101.rf1ainstrw = (cc1101.rf1ainstrw & 0xff00) | val;
      cc1101mm_instruction (val);
      break;
    case RF1AINSTR1W + 1: /* RF1AINSTR1B */
      cc1101.rf1ainstr1b = val;
      cc1101mm_instruction (val);
      cc1101mm_data (0x00);
      break;
    default:
      ERROR ("msp430:cc1101: bad byte write at address 0x%04x = 0x%02x\n", addr, val & 0xff);
    }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void
cc1101mm_write16 (uint16_t addr, int16_t val)
{
  uint16_t UNUSED tmp = val;
  HW_DMSG ("msp430:cc1101: word write at address 0x%04x = 0x%04x\n", addr, val & 0xffff);
  switch (addr)
    {
    case RF1AIFCTL1:
      cc1101.rf1aifctl1.s = val;
      break;
    case RF1AINSTRW:
      cc1101.rf1ainstrw = val;
      cc1101mm_instruction (val >> 8); //little endian way!
      cc1101mm_data (val);
      break;
    case RF1AIFG:
      cc1101.rf1aifg = val;
    case RF1AIES:
      HW_DMSG ("msp430:cc1101: word write at RF1AIES = 0x%04x\n", val & 0xffff);
      cc1101.rf1aies = val;
      break;
    case RF1AIE:
      HW_DMSG ("msp430:cc1101: word write at RF1AIE = 0x%04x\n", val & 0xffff);
      cc1101.rf1aie = val;
      break;
    default:
      ERROR ("msp430:cc1101: bad word write at address 0x%04x = 0x%04x\n", addr, val & 0xffff);
    }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */
void
cc1101mm_instruction (uint8_t val)
{
  uint8_t val_strobe;
  uint8_t val_firstbits;

  /* check error condition */
  if (cc1101.statemachine.state == CC1101MM_WAIT_DATA)
    {
      cc1101.rf1aiferr.b.operr = 1;
    }

  val_strobe = val & 0x7f;
  val_firstbits = val & 0xc0;

  if (val_strobe == CC1101MM_SRES)
    {
      cc1101mm_reset ();
      //make this better
      cc1100->addr = CC1100_STROBE_SPWD;
      cc1100_strobe_command (cc1100);
    }
  else if (val_strobe == CC1101MM_SFSTXON)
    {
      ERROR ("cc1101mm:instruction: SFSTXON not implemented\n");
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SXOFF)
    {
      HW_DMSG ("cc1101mm:instruction: SXOFF\n");
      cc1100->addr = CC1100_STROBE_SXOFF;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SCAL)
    {
      ERROR ("cc1101mm:instruction: SCAL not implemented\n");
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SRX)
    {
      HW_DMSG ("cc1101mm:instruction: SRX\n");
      cc1100->addr = CC1100_STROBE_SRX;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_STX)
    {
      HW_DMSG ("cc1101mm:instruction: STX\n");
      cc1100->addr = CC1100_STROBE_STX;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SIDLE)
    {
      HW_DMSG ("cc1101mm:instruction: SIDLE\n");
      cc1100->addr = CC1100_STROBE_SIDLE;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SWOR)
    {
      ERROR ("cc1101mm:instruction: SWOR not implemented\n");
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SPWD)
    {
      ERROR ("cc1101mm:instruction: SPWD not implemented\n");
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SFRX)
    {
      HW_DMSG ("cc1101mm:instruction: SFRX\n");
      cc1100->addr = CC1100_STROBE_SFRX;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SFTX)
    {
      HW_DMSG ("cc1101mm:instruction: SFTX\n");
      cc1100->addr = CC1100_STROBE_SFTX;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SWORRST)
    {
      ERROR ("cc1101mm:instruction: SWORRST not implemented\n");
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
  else if (val_strobe == CC1101MM_SNOP)
    {
      //HW_DMSG ("cc1101mm:instruction: SNOP\n");
      cc1100->addr = CC1100_STROBE_SNOP;
      cc1100_strobe_command (cc1100);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    }
    /* Begin of instructions awaiting DATA*/
  else if (val == CC1101MM_SNGLPATABRD)
    {
      ERROR ("cc1101mm:instruction: SNGLPATABRD not implemented\n");
    }
  else if (val == CC1101MM_SNGLPATABWR)
    {
      HW_DMSG ("cc1101mm:instruction: SNGLPATABWR\n");
      cc1101.statemachine.instruction = CC1101MM_SNGLPATABWR;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val == CC1101MM_PATABRD)
    {
      HW_DMSG ("cc1101mm:instruction: PATABRD\n");
      cc1101.statemachine.instruction = CC1101MM_PATABRD;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val == CC1101MM_PATABWR)
    {
      HW_DMSG ("cc1101mm:instruction: PATABWR\n");
      cc1101.statemachine.instruction = CC1101MM_PATABWR;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val == CC1101MM_SNGLRXRD)
    {
      HW_DMSG ("cc1101mm:instruction: SNGLRXRD\n");
      cc1101.statemachine.instruction = CC1101MM_SNGLRXRD;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val == CC1101MM_SNGLTXWR)
    {
      ERROR ("cc1101mm:instruction: SNGLTXWR not implemented\n");
    }
  else if (val == CC1101MM_RXFIFORD)
    {
      ERROR ("cc1101mm:instruction: RXFIFORD not implemented\n");
    }
  else if (val == CC1101MM_TXFIFOWR)
    {
      HW_DMSG ("cc1101mm:instruction: TXFIFOWR\n");
      cc1101.statemachine.instruction = CC1101MM_TXFIFOWR;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
    /* following instructions need to be checked as last */
  else if (val_firstbits == CC1101MM_SNGLREGRD)
    {
      HW_DMSG ("cc1101mm:instruction: SNGLREGRD\n");
      cc1101.statemachine.instruction = CC1101MM_SNGLREGRD;
      cc1101.statemachine.parameter = val & 0x3f;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val_firstbits == CC1101MM_SNGLREGWR)
    {
      HW_DMSG ("cc1101mm:instruction: SNGLREGWR\n");
      cc1101.statemachine.instruction = CC1101MM_SNGLREGWR;
      cc1101.statemachine.parameter = val & 0x3f;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else if (val_firstbits == CC1101MM_REGRD)
    {
      if ((val & 0x3f) < 0x30)
        {
          HW_DMSG ("cc1101mm:instruction: REGRD\n");
          cc1101.statemachine.instruction = CC1101MM_REGRD;
          cc1101.statemachine.parameter = val & 0x3f;
          cc1101mm_setstate (CC1101MM_WAIT_DATA);
        }
      else
        {
          HW_DMSG ("cc1101mm:instruction: STATREGRD\n");
          cc1101.statemachine.instruction = CC1101MM_STATREGRD;
          cc1101.statemachine.parameter = val & 0x3f;
          cc1101mm_setstate (CC1101MM_WAIT_DATA);
        }
    }
  else if (val_firstbits == CC1101MM_REGWR)
    {
      HW_DMSG ("cc1101mm:instruction: REGWR");
      cc1101.statemachine.instruction = CC1101MM_REGWR;
      cc1101.statemachine.parameter = val & 0x3f;
      cc1101mm_setstate (CC1101MM_WAIT_DATA);
    }
  else
    {
      ERROR ("cc1101mm:instruction unknown: 0x%02x", val);
    }
  if(val_strobe != CC1101MM_PATABRD && val_strobe != CC1101MM_PATABWR && val_strobe != CC1101MM_SNGLPATABRD && val_strobe != CC1101MM_SNGLPATABWR) {
    // Reset PATable counter
    cc1100->patable_cnt = 0;
  }

  cc1101.statemachine.rxfifo = val & 0x80;
  cc1101mm_writestatus (val & 0x80);
}

void
cc1101mm_data (int8_t data)
{
  HW_DMSG ("msp430:cc1101:cc1101mm_data  data 0x%02x\n", data);
  uint8_t regdata;
  /* check error condition */
  if (cc1101.statemachine.state == CC1101MM_WAIT_INSTR)
    {
      ERROR ("cc1101mm:data:awaiting instruction\n");
      cc1101.rf1aiferr.b.operr = 1;
      return;
    }

  switch (cc1101.statemachine.instruction)
    {
    case CC1101MM_SNGLREGRD:
      regdata = cc1100_read_register (cc1100, cc1101.statemachine.parameter);
      cc1101.rf1astatw = (cc1101.rf1astatw & 0xff00) | regdata;
      cc1101.rf1astat1w = (cc1101.rf1astat1w & 0xff00) | regdata;
      cc1101.rf1astat2w = (cc1101.rf1astat2w & 0xff00) | regdata;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
      break;
    case CC1101MM_SNGLREGWR:
      cc1100_write_register (cc1100, cc1101.statemachine.parameter, data);
      cc1101mm_writestatus_data (cc1101.statemachine.rxfifo);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
      break;
    case CC1101MM_REGRD:
      regdata = cc1100_read_register (cc1100, cc1101.statemachine.parameter);
      cc1101.rf1astatw = (cc1101.rf1astatw & 0xff00) | regdata;
      cc1101.rf1astat1w = (cc1101.rf1astat1w & 0xff00) | regdata;
      cc1101.rf1astat2w = (cc1101.rf1astat2w & 0xff00) | regdata;
      cc1101.statemachine.parameter++;
      cc1101.rf1aifctl1.b.rfdoutifg = 1;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR_DATA);
      break;
    case CC1101MM_REGWR:
      cc1100_write_register (cc1100, cc1101.statemachine.parameter, data);
      cc1101mm_writestatus_data (cc1101.statemachine.rxfifo);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR_DATA);
      break;
    case CC1101MM_STATREGRD:
      regdata = cc1100_read_ro_register(cc1100,cc1101.statemachine.parameter);
      cc1101.rf1astatw = (cc1101.rf1astatw & 0xff00) | regdata;
      cc1101.rf1astat1w = (cc1101.rf1astat1w & 0xff00) | regdata;
      cc1101.rf1astat2w = (cc1101.rf1astat2w & 0xff00) | regdata;
      cc1101.rf1aifctl1.b.rfdoutifg = 1;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
      break;
    case CC1101MM_SNGLPATABWR:
      cc1100->patable[cc1100->patable_cnt] = data;
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
    case CC1101MM_PATABRD:
      //HW_DMSG ("msp430:cc1101:  patable 0x%04x\n", cc1100->patable[cc1100->patable_cnt]);
      cc1101.rf1astatw = (cc1101.rf1astatw & 0xff00) | cc1100->patable[cc1100->patable_cnt];
      cc1101.rf1astat1w = (cc1101.rf1astat1w & 0xff00) | cc1100->patable[cc1100->patable_cnt];
      cc1101.rf1astat2w = (cc1101.rf1astat2w & 0xff00) | cc1100->patable[cc1100->patable_cnt];
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;
      cc1101.rf1aifctl1.b.rfdoutifg = 1;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR_DATA);
      break;
    case CC1101MM_PATABWR:
      cc1100->patable[cc1100->patable_cnt] = data;
      cc1100->patable_cnt = (cc1100->patable_cnt + 1) % 8;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR_DATA);
      break;
    case CC1101MM_SNGLRXRD:
      regdata = (0xff & cc1100_get_rx_fifo(cc1100));
      cc1101.rf1astatw = (cc1101.rf1astatw & 0xff00) | regdata;
      cc1101.rf1astat1w = (cc1101.rf1astat1w & 0xff00) | regdata;
      cc1101.rf1astat2w = (cc1101.rf1astat2w & 0xff00) | regdata;
      cc1101.rf1aifctl1.b.rfdoutifg = 1;
      cc1101mm_setstate (CC1101MM_WAIT_INSTR);
      break;
    case CC1101MM_TXFIFOWR:
      cc1100_put_tx_fifo (cc1100, data);
      cc1101mm_setstate (CC1101MM_WAIT_INSTR_DATA);
      break;
    default:
      ERROR ("cc1101mm:instrdata: instruction not implemented\n");
    }


  cc1101mm_writestatus (cc1101.statemachine.rxfifo & 0x80);
}

void
cc1101mm_setstate (int8_t state)
{
  cc1101.statemachine.state = state;
  switch (state)
    {
    case CC1101MM_WAIT_INSTR:
      cc1101.rf1aifctl1.b.rfinstrifg = 1;
      break;
    case CC1101MM_WAIT_DATA:
      cc1101.rf1aifctl1.b.rfinstrifg = 0;
      cc1101.rf1aifctl1.b.rfdinifg = 1;
      break;
    case CC1101MM_WAIT_INSTR_DATA:
      cc1101.rf1aifctl1.b.rfinstrifg = 1;
      cc1101.rf1aifctl1.b.rfdinifg = 1;
      break;
    }
}

uint8_t
cc1101mm_statusbyte (int rxfifo)
{
  uint8_t status = 0x00;

  switch (cc1100->fsm_state)
    {
    case CC1100_STATE_SLEEP: /* don't care, STATUS not available in SLEEP mode        */
      status = 0; /* (CC1100_STATUS_IDLE                << 4) & 0xF0;  */
      break;

    case CC1100_STATE_XOFF: /* don't really care since XOFF is set when CSn goes low */
      status = 0; /* (CC1100_STATUS_IDLE                << 4) & 0xF0;  */
      break;

    case CC1100_STATE_IDLE:
      status = (CC1100_STATUS_IDLE << 4) & 0xF0;
      break;
    case CC1100_STATE_MANCAL:
    case CC1100_STATE_CALIBRATE:
    case CC1100_STATE_FS_CALIBRATE:
      status = (CC1100_STATUS_CALIBRATE << 4) & 0xF0;
      break;
    case CC1100_STATE_TX:
      status = (CC1100_STATUS_TX << 4) & 0xF0;
      break;
    case CC1100_STATE_FSTXON:
      status = (CC1100_STATUS_FSTXON << 4) & 0xF0;
      break;

    case CC1100_STATE_SETTLING:
    case CC1100_STATE_FS_WAKEUP:
    case CC1100_STATE_TXRX_SETTLING:
    case CC1100_STATE_RXTX_SETTLING:
      status = (CC1100_STATUS_SETTLING << 4) & 0xF0;
      break;

    case CC1100_STATE_RX:
      status = (CC1100_STATUS_RX << 4) & 0xF0;
      break;
    case CC1100_STATE_RX_OVERFLOW:
      status = (CC1100_STATUS_RXFIFO_OVERFLOW << 4) & 0xF0;
      break;
    case CC1100_STATE_TX_UNDERFLOW:
      status = (CC1100_STATUS_TXFIFO_UNDERFLOW << 4) & 0xF0;
      break;
    default:
      CC1100_DBG_IMPL ("cc1100:write_status: unspecified value 0x%02x\n", cc1100->fsm_state & 0xff);
      status = 0x00;
      break;
    }

  if (rxfifo)
    {
      if ((cc1100->rxBytes) > 15)
        {
          status |= 0x0F;
        }
      else
        {
          status |= ( 0x0F & cc1100->rxBytes );
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
          status |= ( 0x0F & (CC1100_TXFIFO_LENGTH - cc1100->txBytes) );
        }
    }
  return status;
}

void
cc1101mm_writestatus (int rxfifo)
{
  uint8_t status = cc1101mm_statusbyte (rxfifo);
  cc1101.rf1astatw = (cc1101.rf1astatw & 0x00ff) | (status << 8);
  cc1101.rf1astat1w = (cc1101.rf1astat1w & 0x00ff) | (status << 8);
  cc1101.rf1astat2w = (cc1101.rf1astat2w & 0x00ff) | (status << 8);
  //HW_DMSG ("msp430:cc1101:cc1101mm_writestatus:after:  rf1astatw 0x%04x\n", cc1101.rf1astatw);
}

void
cc1101mm_writestatus_data (int rxfifo)
{
  cc1101mm_writestatus(rxfifo);
  /* borked?
  uint8_t status = cc1101mm_statusbyte (rxfifo);
  cc1101.rf1astatw = (cc1101.rf1astatw & 0x00ff) | status;
  cc1101.rf1astat1w = (cc1101.rf1astat1w & 0x00ff) | status;
  cc1101.rf1astat2w = (cc1101.rf1astat2w & 0x00ff) | status;
   * */
}

int
cc1101mm_chkifg ()
{
  if(cc1101.send_intr) {
    cc1101.send_intr=0;
    msp430_interrupt_set(INTR_RF1A);
    return 1;
  } else {
    return 0;
  }
}

void cc1101mm_assert_gdo(uint8_t event, int assert)
{
  switch (event) {
  case 0x00: /* RFIFG3 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 3);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 3));
    }
    break;
  case 0x01: /* RFIFG4 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 4);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 4));
    }
    break;
  case 0x02: /* RFIFG5 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 5);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 5));
    }
    break;
  case 0x03: /* RFIFG6 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 6);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 6));
    }
    break;
  case 0x04: /* RFIFG7 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 7);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 7));
    }
    break;
  case 0x05: /* RFIFG8 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 8);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 8));
    }
    break;
  case 0x06: /* RFIFG9 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 9);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 9));
      if(cc1101.rf1aie & (1<<9)) {
        cc1101.send_intr = 1;
        cc1101.rf1aifg |= (1<<9);
        cc1101.rf1aiv = 0x0014;
      }
    }
    break;
  case 0x07: /* RFIFG10 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 10);
      /*if(cc1101.rf1aie & (1<<10)) {
        cc1101.send_intr = 1;
        cc1101.rf1aifg |= (1<<10);
        //cc1101.rf1aiv = 0x002c;
      }*/
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 10));
    }
    break;
  case 0x08: /* RFIFG11 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 11);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 11));
    }
    break;
  case 0x09: /* RFIFG12 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 12);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 12));
    }
    break;
  case 0x0E: /* RFIFG13 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 13);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 13));
    }
    break;
  case 0x24: /* RFIFG14 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 14);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 14));
    }
    break;
  case 0x25: /* RFIFG15 */
    if (assert == CC1100_PIN_ASSERT) {
      cc1101.rf1ain |= (1 << 15);
    } else {
      cc1101.rf1ain = (cc1101.rf1ain & !(1 << 15));
    }
    break;
  }
}

#endif // _have_cc1101mm



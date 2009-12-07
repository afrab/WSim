
/**
 *  \file   atmega128.c
 *  \brief  Atmega128 MCU definition and macros
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "atmega128.h"
#include "src/options.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(ENABLE_RAM_CONTROL)
/* 
 * This RAM Control is not backtracked since we have to survice a 
 * backtrack when doing GDB hardware breakpoint while in WSNet mode.
 * This will have an influence on read before write error detection.
 *
 */
uint8_t  MCU_RAMCTL      [MAX_RAM_SIZE];
uint32_t MCU_RAMCTL_ADDR;
#endif

struct atmega128_mcu_t mcu;
struct atmega128_mcu_t mcu_backup;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int mcu_arch_id(void)
{
  return MCU_ARCH_ID;
}

int mcu_mach_id(void)
{
  return MCU_MACH_ID;
}

char* mcu_name(void)
{
  return MCU_NAME;
}

char* mcu_modelname(void)
{
  return MCU_MODEL_NAME;
}

void mcu_print_description(void)
{
  OUTPUT("mcu   : %s\n",MCU_NAME);
  OUTPUT("model : %s\n",MCU_MODEL_NAME);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint64_t mcu_get_cycles(void)
{
  return MCU_CYCLE_CPT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint64_t mcu_get_insn(void)
{
  return MCU_INSN_CPT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int mcu_options_add(void)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int atmega128_mcu_create(int UNUSED xtal, int UNUSED xosc)
{
  HW_DMSG_ATM("== ATMEGA128 creation\n");
  MCU_INSN_CPT  = 0;
  MCU_CYCLE_CPT = 0;
  atmega128_io_init();
  
  // MCU_CLOCK.xtal_freq = xtal;
  // MCU_CLOCK.xosc_freq = xosc;

  mcu_ramctl_init();
  mcu_print_description();
  HW_DMSG_ATM("==\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_reset(void)
{
  atmega128_alu_reset();
  atmega128_digiIO_reset();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_state_save(void)
{
  HW_DMSG_ATM("atmega128: == state save \n");
  HW_DMSG_ATM("atmega128: PC 0x%04x \n",mcu_get_pc());
  HW_DMSG_ATM("atmega128: == \n");
  memcpy(&mcu_backup,&mcu,sizeof(struct atmega128_mcu_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_state_restore(void)
{
  unsigned int old_run_mode = RUNNING_MODE();
  HW_DMSG_ATM("atmega128: == state restore \n");
  HW_DMSG_ATM("atmega128: OLD PC 0x%04x \n",mcu_get_pc());
  memcpy(&mcu,&mcu_backup,sizeof(struct atmega128_mcu_t));
  HW_DMSG_ATM("atmega128: NEW PC 0x%04x \n",mcu_get_pc());
  HW_DMSG_ATM("atmega128: == \n");
  if (old_run_mode != RUNNING_MODE())
    {
      mcu_signal_add( SIG_MCU_LPM_CHANGE );
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_dump_stats(int64_t user_nanotime)
{
#define NANO  (1000*1000*1000)
  float mean_cs, mean_freq;
  float simu_mean_cs, simu_mean_freq;

  mean_cs   = (float)MCU_CYCLE_CPT / ((float)MACHINE_TIME_GET_NANO() / (float)NANO);
  mean_freq = mean_cs / (1000*1000);

  OUTPUT("  simulated mcu instructions    : %"PRId64"\n",MCU_INSN_CPT);
  OUTPUT("  simulated mcu cycles          : %"PRId64"\n",MCU_CYCLE_CPT);
  OUTPUT("  simulated mcu mean freq       : %3.2f c/s (%3.2f MHz)\n",mean_cs,mean_freq);
  if (user_nanotime > 0)
    {
      simu_mean_cs   = (float)MCU_CYCLE_CPT / ((float)user_nanotime / (float)NANO);
      simu_mean_freq = simu_mean_cs / (1000*1000);
      OUTPUT("  simulation mean freq          : %3.2f c/s (%3.2f MHz)\n",simu_mean_cs,simu_mean_freq);
      OUTPUT("     - does not take into account sleep modes\n");
    }
}

/* ************************************************** */
/* ** Signals *************************************** */
/* ************************************************** */

void mcu_signal_set(uint32_t s)
{
  MCU_SIGNAL = s;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_signal_add(uint32_t s)
{
  MCU_SIGNAL |= s;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_signal_remove(uint32_t s)
{
  MCU_SIGNAL &= ~s;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_signal_clr()
{
  MCU_SIGNAL = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint32_t mcu_signal_get()
{
  return MCU_SIGNAL;
}

/* ************************************************** */
/* ** JTAG and GDB ********************************** */
/* ************************************************** */

int mcu_registers_number()
{
  return MCU_REGISTERS;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint16_t mcu_register_get(int i)
{
  return MCU_REGS[i];
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_register_set(int i, uint16_t v)
{
  MCU_REGS[i] = v;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint8_t  mcu_jtag_read_byte(uint16_t addr)
{
  return MCU_RAM[addr];
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint16_t  mcu_jtag_read_word(uint16_t addr)
{
  return MCU_RAM[addr+1] << 8 | MCU_RAM[addr];
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_jtag_write_byte(uint16_t addr, uint8_t val)
{
  MCU_RAM[addr] = val;
  mcu_ramctl_write(addr);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_jtag_write_word(uint16_t addr, uint16_t val)
{
  MCU_RAM[addr    ] = (val & 0xff);
  MCU_RAM[addr + 1] = (val >> 8) & 0xff;
  mcu_ramctl_write(addr);
  mcu_ramctl_write(addr+1);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int mcu_jtag_read_section(uint8_t *mem, uint16_t start, uint16_t size)
{
  long max_size;
  
  if (start + size > MAX_RAM_SIZE)
    max_size = (MAX_RAM_SIZE - start) - 1;
  else
    max_size = size;

  memcpy(mem, MCU_RAM + start, max_size);
  return max_size;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_jtag_write_section(uint8_t *mem, uint16_t start, uint16_t size)
{
  memcpy(MCU_RAM + start, mem, size);
  mcu_ramctl_write_block(start,size);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_jtag_write_zero(uint16_t start, uint16_t size)
{
  memset(MCU_RAM + start, 0, size);
  mcu_ramctl_write_block(start,size);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(ENABLE_RAM_CONTROL)

int mcu_ramctl_init(void)
{
  int i;
  for(i=0; i<MAX_RAM_SIZE; i++)
    {
      MCU_RAMCTL[i] = MAC_MUST_WRITE_FIRST;
    }
  MCU_RAMCTL_ADDR = 0;
  return 0;
}

void mcu_ramctl_tst_read(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_READ) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_READ) );
      MCU_RAMCTL_ADDR = addr;
    }
}

void mcu_ramctl_tst_write(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_WRITE) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_WRITE) );
      MCU_RAMCTL_ADDR = addr;
    }
} 

void mcu_ramctl_tst_fetch(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_BREAK_WATCH_FETCH) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(b & MAC_BREAK_WATCH_FETCH) );
      MCU_RAMCTL_ADDR = addr;
    }
}

void mcu_ramctl_set_bp(uint16_t addr, int type)
{
  /* SW_DMSG_BRK("software: set breakpoint type %d (%s) at 0x%04x\n",
     type,mcu_ramctl_str(type),addr); */
  MCU_RAMCTL[addr] |= type;
}

void mcu_ramctl_unset_bp(uint16_t addr, int type)
{
  /* SW_DMSG_BRK("software: del breakpoint type %d (%s) at 0x%04x\n",
     type,mcu_ramctl_str(type),addr); */
  MCU_RAMCTL[addr] &= ~type;
}

void mcu_ramctl_read(uint16_t addr)
{
  mcu_ramctl_tst_read(addr);
}

void mcu_ramctl_read_block(uint16_t addr, int size)
{
  int i;
  for(i=0; i<size; i++)
    {
      mcu_ramctl_read(addr+i);
    }
}

void mcu_ramctl_write(uint16_t addr)
{
  mcu_ramctl_tst_write(addr);
  MCU_RAMCTL[addr] &= ~MAC_MUST_WRITE_FIRST;
}

void mcu_ramctl_write_block(uint16_t addr, int size)
{
  int i;
  for(i=0; i<size; i++)
    {
      mcu_ramctl_write(addr + i);
    }
}

uint8_t mcu_ramctl_read_ctl(uint16_t addr)
{
  uint8_t ret = 0;
  /* if (ret < MAX_RAM_SIZE) */
    {
      ret = MCU_RAMCTL[addr];
    }
  return ret;
}

#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

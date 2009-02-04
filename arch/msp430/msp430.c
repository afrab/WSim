
/**
 *  \file   msp430.c
 *  \brief  MSP430 MCU definition and macros
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct msp430_mcu_t mcu;
struct msp430_mcu_t mcu_backup;

int msp430_trace_pc_switch;
int msp430_trace_sp_switch;

tracer_id_t MSP430_TRACER_ACLK;
tracer_id_t MSP430_TRACER_MCLK;
tracer_id_t MSP430_TRACER_SMCLK;
tracer_id_t MSP430_TRACER_GIE;
tracer_id_t MSP430_TRACER_PC;
tracer_id_t MSP430_TRACER_SP;
tracer_id_t MSP430_TRACER_INTR;
tracer_id_t MSP430_TRACER_LPM;
tracer_id_t MSP430_TRACER_PORT1;
tracer_id_t MSP430_TRACER_PORT2;
tracer_id_t MSP430_TRACER_PORT3;
tracer_id_t MSP430_TRACER_PORT4;
tracer_id_t MSP430_TRACER_PORT5;
tracer_id_t MSP430_TRACER_PORT6;
tracer_id_t MSP430_TRACER_USART0;
tracer_id_t MSP430_TRACER_USART1;

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
  OUTPUT("  xin : %d\n",MCU_CLOCK.lfxt1_freq);
#if defined(__msp430_have_xt2)
  OUTPUT("  xt2 : %d\n",MCU_CLOCK.xt2_freq);
#endif
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

static struct moption_t trace_pc_opt = {
  .longname    = "msp430_trc_pc",
  .type        = no_argument,
  .helpstring  = "msp430 trace PC register",
  .value       = NULL
};

static struct moption_t trace_sp_opt = {
  .longname    = "msp430_trc_sp",
  .type        = no_argument,
  .helpstring  = "msp430 trace SP register",
  .value       = NULL
};

int mcu_options_add(void)
{
  options_add( &trace_pc_opt );
  options_add( &trace_sp_opt );
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_xt2)
int msp430_mcu_create(int xt1, int xt2)
#else
int msp430_mcu_create(int xt1)
#endif
{
  HW_DMSG_MSP("== MSP430 creation\n");
  MCU_INSN_CPT  = 0;
  MCU_CYCLE_CPT = 0;
  msp430_io_init();

  MCU_CLOCK.lfxt1_freq = xt1;
#if defined(__msp430_have_xt2)
  MCU_CLOCK.xt2_freq   = xt2;
#endif

  mcu_ramctl_init();
  mcu_print_description();
  HW_DMSG_MSP("==\n");

  if (trace_pc_opt.isset) 
    {
      MSP430_TRACER_PC       = tracer_event_add_id(16, "PC", "msp430");
      msp430_trace_pc_switch = 1;
    }
  else
    {
      msp430_trace_pc_switch = 0;
    }

  if (trace_sp_opt.isset) 
    {
      MSP430_TRACER_SP       = tracer_event_add_id(16, "SP", "msp430");
      msp430_trace_sp_switch = 1;
    }
  else
    {
      msp430_trace_sp_switch = 0;
    }

  MSP430_TRACER_GIE    = tracer_event_add_id(1,  "gie",        "msp430");
  MSP430_TRACER_INTR   = tracer_event_add_id(8,  "interrupt",  "msp430");
  MSP430_TRACER_LPM    = tracer_event_add_id(8,  "LPM",        "msp430");
  MSP430_TRACER_MCLK   = tracer_event_add_id(32, "mclk",       "msp430");
  MSP430_TRACER_SMCLK  = tracer_event_add_id(32, "smclk",      "msp430");
  MSP430_TRACER_ACLK   = tracer_event_add_id(32, "aclk",       "msp430");
  MSP430_TRACER_PORT1  = tracer_event_add_id(8,  "port_out_1", "msp430");
  MSP430_TRACER_PORT2  = tracer_event_add_id(8,  "port_out_2", "msp430");
  MSP430_TRACER_PORT3  = tracer_event_add_id(8,  "port_out_3", "msp430");
  MSP430_TRACER_PORT4  = tracer_event_add_id(8,  "port_out_4", "msp430");
  MSP430_TRACER_PORT5  = tracer_event_add_id(8,  "port_out_5", "msp430");
  MSP430_TRACER_PORT6  = tracer_event_add_id(8,  "port_out_6", "msp430");
  MSP430_TRACER_USART0 = tracer_event_add_id(8,  "Usart0",     "msp430");
  MSP430_TRACER_USART1 = tracer_event_add_id(8,  "Usart1",     "msp430");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_reset(void)
{
  msp430_sfr_reset(); /* keep sfr first */

#if defined(__msp430_have_svs_at_0x55)
  msp430_svs_reset();
#endif

  /* system clock */
#if defined(__msp430_have_basic_clock)
  msp430_basic_clock_reset();
#elif defined(__msp430_have_fll_and_xt2)
  msp430_fll_clock_reset();
#else
  #error "msp430 reset : no system clock"
#endif

  /* general */
  msp430_alu_reset();
  msp430_digiIO_reset();

#if defined(__msp430_have_hwmul)
  msp430_hwmul_reset();
#endif
#if defined(__msp430_have_dma)
  msp430_dma_reset();
#endif
#if defined(__msp430_have_flash)
  msp430_flash_reset();
#endif

  /* usart */
  msp430_usart0_reset();
#if defined(__msp430_have_usart1)
  msp430_usart1_reset();
#endif

  /* timers */
#if defined(__msp430_have_watchdog)
  msp430_watchdog_reset();
#endif
#if defined(__msp430_have_basic_timer)
  msp430_basic_timer_reset();
#endif
#if defined(__msp430_have_timera3)
  msp430_timerA3_reset();
#endif
#if defined(__msp430_have_timera5)
  msp430_timerA5_reset();
#endif
#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)
  msp430_timerB_reset();
#endif

  /* cmp, adc, dac */
#if defined(__msp430_have_cmpa)
  msp430_cmpa_reset();
#endif
#if defined(__msp430_have_adc12)
  msp430_adc12_reset();
#endif
#if defined(__msp430_have_adc10)
  msp430_adc10_reset();
#endif
#if defined(__msp430_have_dac12)
  msp430_dac12_reset();
#endif

  /* misc */
#if defined(__msp430_have_lcd)
  msp430_lcd_reset();
#endif
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void 
msp430_reset_pin_assert()
{
  if (MCU.sfr.ie1.b.nmiie == 1) // nmi mode
    {
      msp430_interrupt_set(INTR_NMI);
    }
  else
    {
      HW_DMSG_MCU("msp430: ===============\n");
      HW_DMSG_MCU("msp430: == POR Reset ==\n");
      HW_DMSG_MCU("msp430: ===============\n");
      mcu_reset();
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_state_save()
{
  HW_DMSG_MSP("msp430: == state save \n");
  HW_DMSG_MSP("msp430: PC 0x%04x \n",mcu_get_pc());
  HW_DMSG_MSP("msp430: == \n");
  memcpy(&mcu_backup,&mcu,sizeof(struct msp430_mcu_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_state_restore()
{
  unsigned int old_run_mode = RUNNING_MODE();
  HW_DMSG_MSP("msp430: == state restore \n");
  HW_DMSG_MSP("msp430: OLD PC 0x%04x \n",mcu_get_pc());
  memcpy(&mcu,&mcu_backup,sizeof(struct msp430_mcu_t));
  HW_DMSG_MSP("msp430: NEW PC 0x%04x \n",mcu_get_pc());
  HW_DMSG_MSP("msp430: == \n");
  if (old_run_mode != RUNNING_MODE())
    {
      MCU_SIGNAL |= SIG_MCU_LPM_CHANGE;
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
  OUTPUT("  mcu exit at PC                : 0x%04x\n",mcu_get_pc());
  OUTPUT("  mcu exit in LPM mode          : %s\n",msp430_lpm_names[RUNNING_MODE()]);
  OUTPUT("  mcu exit with IV              : 0x%04x\n",MCU_IV);
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
/* ** RAM Control *********************************** */
/* ************************************************** */

#if defined(ENABLE_RAM_CONTROL)

void mcu_ramctl_init(void)
{
  int i;
  for(i=0; i<MAX_RAM_SIZE; i++)
    {
      MCU_RAMCTL[i] = MAC_MUST_WRITE_FIRST;
    }
}

void mcu_ramctl_tst_read(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_READ) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_READ) );
    }
}

void mcu_ramctl_tst_write(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_WRITE) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_WRITE) );
    }
} 

void mcu_ramctl_tst_fetch(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_BREAK_WATCH_FETCH) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(b & MAC_BREAK_WATCH_FETCH) );
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

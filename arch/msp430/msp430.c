
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

#if defined(ENABLE_RAM_CONTROL)
/* 
 * This RAM Control is not backtracked since we have to survice a 
 * backtrack when doing GDB hardware breakpoint while in WSNet mode.
 * This will have an influence on read before write error detection.
 *
 */
uint8_t  MCU_RAMCTL     [MAX_RAM_SIZE];
uint32_t MCU_RAMCTL_ADDR;
#endif

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
tracer_id_t MSP430_TRACER_BKP;
tracer_id_t MSP430_TRACER_USART0RX;
tracer_id_t MSP430_TRACER_USART0TX;
tracer_id_t MSP430_TRACER_USART1RX;
tracer_id_t MSP430_TRACER_USART1TX;

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
  OUTPUT_BOXM("mcu   : %s\n",MCU_NAME);
  OUTPUT_BOXM("model : %s\n",MCU_MODEL_NAME);
  OUTPUT_BOXM("  xin : %d\n",MCU_CLOCK.lfxt1_freq);
#if defined(__msp430_have_xt2)
  OUTPUT_BOXM("  xt2 : %d\n",MCU_CLOCK.xt2_freq);
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
  msp430_adc_option_add();
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_infomem_init(void)
{
  /* infomem information */
  {
    int i;
    int size = sizeof(infomem)/sizeof(infomem_t);
    for(i=0; i < size; i++)
      {
	mcu_jtag_write_byte(infomem[i].addr, infomem[i].value);
      }
  }
}

/* ************************************************** */
/* ** MCU_CREATE ************************************ */
/* ************************************************** */

#if defined(__msp430_have_ucs)
  int msp430_mcu_create(int xt1, int xt2, int vlo, int refo)
#elif defined(__msp430_have_basic_clock_plus)
  #if defined(__msp430_have_xt2)
  int msp430_mcu_create(int xt1, int xt2, int vlo)
  #else
  int msp430_mcu_create(int xt1, int vlo)
  #endif
#elif defined(__msp430_have_xt2)
int msp430_mcu_create(int xt1, int xt2)
#else
int msp430_mcu_create(int xt1)
#endif
{
  int ret = 0;

  MCU_INSN_CPT  = 0;
  MCU_CYCLE_CPT = 0;

  msp430_io_create();
  mcu_ramctl_init();
  msp430_infomem_init();


  // basic
  msp430_sfr_create();
  msp430_svs_create();
  msp430_hwmul_create();
  msp430_digiIO_create();
  msp430_pmm_create();
  msp430_portmap_create();
  // system clock
  msp430_ucs_create();
  msp430_basic_clock_create();
  msp430_basic_clock_plus_create();
  msp430_fll_clock_create();
  // serial interfaces
  msp430_uscia0_create();
  msp430_uscib0_create();
  msp430_usart0_create();
  msp430_usart1_create();
  // timers
  msp430_watchdog_create();
  msp430_basic_timer_create();
  msp430_timerA_create();
  msp430_timerB_create();
  msp430_timerTA0_create();
  msp430_timerTA1_create();
  // analog
  msp430_cmpa_create();
  msp430_adc10_create();
  msp430_adc12_create();
  msp430_dac12_create();
  // misc
  msp430_dma_create();
  msp430_flash_create();
  msp430_lcd_create();
  msp430_lcdb_create();
  msp430_rtc_create();


  MCU_CLOCK.lfxt1_freq = xt1;
#if defined(__msp430_have_xt2)
  MCU_CLOCK.xt2_freq   = xt2;
#endif
#if defined(__msp430_have_basic_clock_plus)
  MCU_CLOCK.vlo_freq   = vlo;
#endif

  msp430_trace_pc_switch = 0;
  msp430_trace_sp_switch = 0;
  if (trace_pc_opt.isset) 
    {
      MSP430_TRACER_PC       = tracer_event_add_id(16, "PC", "msp430");
      msp430_trace_pc_switch = 1;
    }

  if (trace_sp_opt.isset) 
    {
      MSP430_TRACER_SP       = tracer_event_add_id(16, "SP", "msp430");
      msp430_trace_sp_switch = 1;
    }

  MSP430_TRACER_GIE      = tracer_event_add_id(1,  "intr_gie",   "msp430");
  MSP430_TRACER_INTR     = tracer_event_add_id(8,  "intr_num",   "msp430");
  MSP430_TRACER_MCLK     = tracer_event_add_id(32, "clk_mclk",   "msp430");
  MSP430_TRACER_SMCLK    = tracer_event_add_id(32, "clk_smclk",  "msp430");
  MSP430_TRACER_ACLK     = tracer_event_add_id(32, "clk_aclk",   "msp430");
  MSP430_TRACER_LPM      = tracer_event_add_id(8,  "LPM",        "msp430");
  MSP430_TRACER_BKP      = tracer_event_add_id(8,  "breakpoint", "msp430");
  MSP430_TRACER_USART0RX = tracer_event_add_id(16, "Usart0_RX",  "msp430");
  MSP430_TRACER_USART0TX = tracer_event_add_id(16, "Usart0_TX",  "msp430");
  MSP430_TRACER_USART1RX = tracer_event_add_id(16, "Usart1_RX",  "msp430");
  MSP430_TRACER_USART1TX = tracer_event_add_id(16, "Usart1_TX",  "msp430");

  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_reset()
{
  /* 
   * Important: keep sfr first 
   */
  msp430_sfr_reset(); 

  msp430_alu_reset();
  msp430_svs_reset();
  msp430_hwmul_reset();
  msp430_digiIO_reset();
  msp430_pmm_reset();
  msp430_portmap_reset();
  // system clock
  msp430_ucs_reset();
  msp430_basic_clock_reset();
  msp430_basic_clock_plus_reset();
  msp430_fll_clock_reset();
  // serial interfaces
  msp430_uscia0_reset();
  msp430_uscib0_reset();
  msp430_usart0_reset();
  msp430_usart1_reset();
  // timers
  msp430_watchdog_reset();
  msp430_basic_timer_reset();
  msp430_timerA_reset();
  msp430_timerB_reset();
  msp430_timerTA0_reset();
  msp430_timerTA1_reset();
  // analog
  msp430_adc10_reset();
  msp430_adc12_reset();
  msp430_dac12_reset();
  msp430_cmpa_reset();
  // misc
  msp430_dma_reset();
  msp430_flash_reset();
  msp430_lcd_reset();
  msp430_lcdb_reset();
  msp430_rtc_reset();

#if defined(SOFT_INTR)
  MCU.soft_intr         = 0;
  MCU.soft_intr_timeend = 0;
  etracer_slot_event(SOFT_INTR_EVT, ETRACER_PER_EVT_MODE_CHANGED, 1, 0);
#endif
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_devices_update(unsigned int cycles)
{
  int time;
  /*
   * clock must be first
   */
  time = MCU_CLOCK_SYSTEM_UPDATE(cycles);
  MACHINE_TIME_SET_INCR(time);

  msp430_sfr_update();
  msp430_svs_update();
  msp430_hwmul_update();
  msp430_digiIO_update();
  msp430_pmm_update();
  msp430_portmap_update();
  // serial interfaces
  msp430_uscia0_update();
  msp430_uscib0_update();
  msp430_usart0_update();
  msp430_usart1_update();
  // timers
  msp430_watchdog_update();
  msp430_basic_timer_update();
  msp430_timerA_update();
  msp430_timerB_update();
  msp430_timerTA0_update();
  msp430_timerTA1_update();
  // analog
  msp430_adc10_update();
  msp430_adc12_update();
  msp430_dac12_update();
  msp430_cmpa_update();
  // misc
  msp430_dma_update();
  msp430_flash_update();
  msp430_lcd_update();
  msp430_lcdb_update();
  msp430_rtc_update();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_update_done()
{
  uint32_t signal;
  /* timers capture operations                    */
  msp430_timerA_capture();
  msp430_timerB_capture();
  msp430_timerTA0_capture();
  msp430_timerTA1_capture();

  /* serial port                                  */
  msp430_usart0_update_done();
  msp430_usart1_update_done();

  /* clear update flags on internal devices       */
  msp430_digiIO_update_done();
  MCU_CLOCK_SYSTEM_UPDATE_DONE();

  /* eSimu: record current slot, start a new one  */
  etracer_slot_end( MACHINE_TIME_GET_INCR() ); 

  /* Pending IRQ                                  */
  MCU_ALU.etracer_except = msp430_interrupt_start_if_any();

  /* */
  signal = mcu_signal_get();
  if ((signal & SIG_MCU_LPM_CHANGE) != 0)
    {
      int prev_run_mode = MCU_ALU.curr_run_mode; 
      int curr_run_mode = RUNNING_MODE();
      HW_DMSG_LPM("msp430:lpm: Low power mode [%s] changed to [%s] at [%" PRId64 "]\n",
                  msp430_lpm_names[prev_run_mode],          
                  msp430_lpm_names[curr_run_mode], 
                  MACHINE_TIME_GET_NANO());

      TRACER_TRACE_LPM(curr_run_mode);
      MCU_CLOCK_SYSTEM_SPEED_TRACER();
      mcu_signal_remove(SIG_MCU_LPM_CHANGE); 
       
      /* we were AM, we are going at least LPM0 */
      if (((prev_run_mode & 1) == 0) && ((curr_run_mode & 1) != 0))
	{
	  etracer_slot_set_ns();
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * msp430_mcu_run
 * low power state machine handling
 */
void mcu_run()
{
  unsigned int cycles;

  MCU_ALU.curr_run_mode = RUNNING_MODE(); 

  /*
   * if (runmode & 1 == 1) then the cpuoff bit is set and MCLK is disabled 
   */
  if ((MCU_ALU.curr_run_mode & 1) == 0)
    {
      cycles = msp430_mcu_run_insn();
    }
  else
    {
      /* VERBOSE(3,"msp430: run LPM mode\n"); */
      cycles = msp430_mcu_run_lpm();
    }

  msp430_devices_update(cycles);
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
      mcu_signal_add( SIG_MCU_LPM_CHANGE );
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_system_clock_speed_tracer_update(void)
{
  MCU_CLOCK_SYSTEM_SPEED_TRACER();
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

  OUTPUT_STATS("  simulated mcu instructions    : %"PRId64"\n",MCU_INSN_CPT);
  OUTPUT_STATS("  simulated mcu cycles          : %"PRId64"\n",MCU_CYCLE_CPT);
  OUTPUT_STATS("  simulated mcu mean freq       : %3.2f c/s (%3.2f MHz)\n",mean_cs,mean_freq);
  if (user_nanotime > 0)
    {
      simu_mean_cs   = (float)MCU_CYCLE_CPT / ((float)user_nanotime / (float)NANO);
      simu_mean_freq = simu_mean_cs / (1000*1000);
      OUTPUT_STATS("  simulation mean freq          : %3.2f c/s (%3.2f MHz)\n",simu_mean_cs,simu_mean_freq);
      OUTPUT_STATS("     - does not take into account sleep modes\n");
    }
  OUTPUT_STATS("  mcu exit at PC                : 0x%04x\n",mcu_get_pc());
  OUTPUT_STATS("  mcu exit in LPM mode          : %s\n",msp430_lpm_names[RUNNING_MODE()]);
  OUTPUT_STATS("  mcu exit with IV              : 0x%08x\n",MCU_IV);
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

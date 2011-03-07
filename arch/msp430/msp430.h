
/**
 *  \file   msp430.h
 *  \brief  MSP430 MCU definition and macros
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430INT_H
#define MSP430INT_H

#include <stdio.h>

/* The msp430 is little endian (default) */
#define TARGET_BYTE_ORDER_DEFAULT LITTLE_ENDIAN

#include "msp430_models.h"
#include "msp430_debug.h"
#include "msp430_sfr.h"
#include "msp430_intr.h"
#include "msp430_fll_clock.h"
#include "msp430_basic_clock.h"
#include "msp430_basic_clock_plus.h"
#include "msp430_alu.h"
#include "msp430_io.h"
#include "msp430_hwmul.h"
#include "msp430_digiIO.h"
#include "msp430_uscib.h"
#include "msp430_usart.h"
#include "msp430_basic_timer.h"
#include "msp430_watchdog.h"
#include "msp430_timer.h"
#include "msp430_lcd.h"
#include "msp430_dma.h"
#include "msp430_flash.h"
#include "msp430_svs.h"
#include "msp430_cmpa.h"
#include "msp430_adc12.h"
#include "msp430_adc10.h"
#include "msp430_dac12.h"

  /**
   * 0x0ffff
   *    interrupt vector table
   * 0x0ffe0
   *    Flash/ROM
   * --
   *    [empty]
   * --
   *    RAM
   * 0x00200
   * 0x001ff
   *    16 bits peripherals modules
   * 0x00100
   * 0x000ff
   *    8 bits peripherals modules 
   * 0x00010
   * 0x0000f
   *    special functions registers
   * 0x00000
   */

#define MAX_RAM_SIZE   0x10000 /* 64KB */

#define MCU_T msp430_mcu_t  

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

extern int msp430_trace_pc_switch;
extern int msp430_trace_sp_switch;
extern tracer_id_t MSP430_TRACER_ACLK;
extern tracer_id_t MSP430_TRACER_MCLK;
extern tracer_id_t MSP430_TRACER_SMCLK;
extern tracer_id_t MSP430_TRACER_GIE;
extern tracer_id_t MSP430_TRACER_PC;
extern tracer_id_t MSP430_TRACER_SP;
extern tracer_id_t MSP430_TRACER_INTR;
extern tracer_id_t MSP430_TRACER_LPM;
extern tracer_id_t MSP430_TRACER_BKP;
extern tracer_id_t MSP430_TRACER_PORT1;
extern tracer_id_t MSP430_TRACER_PORT2;
extern tracer_id_t MSP430_TRACER_PORT3;
extern tracer_id_t MSP430_TRACER_PORT4;
extern tracer_id_t MSP430_TRACER_PORT5;
extern tracer_id_t MSP430_TRACER_PORT6;
extern tracer_id_t MSP430_TRACER_USART0;
extern tracer_id_t MSP430_TRACER_USART1;
//extern tracer_id_t MSP430_TRACER_USCIB0;

#if defined(XCODE_DEBUG)
#define TRACER_TRACE_PC(v)			\
  do {						\
    if (msp430_trace_pc_switch)			\
      tracer_event_record(MSP430_TRACER_PC,v);	\
  } while (0)
#define TRACER_TRACE_SP(v)			\
  do {						\
    if (msp430_trace_sp_switch)			\
      tracer_event_record(MSP430_TRACER_SP,v);	\
  } while (0)
#else
#define TRACER_TRACE_PC(v)      do { } while (0)
#define TRACER_TRACE_SP(v)      do { } while (0)
#endif

#define TRACER_TRACE_GIE(v)     tracer_event_record(MSP430_TRACER_GIE,v)
#define TRACER_TRACE_INTR(v)    tracer_event_record(MSP430_TRACER_INTR,v)
#define TRACER_TRACE_LPM(v)     tracer_event_record(MSP430_TRACER_LPM,v)
#define TRACER_TRACE_BKP(v)     tracer_event_record(MSP430_TRACER_BKP,v)

#define TRACER_TRACE_ACLK(v)    tracer_event_record(MSP430_TRACER_ACLK,v)
#define TRACER_TRACE_MCLK(v)    tracer_event_record(MSP430_TRACER_MCLK,v)
#define TRACER_TRACE_SMCLK(v)   tracer_event_record(MSP430_TRACER_SMCLK,v)

#define TRACER_TRACE_PORT1(v)   tracer_event_record(MSP430_TRACER_PORT1,v)
#define TRACER_TRACE_PORT2(v)   tracer_event_record(MSP430_TRACER_PORT2,v)
#define TRACER_TRACE_PORT3(v)   tracer_event_record(MSP430_TRACER_PORT3,v)
#define TRACER_TRACE_PORT4(v)   tracer_event_record(MSP430_TRACER_PORT4,v)
#define TRACER_TRACE_PORT5(v)   tracer_event_record(MSP430_TRACER_PORT5,v)
#define TRACER_TRACE_PORT6(v)   tracer_event_record(MSP430_TRACER_PORT6,v)

#define TRACER_TRACE_USART0(v)  tracer_event_record(MSP430_TRACER_USART0,v)
#define TRACER_TRACE_USART1(v)  tracer_event_record(MSP430_TRACER_USART1,v)

//#define TRACER_TRACE_USCIB0(v)  tracer_event_record(MSP430_TRACER_USCIB0,v)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct msp430_mcu_t {

  uint8_t ram     [MAX_RAM_SIZE]; /* ram + flash */

  uint8_t RST;

  struct msp430_alu_t          alu;

#if defined(__msp430_have_fll_and_xt2)
  struct msp430_fll_clock_t    fll_clock;
#endif
#if defined(__msp430_have_basic_clock)
  struct msp430_basic_clock_t  basic_clock;
#endif
#if defined(__msp430_have_basic_clock_plus)
  struct msp430_basic_clock_plus_t  basic_clock_plus;
#endif

#if defined(__msp430_have_basic_timer)
  struct msp430_basic_timer_t  bt;
#endif
#if defined(__msp430_have_timera3)
  struct msp430_timerA3_t      timerA3;
#endif
#if defined(__msp430_have_timera5)
  struct msp430_timerA5_t      timerA5;
#endif
#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)
  struct msp430_timerB_t       timerB;
#endif

  struct msp430_digiIO_t       digiIO;
  struct msp430_sfr_t          sfr;

#if defined(__msp430_have_watchdog)
  struct msp430_watchdog_t     watchdog;
#endif

#if defined(__msp430_have_uscib0)
  struct msp430_uscib_t        uscib0;
#endif

#if defined(__msp430_have_usart0)
  struct msp430_usart_t        usart0;
#endif

#if defined(__msp430_have_usart1)
  struct msp430_usart_t        usart1;
#endif

#if defined(__msp430_have_hwmul)
  struct msp430_hwmul_t        hwmul;
#endif

#if defined(__msp430_have_lcd)
  struct msp430_lcd_t          lcd;
#endif

#if defined(__msp430_have_dma)
  struct msp430_dma_t          dma;
#endif

#if defined(__msp430_have_flash)
  struct msp430_flash_t        flash;
#endif

#if defined(__msp430_have_svs_at_0x55)
  struct msp430_svs_t          svs;
#endif

#if defined(__msp430_have_adc10)
  struct msp430_adc10_t        adc10;
#endif
#if defined(__msp430_have_adc12)
  struct msp430_adc12_t        adc12;
#endif

#if defined(SOFT_INTR)
  int      soft_intr;
  uint64_t soft_intr_timeend;
#endif
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

extern struct msp430_mcu_t mcu;
extern struct msp430_mcu_t mcu_backup;

#define MCU                mcu
#define MCU_ALU            MCU.alu
#define MCU_REGS           MCU_ALU.regs
#define MCU_IV             MCU_ALU.interrupt_vector
#define MCU_SIGNAL         MCU_ALU.signal
#define MCU_INSN_CPT       MCU_ALU.insn_counter
#define MCU_CYCLE_CPT      MCU_ALU.cycle_counter
#define MCU_HWMUL          MCU.hwmul
#define MCU_FLASH          MCU.flash
#define MCU_RAM            MCU.ram
#define MCU_DMA            MCU.dma

#if defined(__msp430_have_basic_clock)
#define MCU_CLOCK          MCU.basic_clock
#endif
#if defined(__msp430_have_basic_clock_plus)
#define MCU_CLOCK          MCU.basic_clock_plus
#endif
#if defined(__msp430_have_fll_and_xt2)
#define MCU_CLOCK          MCU.fll_clock
#endif

#define CYCLE_INCREMENT    MCU_CLOCK.MCLK_increment

#define MCU_INTR           msp430_interrupt

#if defined(__msp430_have_basic_clock_plus)
   #if defined(__msp430_have_xt2)
   int msp430_mcu_create(int xt1, int xt2, int vlo);
   #else
   int msp430_mcu_create(int xt1, int vlo);
   #endif
#elif defined(__msp430_have_xt2)
int msp430_mcu_create(int xt1, int xt2);
#else
int msp430_mcu_create(int xt1);
#endif

void     msp430_reset_pin_assert ();

#endif /* ifndef _MSP430_H */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

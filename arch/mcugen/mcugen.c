
/**
 *  \file   mcugen.c
 *  \brief  Generic MCU ALU emulation
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "mcugen.h"


/**
 * global variables used in this file
 *
 *
 */ 

struct mcugen_mcu_t mcu;
struct mcugen_mcu_t mcu_backup;


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define NANO  (1000*1000*1000)

int mcugen_mcu_create(int freq)
{
  MCU.clock_increment      = 0;
  MCU.clock_counter        = 0;
  MCU.clock_freq           = freq;
  MCU.clock_cycle_nanotime = NANO / freq;
  /* tracer_event_add_id */
  mcu_reset();
  mcu_ramctl_init();
  mcu_print_description();
  return 0;
}

int mcugen_clock_update(int UNUSED clock_add)
{
  int nano_add = 0;
  MCU.clock_increment  = clock_add;
  MCU.clock_counter   += clock_add;
  nano_add             = clock_add * MCU.clock_cycle_nanotime;
  return nano_add; /* nanosecond */
}

void mcugen_clock_update_done(void)
{
  /* reset internal state machine variables */
  MCU.clock_increment  = 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void mcugen_mcu_update(unsigned int cycles)
{
  int time;
  
  /*****************************/
  /* basic clock must be first */
  /*****************************/
  time = mcugen_clock_update(cycles);
  
  MACHINE_TIME_SET_INCR(time);

  /********************/
  /* internal devices */
  /********************/

  /***************************/
  /* external devices update */
  /***************************/
  devices_update(); /* update digi IO ports */

  /******************************************/
  /* clear update flags on internal devices */
  /******************************************/
  // mcugen_digiIO_update_done();
  mcugen_clock_update_done();

  /*********************************/
  /* etrace for eSimu              */
  /*********************************/
  etracer_slot_end(time); /* record current slot, start a new one */
  //  MCU_ALU.etracer_except = msp430_interrupt_start_if_any();

  /*********************************/
  /* check for a pending interrupt */
  /*********************************/
  
  MACHINE_TIME_CLR_INCR();
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void mcugen_mcu_run_insn(void)
{
  do 
    {
      unsigned int cycles = 0;
      // MCU_ALU.pc = MCU_ALU.next_pc;
      // cycles = mcugen_execute_insn();
      mcugen_mcu_update(cycles);
    }
  while (MCU.signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void mcugen_mcu_run_lpm(void)
{
#define LPM_UPDATE_CYCLES 2
  do {
    mcugen_mcu_update(LPM_UPDATE_CYCLES);
  } while (MCU.signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

int mcugen_running_mode()
{
  return 1;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void mcu_run(void)
{
  uint32_t signal;
  int curr_run_mode;
  int prev_run_mode;

  curr_run_mode = mcugen_running_mode(); 

  do {
    if ((curr_run_mode & 1) == 0)
      {
	mcugen_mcu_run_insn();
      }
    else
      {
	mcugen_mcu_run_lpm();
      }

    prev_run_mode = curr_run_mode; 
    curr_run_mode = mcugen_running_mode();
    signal        = mcu_signal_get();

    if ((signal & SIG_MCU_LPM_CHANGE) != 0)
      {
	HW_DMSG("mcugen:lpm: Low power mode changed at [%" PRId64 "]\n",
		    MACHINE_TIME_GET_NANO());
	mcu_signal_remove(SIG_MCU_LPM_CHANGE); 
	signal = mcu_signal_get();

	/* we were AM, we are going at least LPM0 */
	if (((prev_run_mode & 1) == 0) && ((curr_run_mode & 1) != 0))
	  {
	    etracer_slot_set_ns();
	  }

      }

  } while (MCU.signal == 0);

}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

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

void mcu_dump_stats(int64_t UNUSED user_nanotime)
{
  /* dump stats about MCU */
}


void mcu_state_save(void)
{
  memcpy(&mcu_backup,&mcu,sizeof(struct mcugen_mcu_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_state_restore(void)
{
  int old_run_mode = mcugen_running_mode();
  memcpy(&mcu,&mcu_backup,sizeof(struct mcugen_mcu_t));
  if (old_run_mode != mcugen_running_mode())
    {
      mcu_signal_add( SIG_MCU_LPM_CHANGE );
    }
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_print_description(void)
{
  OUTPUT("mcu   : %s\n",MCU_NAME);
  OUTPUT("model : %s\n",MCU_MODEL_NAME);
}


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


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int mcu_options_add(void)
{
  return 0;
}

uint32_t mcu_signal_get()
{
  return MCU.signal;
}

void mcu_signal_set(uint32_t s)
{
  MCU.signal = s;
}

void mcu_signal_add(uint32_t s)
{
  MCU.signal |= s;
}

void mcu_signal_remove(uint32_t s)
{
  MCU.signal &= ~s;
}

void mcu_signal_clr()
{
  MCU.signal = 0;
}

inline uint16_t mcu_get_pc(void)
{
  return 0; // MCU.regs[PC_REG_IDX];
}

inline uint16_t mcu_get_pc_next(void)
{
  return 0; // MCU.next_pc;
}

inline void mcu_set_pc_next(uint16_t UNUSED x)
{
  /* next instruction */
  // MCU_ALU.next_pc = x;
}

void mcu_reset(void)
{
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int mcu_registers_number()
{
  return MCU_REGISTERS;
}

uint16_t mcu_register_get(int i)
{
  return MCU_REGS[i];
}

void mcu_register_set(int i, uint16_t v)
{
  MCU_REGS[i] = v;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* mcu_regname_str(unsigned r)
{
  switch (r)
    {
    case  0: return "r0";
    case  1: return "r1";
    case  2: return "r2";
    case  3: return "r3";
    case  4: return "r4";
    case  5: return "r5";
    case  6: return "r6";
    case  7: return "r7";
    case  8: return "r8";
    case  9: return "r9";
    case 10: return "r10";
    case 11: return "r11";
    case 12: return "r12";
    case 13: return "r13";
    case 14: return "r14";
    case 15: return "r15";
    default: return "xx";
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

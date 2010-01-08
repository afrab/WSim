
/**
 *  \file   machine.c
 *  \brief  Platform Machine definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "machine/machine.h"
#include "libelf/libelf.h"
#include "libetrace/libetrace.h"
#include "liblogger/logger.h"

#include "machine_mon.h"
#include "machine.h"

/**
 * global variable
 **/
struct machine_t machine;
static elf32_t machine_elf = NULL;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void     machine_framebuffer_allocate (void);
void     machine_framebuffer_free     (void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_options_add()
{
  int res = 0;
  /* add options (MCU)                              */
  res += mcu_options_add();
  /* add all devices options = Board + peripherals) */
  res += devices_options_add();
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_create()
{
  int res = 0;

#if defined(DEBUG_MEMFOOTPRINT)
  HW_DMSG_MACH("machine: internal memory size is %d bytes\n",(int)sizeof(struct machine_t));
  HW_DMSG_MACH("  machine_info_t : %d\n",(int)sizeof(struct machine_info_t));
  HW_DMSG_MACH("  device_t       : %d\n",(int)sizeof(struct device_t));
  HW_DMSG_MACH("  tracer_t       : %d\n",(int)sizeof(tracer_t));
  HW_DMSG_MACH("  ui_t           : %d\n",(int)sizeof(struct ui_t));
#endif

  /* zero memory */
  machine.state                 = NULL;
  machine.state_backup          = NULL;
  machine.nanotime_incr         = 0;
  machine.device_max            = 0;
  memset(machine.device,      '\0',sizeof(struct device_t)*DEVICE_MAX);
  memset(machine.device_size, '\0',sizeof(int)*DEVICE_MAX);

  machine.backtrack                   = 0;
  machine.ui.framebuffer_background   = 0;

  /* create devices = mcu + peripherals */
  res += devices_create();
  machine_framebuffer_allocate();

  tracer_set_node_id(machine_get_node_id());
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_delete()
{
  int ret = 0;

  ret +=  devices_delete();
  machine_state_free();
  machine_framebuffer_free();
  libelf_close(machine_elf);

  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_reset()
{
  int res = 0;
  mcu_reset();
  res += devices_reset();
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
int machine_dump_mem(FILE* out, uint8_t *ptr, uint32_t addr, uint32_t size)
{
  int i;
  uint32_t col;
  uint32_t laddr;

#define DUMP_COLS 8

  for(laddr=addr; laddr < (addr+size);  laddr+=(2 * DUMP_COLS))
    {
      fprintf(out,"%04ux  ",(unsigned)laddr & 0xffff);

      for(i=0; i<2; i++)
	{
	  for(col = 0; col < DUMP_COLS; col ++)
	    {
	      fprintf(out,"%02x ",ptr[laddr + i*DUMP_COLS + col]);
	    }
	  fprintf(out," ");
	}

      fprintf(out,"|");
      for(col = 0; col < 2*DUMP_COLS; col ++)
	{
	  char c = ptr[laddr + col];
	  fprintf(out,"%c",(isprint(c) ? c : '.'));
	}
      fprintf(out,"|\n");

    }

  return 0;
}
*/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_dump(const char *filename)
{
  FILE *file;

  HW_DMSG_MACH("machine: dump state to file %s\n",filename);
  if ((file = fopen(filename, "wb")) == NULL)
    {
      ERROR("machine: cannot dump state to file %s\n",filename);
      return 1;
    }

  fclose(file);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_get_node_id(void)
{
  return worldsens_c_get_node_id();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_exit(int arg)
{
  REAL_STDERR("wsim: error, exit\n");
  exit(arg);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline uint32_t machine_run(void)
{
  uint32_t sig;
  /*
   *  HW_DMSG_MACH("machine: run()\n"); 
   */
  do {

    mcu_run();

    sig = mcu_signal_get();
    if ((sig & SIG_MAC) != 0)
      {
	mcu_signal_remove(SIG_MAC);
	if ((sig & MAC_TO_SIG(MAC_MUST_WRITE_FIRST)) != 0)
	  {
	    WARNING("wsim: ========================================\n");
	    WARNING("wsim: Read before Write from PC=0x%04x - 0x%x - %s\n", 
		    mcu_get_pc(),sig,mcu_signal_str());
	    WARNING("wsim: ========================================\n");
	    mcu_signal_remove(MAC_TO_SIG(MAC_MUST_WRITE_FIRST));
	    sig = mcu_signal_get();
	  }

	if ((sig & MAC_TO_SIG(MAC_WATCH_READ)) != 0)
	  {
	    mcu_signal_remove(SIG_MAC | MAC_TO_SIG(MAC_WATCH_READ));
	    /* trace (VCD) are working on writes */
	    /* machine_monitor_add_trace();      */
	    sig = mcu_signal_get();
	  }

	if ((sig & MAC_TO_SIG(MAC_WATCH_WRITE)) != 0)
	  {
	    mcu_signal_remove(SIG_MAC | MAC_TO_SIG(MAC_WATCH_WRITE));
	    machine_monitor_add_trace();
	    sig = mcu_signal_get();
	  }
      }

  } while (sig == 0);

  return sig;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

inline void machine_run_free(void)
{
  uint32_t sig;
  sig = machine_run();
  
  HW_DMSG_MACH("machine:run: stopped at 0x%04x with signal %s\n",mcu_get_pc(),mcu_signal_str());
  /*
   * Allowed outside tools signals
   *    SIG_GDB | SIG_CONSOLE | SIG_WORLDSENS_IO 
   */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define TEST_SIGNAL_EXIT(sig)						\
  ((sig & SIG_CON_IO)                           )  ||			\
  ((sig & SIG_WORLDSENS_KILL)                   )  ||			\
  ((sig & SIG_MCU)    && (sig & SIG_MCU_ILL)    )  ||			\
  ((sig & SIG_HOST)   && (sig & SIG_HOST_SIGNAL))

void TEST_SIGNAL_PRINT(char *name)
{
  OUTPUT("wsim:run:%s: exit at 0x%04x with signal 0x%x (%s)\n", 
	 name,mcu_get_pc(), mcu_signal_get(), mcu_signal_str());
  REAL_STDOUT("wsim:run:%s: exit at 0x%04x with signal 0x%x (%s)\n", 
	 name,mcu_get_pc(), mcu_signal_get(), mcu_signal_str());
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint64_t machine_run_insn(uint64_t insn)
{
  uint32_t sig;
  uint64_t i;
  HW_DMSG_MACH("machine: will run for %" PRIu64 " instructions\n",insn);

  mcu_signal_add(SIG_RUN_INSN);
  machine_state_save();

  for(i=0; i < insn; i++)
    {
      mcu_signal_add(SIG_RUN_INSN);
      (void)machine_run();
      mcu_signal_remove(SIG_RUN_INSN);

      sig = mcu_signal_get();
      if (sig)
	{
	  if (TEST_SIGNAL_EXIT(sig))
	    {
	      TEST_SIGNAL_PRINT("insn");
	      return i;
	    }
	}
    }
  return insn;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

wsimtime_t machine_run_time(wsimtime_t nanotime)
{
  uint32_t sig;
  HW_DMSG_MACH("machine: will run for %" PRIu64 " nano seconds\n",nanotime);

  mcu_signal_add(SIG_RUN_TIME);
  machine_state_save();

  while (MACHINE_TIME_GET_NANO() < nanotime)
    {
      mcu_signal_add(SIG_RUN_TIME);
      (void)machine_run();
      mcu_signal_remove(SIG_RUN_TIME);

      sig = mcu_signal_get();
      if (sig)
	{
	  if (TEST_SIGNAL_EXIT(sig))
	    {
	      TEST_SIGNAL_PRINT("time");
	      return MACHINE_TIME_GET_NANO();
	    }
	}
    }
  return MACHINE_TIME_GET_NANO();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_load_elf(const char* filename, int verbose_level)
{
  machine_elf = libelf_load(filename, verbose_level);
  return machine_elf == NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor(char* monitor, char *modify)
{
  if (machine_elf)
    {
      machine_monitor_init(machine_elf);

      if (monitor)
	{
	  machine_monitor_set(monitor, machine_elf);
	}

      if (modify)
	{
	  machine_modify_set(modify, machine_elf);
	}

      machine_monitor_start();
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_print_description()
{
  OUTPUT("== Machine description\n");
  mcu_print_description();
  devices_print();
  OUTPUT("==\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

inline wsimtime_t machine_get_nanotime()
{
  return MACHINE_TIME_GET_NANO();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint8_t* machine_state_allocate(int devices_size)
{
  uint8_t *ptr;
  int machine_size = sizeof(struct machine_state_t);

  /* machine.state_size_devices must be assigned */
  machine.state_size = machine_size + devices_size;

  printf("allocating %d bytes = %d + %d\n",machine.state_size, machine_size, devices_size);
  if ((machine.state        = (struct machine_state_t*)malloc(machine.state_size)) == NULL)
    {
      ERROR("** Could not allocate memory for devices states storage\n");
      return NULL;
    }

  if ((machine.state_backup = (struct machine_state_t*)malloc(machine.state_size)) == NULL)
    {
      machine_state_free();
      ERROR("** Could not allocate memory for devices states storage backup\n");
      return NULL;
    }

  memset(machine.state,        0, machine.state_size);
  memset(machine.state_backup, 0, machine.state_size);

  ptr = (uint8_t*)machine.state;
  return ptr + machine_size;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_state_free(void)
{
  if (machine.state != NULL)
    {
      free(machine.state);
      machine.state = NULL;
    }
  if (machine.state_backup != NULL)
    {
      free(machine.state_backup);
      machine.state_backup = NULL;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_state_save()
{
  /* mcu              */
  mcu_state_save();
  /* devices          */
  memcpy(machine.state_backup, machine.state, machine.state_size);
  /* libselect        */
  libselect_state_save();
  /* event tracer     */
  tracer_state_save();
  /* esimu tracer     */
  etracer_state_save();
  /* libwsnet         */
  worldsens_c_state_save();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_state_restore()
{
  /* mcu              */
  mcu_state_restore();
  /* devices          */
  memcpy(machine.state, machine.state_backup, machine.state_size);
  /* libselect        */
  libselect_state_restore();
  /* event tracer     */
  tracer_state_restore();
  /* esimu tracer     */
  etracer_state_restore();
  /* libwsnet         */
  worldsens_c_state_restore();
  /* count backtracks */
  machine.backtrack ++;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_framebuffer_allocate(void)
{
  int i,x,y,w,h;
  machine.ui.width  = 0;
  machine.ui.height = 0;
  machine.ui.bpp    = 3; // verif with ui.c
  
  for(i=0; i<machine.device_max; i++)
    {
      x = 0;
      y = 0;
      w = 0;
      h = 0;

      if (machine.device[i].ui_get_pos)
	{
	  machine.device[i].ui_get_pos (i,&x,&y);
	  if (machine.device[i].ui_get_size)
	    {
	      machine.device[i].ui_get_size(i,&w,&h);
	      machine.ui.height  = ((y+h) > machine.ui.height) ? (y+h) : machine.ui.height;
	      machine.ui.width   = ((x+w) > machine.ui.width)  ? (x+w) : machine.ui.width;
	    }
	}

    }

  machine.ui.framebuffer_size = machine.ui.width * machine.ui.height * machine.ui.bpp;
  //  VERBOSE(3,"machine:framebuffer: size %dx%dx%d\n",machine.ui.width, machine.ui.height, machine.ui.bpp);
  //  VERBOSE(3,"machine:framebuffer: size %d\n", machine.ui.framebuffer_size);
  //  VERBOSE(3,"machine:framebuffer: background 0x%08x\n",machine.ui.framebuffer_background);

  if (machine.ui.framebuffer_size > 0) 
    {
      uint8_t r,g,b;
      r = (machine.ui.framebuffer_background >> 16) & 0xff;
      g = (machine.ui.framebuffer_background >>  8) & 0xff;
      b = (machine.ui.framebuffer_background >>  0) & 0xff;
      //  VERBOSE(3,"machine:framebuffer:bg: 0x%02x%02x%02x\n",r,g,b);

      if ((machine.ui.framebuffer = (uint8_t*)malloc(machine.ui.framebuffer_size)) == NULL)
	{
	  HW_DMSG_DEV("Cannot allocate framebuffer\n");
	}
      for(i=0; i < (int)machine.ui.framebuffer_size; i+=machine.ui.bpp)
	{
	  setpixel(i, r,g,b);
	}
    }
}

/**************************************************/
/**************************************************/
/**************************************************/

void machine_framebuffer_free(void)
{
  if (machine.ui.framebuffer)
    {
      free(machine.ui.framebuffer);
    }
}

/**************************************************/
/**************************************************/
/**************************************************/

#define NANO  (1000*1000*1000)
#define MILLI (1000*1000)
#define MICRO (1000)

void machine_dump_stats(uint64_t user_nanotime)
{
  uint64_t s;
  uint64_t ms;
  uint64_t us;
  float speedup;

  s         = MACHINE_TIME_GET_NANO() / NANO;
  ms        = ( MACHINE_TIME_GET_NANO() - (s * NANO)) / MILLI;
  us        = ( MACHINE_TIME_GET_NANO() - (s * NANO)) / MICRO;

  OUTPUT("Machine stats:\n");
  OUTPUT("--------------\n");
  OUTPUT("  simulated time                : %"PRId64" ns (%"PRId64".%03"PRId64" s)\n", MACHINE_TIME_GET_NANO(),s,ms);
  if (user_nanotime > 0)
    {
      speedup = (float)MACHINE_TIME_GET_NANO() / (float)user_nanotime;
      OUTPUT("  simulation speedup            : %2.2f\n",speedup);
    }
  OUTPUT("  machine exit with signal      : %s\n",mcu_signal_str());

  OUTPUT("\n");

  OUTPUT("MCU:\n");
  OUTPUT("----\n");
  mcu_dump_stats(user_nanotime);

  OUTPUT("\n");

  OUTPUT("DEVICES:\n");
  OUTPUT("--------\n");
  devices_dump_stats(user_nanotime);

  OUTPUT("\n");
}

/**************************************************/
/**************************************************/
/**************************************************/

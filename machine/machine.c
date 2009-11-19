
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

/**
 * global variable
 **/
struct machine_t machine;
elf32_t machine_elf;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void     machine_framebuffer_allocate (void);
void     machine_framebuffer_free     (void);
void     machine_monitor_add_trace    (void);

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
  machine.nanotime        = 0;
  machine.nanotime_incr   = 0;
  machine.nanotime_backup = 0;
  machine.device_max      = 0;
  memset(machine.device,      '\0',sizeof(struct device_t)*DEVICE_MAX);
  memset(machine.device_size, '\0',sizeof(int)*DEVICE_MAX);

  machine.backtrack                   = 0;
  machine.ui.framebuffer_background   = 0;

  /* create devices = mcu + peripherals */
  res += devices_create();
  
  machine_framebuffer_allocate();

  /* machine_reset(); */

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

#define MONITOR_DEFAULT_SIZE        1
#define MONITOR_MAX_VARIABLE_NAME  50
#define MONITOR_MAX_WATCHPOINT     20

struct watchpoint_t {
  char        name[MONITOR_MAX_VARIABLE_NAME];
  uint32_t    addr;
  int         size; /* bytes */
  int         mode; /* MAC_WATCH_WRITE | MAC_WATCH_READ */
  tracer_id_t trc_id;
};

static struct watchpoint_t watchpoint[MONITOR_MAX_WATCHPOINT];
static int watchpoint_max;

void machine_monitor_print()
{
  int i;
  for(i=0; i<watchpoint_max; i++)
    {
      OUTPUT("  monitor: %s 0x%04x:%d %c%c\n",
	     watchpoint[i].name,
	     watchpoint[i].addr,
	     watchpoint[i].size,
	     (watchpoint[i].mode & MAC_WATCH_READ ) ? 'r':' ',
	     (watchpoint[i].mode & MAC_WATCH_WRITE) ? 'w':' '
	     );
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_vcd_start(void)
{
  int i;
  for(i=0; i<watchpoint_max; i++)
    {
      watchpoint[i].trc_id = tracer_event_add_id(
						 watchpoint[i].size * 8,
						 watchpoint[i].name,
						 "monitor"
						 );

      mcu_ramctl_set_bp(watchpoint[i].addr,watchpoint[i].mode);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_vcd_stop(void)
{
  int i;
  for(i=0; i<watchpoint_max; i++)
    {
      mcu_ramctl_unset_bp(watchpoint[i].addr,watchpoint[i].mode);
    }  
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_add_trace(void)
{
  int i,found = 0;
  uint32_t addr = MCU_RAMCTL_ADDR;
  for(i=0; i<watchpoint_max; i++)
    {
      if ((addr >= (watchpoint[i].addr)) &&
	  (addr <= (watchpoint[i].addr + watchpoint[i].size)))
	{
	  uint32_t value;
	  switch (watchpoint[i].size)
	    {
	    case 0:
	      value = mcu_jtag_read_byte(addr);
	      break;
	    case 1:
	      value = mcu_jtag_read_byte(addr);
	      break;
	    case 2:
	      value = mcu_jtag_read_word(addr & ~1); /* align read */
	      break;
	    default:
	      value = mcu_jtag_read_word(addr & ~1); /* align read */
	      break;
	    }

	  tracer_event_record(watchpoint[i].trc_id,value);

	  /*
	   * insert code for special purpose symbol here 
	   */

	  /*
	  VERBOSE(6,"machine:monitor: value changed watchpoint %s at 0x%04x=0x%04x\n",
		  watchpoint[i].name, watchpoint[i].addr, value);
 	  */	  


	  if (strcmp(watchpoint[i].name,"__WSIMLOGBUFFER") == 0)
	    {
	      int iter;
	      uint16_t int16;
	      char *logString = (char*)calloc( watchpoint[i].size, sizeof(char));
	      for(iter=0 ; iter<watchpoint[i].size ; iter++) 
		{
		  logString[iter] = (char)mcu_jtag_read_byte(addr+iter); 
		  if (logString[iter]=='\0') 
		    break;
		}

	      switch (logString[0])
		{
		case '1': // char
		  VERBOSE(6,"machine:monitor(0): Variable:%s at:0x%04x has become (char) '%c'\n",watchpoint[i].name,watchpoint[i].addr,logString[1]);
		  break;
		case '2': // Byte (int8)
		  VERBOSE(6,"machine:monitor(0): Variable:%s at:0x%04x has become (int8) '%d'\n",watchpoint[i].name,watchpoint[i].addr,logString[1]);
		  break;
		case '3': // Int (int16)
		  int16 = ((logString[2] & 0xff) << 8) | (logString[1] & 0xff);
		  VERBOSE(6,"machine:monitor(0): Variable:%s at:0x%04x has become (int16) '%d'\n",watchpoint[i].name,watchpoint[i].addr,int16);
		  break;
		case '4': // string
		  VERBOSE(6,"machine:monitor(0): Variable:%s at:0x%04x has become (str) '%s'\n",watchpoint[i].name,watchpoint[i].addr,&logString[1]);
		  break;
		default:
		  VERBOSE(6,"machine:monitor(0): Variable:%s at:0x%04x has become (Unknown type) '%s'\n",watchpoint[i].name,watchpoint[i].addr,iter,logString);
		  break;
		}
	      free(logString);
	    }

	  found = 1;
	}
    }
  if (found == 0)
    {
      WARNING("machine:monitor: watchpoint trace undefined for address 0x%04x\n",addr);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_read_arg(char *subtoken)
{
  if ((subtoken[0] >= '0') && (subtoken[0] <= '9'))
    {
      watchpoint[watchpoint_max].size = atoi(subtoken);
    }
  else
    {
      char c0 = tolower(subtoken[0]);
      char c1 = tolower(subtoken[1]);

      if (c0 == 'r' || c1 == 'r')
	watchpoint[watchpoint_max].mode |= MAC_WATCH_READ;

      if (c0 == 'w' || c1 == 'w')
	watchpoint[watchpoint_max].mode |= MAC_WATCH_WRITE;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_start(char* args)
{
  /* --monitor=0xAAAA:size:rw,symbol:rw ... */
  char delim1[] = ",";
  char delim2[] = ":";
  char *str1,*str2;
  char *token,*subtoken;
  char *saveptr1,*saveptr2;
  int  j;


  OUTPUT("==\n");
  OUTPUT("Memory monitor = %s\n",args);
  watchpoint_max = 0;

  for (j = 1, str1 = args; ; j++, str1 = NULL) 
    {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
	break;
    
      str2 = token;
      subtoken = strtok_r(str2, delim2, &saveptr2);
      if (subtoken == NULL) 
	{ 
	  /* error */
	  continue;
	}
      //OUTPUT("%s\n",subtoken);
      strncpy(watchpoint[watchpoint_max].name,subtoken,
	      MONITOR_MAX_VARIABLE_NAME);
      watchpoint[watchpoint_max].addr = 0;
      watchpoint[watchpoint_max].size = MONITOR_DEFAULT_SIZE;
      watchpoint[watchpoint_max].mode = 0;

      if (subtoken[0] == '0')
	{
	  /* hexa */
	  sscanf(subtoken, "0x%"SCNx32,& watchpoint[watchpoint_max].addr);
	}
      else
	{
	  watchpoint[watchpoint_max].addr = 
	    libelf_symtab_find_addr_by_name(machine_elf, subtoken );

	  watchpoint[watchpoint_max].size =
	    libelf_symtab_find_size_by_name(machine_elf, subtoken );

	  if ((watchpoint[watchpoint_max].addr == 0) &&
	      (watchpoint[watchpoint_max].size == -1))
	    {
	      ERROR("monitor: cannot find symbol \"%s\"\n",subtoken);
	    }
	}

      /*
      VERBOSE(6,"  monitor: probe \"%s\" = 0x%04x\n", watchpoint[watchpoint_max].name, 
	      watchpoint[watchpoint_max].addr);
      */
      subtoken = strtok_r(NULL, delim2, &saveptr2);
      //OUTPUT("  %s\n",subtoken);
      if (subtoken == NULL) 
	{
	  goto my_next_watchpoint;
	}
      machine_monitor_read_arg(subtoken);

      subtoken = strtok_r(NULL, delim2, &saveptr2);
      //OUTPUT("  %s\n",subtoken);
      if (subtoken == NULL) 
	{
	  goto my_next_watchpoint;
	}
      machine_monitor_read_arg(subtoken);

    my_next_watchpoint:
      watchpoint_max ++;
    }


  machine_monitor_print();
  machine_monitor_vcd_start();
  OUTPUT("==\n");
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

void machine_state_save()
{
  /* mcu              */
  mcu_state_save();
  machine.nanotime_backup = machine.nanotime;
  /* devices          */
  memcpy(machine.devices_state_backup, machine.devices_state, machine.devices_state_size);
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
  machine.nanotime = machine.nanotime_backup;
  /* devices          */
  memcpy(machine.devices_state, machine.devices_state_backup, machine.devices_state_size);
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

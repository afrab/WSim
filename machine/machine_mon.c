
/**
 *  \file   machine_mon.c
 *  \brief  Machine memory monitor
 *  \author Antoine Fraboulet
 *  \date   2009
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
#include "machine.h"
#include "machine_mon.h"

static struct watchpoint_t watchpoint[MONITOR_MAX_WATCHPOINT];
static int watchpoint_max;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static char* mode2str(int mode)
{
  switch (mode) {
  case MAC_WATCH_READ:  return "r";
  case MAC_WATCH_WRITE: return "w";
  case (MAC_WATCH_READ | MAC_WATCH_WRITE): return "rw";
  default: return "xx";
  }
}

void machine_monitor_print()
{
  int i;
  OUTPUT("==\n");
  OUTPUT("Memory monitor\n");
  for(i=0; i<watchpoint_max; i++)
    {
      OUTPUT("  monitor: %s 0x%04x:%d %2s mod %d %d\n",
	     watchpoint[i].name,
	     watchpoint[i].addr,
	     watchpoint[i].size,
	     mode2str(watchpoint[i].mode),
	     machine.state->watchpoint_modify_on_first_write[i],
	     watchpoint[i].modify_value
	     );
    }
  OUTPUT("==\n");
}
 
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_vcd_start(void)
{
  int i;
  for(i=0; i<watchpoint_max; i++)
    {
      char mns[MONITOR_MAX_VARIABLE_NAME+2];
      snprintf(mns,MONITOR_MAX_VARIABLE_NAME+2,"%s,%s", 
	       watchpoint[i].name,
	       mode2str(watchpoint[i].mode));

      watchpoint[i].trc_id_val = tracer_event_add_id(watchpoint[i].size * 8,
						     watchpoint[i].name,
						     "monitor" );

      watchpoint[i].trc_id_rw = tracer_event_add_id(2,mns,"monitor");

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

void machine_monitor_init(elf32_t UNUSED elf)
{
  int i;
  for(i=0; i<MONITOR_MAX_WATCHPOINT; i++)
    {
      memset(& watchpoint[i], 0, sizeof(struct watchpoint_t));
      watchpoint[i].size = -1;
    }
  watchpoint_max = 0;
}

void machine_monitor_start     (void)
{
  machine_monitor_print();
  machine_monitor_vcd_start();
}

void machine_monitor_stop      (void)
{
  machine_monitor_vcd_stop();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int machine_monitor_find_by_addr(uint32_t addr)
{
  int i;
  for(i=0; i<watchpoint_max; i++)
    {
      if ((addr >= (watchpoint[i].addr)) &&
	  (addr <  (watchpoint[i].addr + watchpoint[i].size)))
	{
	  return i;
	}
    }  
  return -1;
}

void machine_monitor_add_trace(int access_type)
{
  int      index = -1;
  uint32_t addr  = MCU_RAMCTL_ADDR;
  uint32_t value;

  if ((index = machine_monitor_find_by_addr( addr )) == -1)
    {
      WARNING("machine:monitor: watchpoint trace undefined for address 0x%04x\n",addr);
      return;
    }

  switch (watchpoint[index].size)
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

#if 0
  /* *******************************
   * available information example 
   *
   */

  VERBOSE(6,"machine:monitor: watchpoint trace for addr 0x%04x pc=0x%04x val=0x%04x type=%s\n",
	  addr, mcu_get_pc(), value, mode2str(access_type));
#endif

#if 0
  /* *******************************
   * example on how to set a breakpoint on value write (debugger write watchpoint)
   *
   */

  if ((strcmp(watchpoint[ index ].name,"my_watchpoint_name") == 0) ||
      (addr == my_watchpoint_address))
    {
      if (value == my_breakpoint_value)
	{
	  mcu_signal_add(SIG_MAC | MAC_TO_SIG(MAC_WATCH_WRITE));
	}
    }
#endif

  switch (access_type)
    {
    case MAC_WATCH_READ:
      tracer_event_record(watchpoint[ index ].trc_id_val,value);
      tracer_event_record_force(watchpoint[ index ].trc_id_rw,1);
      break;
    case MAC_WATCH_WRITE:
      tracer_event_record(watchpoint[ index ].trc_id_val,value);
      tracer_event_record_force(watchpoint[ index ].trc_id_rw,2);
      break;
    default:
      /* do nothing */
      break;
    }


  if (((access_type | MAC_WATCH_WRITE) != 0) && 
      (machine.state->watchpoint_modify_on_first_write[ index ]))
    {
      int v_addr  = watchpoint[index].addr;
      int v_size  = watchpoint[index].size;
      int v_value = watchpoint[index].modify_value;

      machine.state->watchpoint_modify_on_first_write[ index ] = 0;
      switch (v_size) {
      case 0:
	ERROR("  modify: variable \"%s\" [0x%04x] has size 0\n", 
	      watchpoint[index].name, v_addr);
	break;
      case 1:
	mcu_jtag_write_byte(v_addr, v_value);
	VERBOSE(6,"  modify:byte variable \"%s\" [0x%04x:%d] = %d (0x%04x)\n", 
		watchpoint[index].name, v_addr, v_size, v_value, v_value);
	break;
      case 2:
	mcu_jtag_write_word(v_addr, v_value);
	VERBOSE(6,"  modify:word variable \"%s\" [0x%04x:%d] = %d (0x%04x)\n", 
		watchpoint[index].name, v_addr, v_size, v_value, v_value);
	break;
      default:
	ERROR("  modify: variable \"%s\" [0x%04x] has a size > 2\n", 
	      watchpoint[index].name, v_addr);
	break;
      }
    }

  /*
   * Log messages from application: see svn:wsim/utils/wsimlog/WSimLog.h
   */
  if (((access_type | MAC_WATCH_WRITE) != 0) && 
      (strcmp(watchpoint[ index ].name,"__WSIMLOGBUFFER") == 0))
    {
      int iter;
      uint16_t int16;
      char *logString = (char*)calloc( watchpoint[ index ].size, sizeof(char));
      for(iter=0 ; iter<watchpoint[ index ].size ; iter++) 
	{
	  logString[iter] = (char)mcu_jtag_read_byte(addr+iter); 
	  if (logString[iter]=='\0') 
	    break;
	}
      
      switch (logString[0])
	{
	case 0  :
	case '0': // init (silent)
	  break;

	case '1': // char
	  VERBOSE(6,"machine:monitor(%d): Variable:%s at:0x%04x (char) = '%c'\n",
		  logString[0], watchpoint[ index ].name,watchpoint[ index ].addr,logString[1]);
	  break;
	case '2': // Byte (int8)
	  VERBOSE(6,"machine:monitor(%d): Variable:%s at:0x%04x (int8) = '%d'\n",
		  logString[0], watchpoint[ index ].name,watchpoint[ index ].addr,logString[1]);
	  break;
	case '3': // Int (int16)
	  int16 = ((logString[2] & 0xff) << 8) | (logString[1] & 0xff);
	  VERBOSE(6,"machine:monitor(%d): Variable:%s at:0x%04x (int16) = '%d'\n",
		  logString[0], watchpoint[ index ].name,watchpoint[ index ].addr,int16);
	  break;
	case '4': // string
	  VERBOSE(6,"machine:monitor(%d): Variable:%s at:0x%04x (str) = '%s'\n",
		  logString[0], watchpoint[ index ].name,watchpoint[ index ].addr,&logString[1]);
	  break;
	default:
	  VERBOSE(6,"machine:monitor(%d): Variable:%s at:0x%04x (Unknown type) = '%s'\n",
		  logString[0], watchpoint[ index ].name,watchpoint[ index ].addr,iter,logString);
	  break;
	}
      free(logString);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_monitor_set(char* args, elf32_t elf)
{
  /* --monitor=0xAAAA:size:rw,symbol:rw ... */
  char delim1[] = ",";
  char delim2[] = ":";
  char *str1,*str2;
  char *token,*subtoken;
  char *saveptr1,*saveptr2;
  int  j;


  //  OUTPUT("==\n");
  //  OUTPUT("Memory monitor = %s\n",args);

  for (j = 1, str1 = args; ; j++, str1 = NULL) 
    {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
	break;
    
      str2 = token;
      /* NAME or HEX */
      if ((subtoken = strtok_r(str2, delim2, &saveptr2)) == NULL)
	{ 
	  continue;
	}
      strncpy(watchpoint[watchpoint_max].name,subtoken, MONITOR_MAX_VARIABLE_NAME);

      /* HEXA:SIZE */
      if (subtoken[0] == '0')
	{
	  sscanf(subtoken, "0x%"SCNx32,& watchpoint[watchpoint_max].addr);
	  if ((subtoken = strtok_r(NULL, delim2, &saveptr2)) != NULL)
	    {
	      sscanf(subtoken, "%d", & watchpoint[watchpoint_max].size );
	    }
	}
      /* NAME */
      else 
	{
	  if (elf != NULL)
	    {
	      watchpoint[watchpoint_max].addr = libelf_symtab_find_addr_by_name(elf, subtoken );
	      watchpoint[watchpoint_max].size = libelf_symtab_find_size_by_name(elf, subtoken );
	    }
	  else
	    {
	      ERROR("monitor: cannot find symbol %s, no elf file given\n", subtoken);
	    }
	}

      if ((watchpoint[watchpoint_max].addr ==  0) && 
	  (watchpoint[watchpoint_max].size == -1))
	{
	  ERROR("monitor: cannot find symbol \"%s\"\n",subtoken);
	  continue;
	}
      else
	{
	  /* RW */
	  if ((subtoken = strtok_r(NULL, delim2, &saveptr2)) == NULL)
	    {
	      //continue;
	    }
	  else
	    {
	      char c0 = tolower((unsigned char)subtoken[0]);
	      char c1 = tolower((unsigned char)subtoken[1]);
	      
	      if (c0 == 'r' || c1 == 'r')
		watchpoint[watchpoint_max].mode |= MAC_WATCH_READ;
	      if (c0 == 'w' || c1 == 'w')
		watchpoint[watchpoint_max].mode |= MAC_WATCH_WRITE;
#if 0
	      OUTPUT("machine:monitor:add name %s addr %x size %d mode %c%c\n",
		      watchpoint[watchpoint_max].name, 
		      watchpoint[watchpoint_max].addr, 
		      watchpoint[watchpoint_max].size,
		      c0?c0:'.',c1?c1:'.');
#endif
	      watchpoint_max ++;
	    }
	}
    }

  //  OUTPUT("==\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void machine_modify_set(char* args, elf32_t elf)
{
  /* --modify=0xAAAA:size:value,symbol:value ... */
  char delim1[] = ",";
  char delim2[] = ":";
  char *str1,*str2;
  char *token,*subtoken;
  char *saveptr1,*saveptr2;
  int  j;

  char v_name[MONITOR_MAX_VARIABLE_NAME]; 
  int  v_size, v_addr, v_value; 

  //  OUTPUT("==\n");
  //  OUTPUT("Memory modify = %s\n",args);

  for (j = 1, str1 = args; ; j++, str1 = NULL) 
    {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
	break;
    
      str2 = token;
      /* NAME or hex */
      if ((subtoken = strtok_r(str2, delim2, &saveptr2)) == NULL)
	{
	  continue;
	}
      strncpy( v_name, subtoken, MONITOR_MAX_VARIABLE_NAME);

      v_addr = 0;
      v_size = -1;

      /* HEXA:SIZE */
      if (subtoken[0] == '0')
	{
	  sscanf(subtoken, "0x%"SCNx32, &v_addr );
	  if ((subtoken = strtok_r(NULL, delim2, &saveptr2)) != NULL)
	    {
	      sscanf(subtoken, "%d", & v_size );
	    }
	}
      /* NAME */
      else
	{
	  if (elf)
	    {
	      v_addr = libelf_symtab_find_addr_by_name(elf, subtoken );
	      v_size = libelf_symtab_find_size_by_name(elf, subtoken );
	    }
	  else
	    {
	      ERROR("monitor: cannot find symbol %s, no elf file given\n", subtoken);
	    }
	}

      if ((v_addr == 0) && 
	  (v_size == -1))
	{
	  ERROR("monitor: cannot find symbol \"%s\"\n",subtoken);
	}

      /* VALUE */
      if ((subtoken = strtok_r(NULL, delim2, &saveptr2)) == NULL)
	{
	  continue;
	}
      sscanf(subtoken, "%d", & v_value );

      int index = -1;
      if ((index = machine_monitor_find_by_addr(v_addr)) == -1)
	{
	  index = watchpoint_max++;
	  strncpy(watchpoint[ index ].name, v_name, MONITOR_MAX_VARIABLE_NAME);
	  watchpoint[ index ].addr = v_addr;
	  watchpoint[ index ].size = v_size;
	  watchpoint[ index ].mode = MAC_WATCH_WRITE;
	  OUTPUT("Memory modify = %s, add index %d\n",watchpoint[ index ].name,index);
	}
      else
	{
	  OUTPUT("Memory modify = %s, index %d\n",watchpoint[ index ].name,index);
	}
      machine.state->watchpoint_modify_on_first_write[index] = 1;
      watchpoint[index].modify_value                         = v_value;
    }
  //  OUTPUT("==\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

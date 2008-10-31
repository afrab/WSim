/**
 *  \file   console_cmd.c
 *  \brief  WSim Console utils functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "machine/machine.h"
#include "libselect/libselect.h"

#include "console_utils.h"
#include "console_cmd.h"

/* ************************************************** */
/* ************************************************** */

// #undef DEBUG

#if defined(DEBUG)
#define DBG_PRINT(x...) HW_DMSG(x)
#else
#define DBG_PRINT(x...) do { } while (0)
#endif

#define NOT_IMPLEMENTED ERROR("\nnot implemented\n\n")

/* ************************************************** */
/* ************************************************** */

void console_print_options(console_cmd_params p)
{
  int i;
  for(i=0; (i<CON_CMD_OPT_MAX) && (p->options[i] != NULL); i++)
    {
      DBG_PRINT("wsim:con:      arg %02d = %s\n",i,p->options[i]);
    }
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_help(console_cmd_params p)
{
  int i;
  DBG_PRINT("wsim:con:utils:help\n");
  console_write(p->cs,"\n");
  for(i=0; i < p->cs->cmd_max; i++)
    {
      console_write(p->cs,"%-11s : %s\n",p->cs->commands[i].name, p->cs->commands[i].text);
    }
  console_write(p->cs,"\n");
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_quit(console_cmd_params UNUSED p)
{
  DBG_PRINT("wsim:con:cmd:quit\n");
  return CON_CMD_QUIT;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_status(console_cmd_params UNUSED p)
{
  DBG_PRINT("wsim:con:cmd:status\n");
  console_print_options(p);
  machine_print_description();
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

void console_dump_section(console_cmd_params p, uint8_t *mbuf, long addr, long size)
{
#define DUMP_COLS 8
#define maxlines  8
#define COUT(x...) console_write(p->cs,x)

  int i;
  int line, col;
  for (line = 0; (line < maxlines) && ((line*2*DUMP_COLS) < size); line ++)
    {
      uint32_t laddr = addr + line * 2 * DUMP_COLS;
      COUT("%04x  ",laddr);
      
      for(i=0; i<2; i++)
	{
	  for(col = 0; col < DUMP_COLS; col ++)
	    {
	      COUT("%02x ",mbuf[ (line*2+i)*(DUMP_COLS) + col]);
	    }
	  COUT(" ");
	}

      COUT("|");
      for(col = 0; col < 2*DUMP_COLS; col ++)
	{
	  char c = mbuf[line * 2 * DUMP_COLS + col];
	  COUT("%c",(isprint(c) ? c : '.'));
	}
      COUT("|\n");
    }  

#undef maxlines
#undef DUMP_COLS
#undef COUT
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_show(console_cmd_params p)
{
  DBG_PRINT("wsim:con:cmd:show\n");
  console_print_options(p);
  
  if (p->nopt == 1)
    {
      if (strcmp(p->options[0],"verbose") == 0)
	{
	  console_write(p->cs,"verbode level %d\n",logger_verbose_level);
	}
      else if (strcmp(p->options[0],"machine") == 0)
	{
	  machine_print_description();
	}
      else if (strcmp(p->options[0],"pc") == 0)
	{
	  console_write(p->cs,"PC 0x%04x\n",mcu_get_pc());
	}
      else if (strcmp(p->options[0],"registers") == 0)
	{
	  int i;
	  console_write(p->cs,"registers\n");
	  for(i=0; i<mcu_registers_number(); i++)
	    {
	      console_write(p->cs," %3s : 0x%04x\n",
			    mcu_regname_str(i),
			    mcu_register_get(i) & 0xffffu);
	    }
	}
      else if (strcmp(p->options[0],"signal") == 0)
	{
	  console_write(p->cs,"signal 0x%08x\n",mcu_signal_get());
	}
    }
  else /* nopt > 1 */
    {
      if (strcmp(p->options[0],"ram") == 0) /* RAM */
	{
	  long addr;
	  long size;
#         define MBUF_SIZE 0x100
	  uint8_t mbuf[MBUF_SIZE];

	  if ((p->options[1] == NULL)                  ||
	      (console_htoi(p->options[1],&addr) != 0) ||
	      (addr < 0))
	    {
	      console_write(p->cs,"   show ram hex_addr, cannot parse command\n");
	      return CON_CMD_ERROR;
	    }

	  console_write(p->cs,"   show ram at address 0x%04x\n",addr);
	  if ((size = mcu_jtag_read_section(mbuf,addr,MBUF_SIZE)) != 0)
	    {
	      console_dump_section(p,mbuf,addr,size);
	    }
	}

      else if (strcmp(p->options[0],"flash") == 0) /* FLASH */
	{
	  long addr;
	  if ((p->options[1] == NULL)                  ||
	      (console_htoi(p->options[1],&addr) == 0) ||
	      (addr < 0))
	    {
	      console_write(p->cs,"   show flash hex_addr, cannot parse command\n");
	      return CON_CMD_ERROR;
	    }
	  console_write(p->cs,"   show flash at address 0x%04x\n",addr);
	  console_write(p->cs,"   not implemented\n");
	  NOT_IMPLEMENTED;
	}
    }
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_set(console_cmd_params p)
{
  DBG_PRINT("wsim:con:cmd:set\n");
  /* console_print_options(p); */

  if (p->nopt == 2)
    {
      if (strcmp(p->options[0],"verbose") == 0)
	{
	  long level;
	  if (console_atoi(p->options[1],&level))
	    {
	      console_write(p->cs,"error on last argument %s\n",p->options[2]);
	    }
	  else
	    {
	      DBG_PRINT("wsim:con:   set verbose level to %ld\n",level);
	      logger_verbose_level = level;
	    }
	}
      else
	{
	  console_write(p->cs,"unknown command %s\n",p->options[0]);
	}
    }
  else
    {
      console_write(p->cs,"error on set, wrong number of argument (%d)\n",p->nopt);
    }

  DBG_PRINT("wsim:con:cmd:set:end\n");
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_run (console_cmd_params p)
{
  DBG_PRINT("wsim:con:cmd:run\n");
  assert(libselect_fd_register(p->cs->fd_input, SIG_CON_IO) != -1);
  /* do something */
  machine_run_free();
  assert(libselect_fd_unregister(p->cs->fd_input) != -1);

  if ((mcu_signal_get() & SIG_CON_IO) == 0)
    {
      DBG_PRINT("wsim:con:cmd:run: exit without SIC_CON_IO signal - signal = 0x%x\n",mcu_signal_get());
      return CON_CMD_ERROR_SIG;
    }

  mcu_signal_remove(SIG_CON_IO);
  DBG_PRINT("wsim:con:cmd:run: return OK_SIG (0x%08x)\n",mcu_signal_get());
  return CON_CMD_OK_SIG;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_run_insn(console_cmd_params p)
{
  uint64_t n = atoll(p->options[0]);

  DBG_PRINT("wsim:con:cmd:run_insn: run for %"PRIu64" instructions\n",n);
  assert(libselect_fd_register(p->cs->fd_input, SIG_CON_IO) != -1);
  /* do something */
  machine_run_insn(n);
  assert(libselect_fd_unregister(p->cs->fd_input) != -1);

  if ((mcu_signal_get() & SIG_CON_IO))
    {
      mcu_signal_remove(SIG_CON_IO);
      DBG_PRINT("wsim:con:cmd:run_insn: exit with SIC_CON_IO signal\n");
      return CON_CMD_OK_SIG;
    }

  DBG_PRINT("wsim:con:cmd:run_insn: return OK\n");
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_run_time(console_cmd_params p)
{
  uint64_t n = atoll(p->options[0]);

  DBG_PRINT("wsim:con:cmd:run_time: run for %"PRIu64" ns\n",n);
  assert(libselect_fd_register(p->cs->fd_input, SIG_CON_IO) != -1);
  /* do something */
  machine_run_time(n);
  assert(libselect_fd_unregister(p->cs->fd_input) != -1);

  if ((mcu_signal_get() & SIG_CON_IO))
    {
      mcu_signal_remove(SIG_CON_IO);
      DBG_PRINT("wsim:con:cmd:run_insn: exit with SIC_CON_IO signal\n");
      return CON_CMD_OK_SIG;
    }

  DBG_PRINT("wsim:con:cmd:run_insn: return OK\n");
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_reset(console_cmd_params UNUSED p)
{
  DBG_PRINT("wsim:con:cmd:reset\n");
  machine_reset();
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

con_cmd_val console_cmd_loadelf(console_cmd_params UNUSED p)
{
  DBG_PRINT("wsim:con:cmd:loadelf\n");
  NOT_IMPLEMENTED;
  return CON_CMD_OK;
}

/* ************************************************** */
/* ************************************************** */

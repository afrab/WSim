/**
 *  \file   console.c
 *  \brief  WSim Console functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stdlib.h>

#include "config.h"
#include "liblogger/logger.h"
#include "libselect/libselect.h"

#include "arch/common/missing.h"
#include "console_utils.h"
#include "console_cmd.h"
#include "console.h"

/* ************************************************** */
/* ************************************************** */

// #undef DEBUG

#if defined(DEBUG)
#define DBG_PRINT(x...) fprintf(stdout,x)
#else
#define DBG_PRINT(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */

int console_cmd_add(console_state cs, console_cmd cmd)
{
  if (cs->cmd_max < CON_CMD_MAX)
    {
      memcpy(& (cs->commands[cs->cmd_max]), cmd, sizeof(struct console_cmd_t));
      cs->cmd_max ++;
      return 0;
    }
  else
    {
      ERROR("console: maximum command reached\n");
    }
  return 1;
}

/* ************************************************** */
/* ************************************************** */

static int console_init(console_state cs)
{
  struct console_cmd_t cmd_help     = { "help",    0, "help",    console_cmd_help    };
  struct console_cmd_t cmd_quit     = { "quit",    0, "quit",    console_cmd_quit    };
  struct console_cmd_t cmd_status   = { "status",  0, "status",  console_cmd_status  };
  struct console_cmd_t cmd_show     = { "show",    1, "show",    console_cmd_show    };
  struct console_cmd_t cmd_set      = { "set",     2, "set",     console_cmd_set     };

  struct console_cmd_t cmd_loadelf  = { "loadelf", 1, "loadelf", console_cmd_loadelf };

  struct console_cmd_t cmd_reset    = { "reset",   0, "reset",   console_cmd_reset   };
  struct console_cmd_t cmd_run      = { "run",     0, "run",     console_cmd_run     };
  struct console_cmd_t cmd_run_insn = { "run-insn",1, "run-insn",console_cmd_run_insn};
  struct console_cmd_t cmd_run_time = { "run-time",1, "run-time",console_cmd_run_time};



  memset(cs,0,sizeof(struct console_state_t));

  console_cmd_add(cs,&cmd_help);
  console_cmd_add(cs,&cmd_quit);
  console_cmd_add(cs,&cmd_status);
  console_cmd_add(cs,&cmd_show);
  console_cmd_add(cs,&cmd_set);
  console_cmd_add(cs,&cmd_loadelf);

  console_cmd_add(cs,&cmd_reset);
  console_cmd_add(cs,&cmd_run);
  console_cmd_add(cs,&cmd_run_insn);
  console_cmd_add(cs,&cmd_run_time);

  strncpyz(cs->ps1,"wsim > ",CON_PS1_MAX);

  cs->fd_input  = 0;
  cs->fd_output = 1;

  return 0;
}

/* ************************************************** */
/* ************************************************** */

static int console_ui_delim(char c)
{
  return (c == ' ' || c == '\t' || c == 10 || c == 13 || c == '\n');
}

/* ************************************************** */
/* ************************************************** */

#define MAX_REQ     1024
#define MAX_TOK     CON_CMD_OPT_MAX
#define MAX_TOKLEN  256

static int get_tokens(const char str[MAX_REQ], char tok[MAX_TOK][MAX_TOKLEN], int (*test_delim)(char))
{
  int i     = 0;
  int token = 0;
  int index = 0;

  while (str[i] && token < MAX_TOK) 
    {
      /* strip spaces */
      while (str[i] && test_delim(str[i])) {
	i++;
      }

      /* copy token */
      index = 0;
      while (str[i] && !test_delim(str[i]) && index < MAX_TOKLEN) 
	{
	  tok[token][index++] = str[i++];
	}

      /* null terminate token */
      if (index)
	{
	  tok[token++][index] = '\0';
	}
    }
  return token;
}

/* ************************************************** */
/* ************************************************** */

static int console_command(console_state cs)
{
  int i,o;
  int ntokens;
  char req[MAX_REQ];
  char tokens[MAX_TOK][MAX_TOKLEN];

  struct console_cmd_params_t p;
  memset(&p,0,sizeof(struct console_cmd_params_t));
  p.cs = cs;

  switch (console_read(cs,req,sizeof(req))) 
    {
    case -1:
      console_write(cs,"   cannot read command\n");
      return CON_QUIT;
    case 0:
      return CON_CMD_NONE;
    default:
      // DBG_PRINT("wsim:con:main: read -%s-\n",req);
      break;
    }

  if ((ntokens = get_tokens(req,tokens,console_ui_delim)) == 0)
    {
      return console_cmd_help(&p);
    }
  
  for(i=0; i < cs->cmd_max ; i++)
    {
      if (strcasecmp(cs->commands[i].name,tokens[0]) == 0)
	{
	  if (cs->commands[i].nb_option > (ntokens - 1))
	    {
	      console_write(cs,"  incorrect number of arguments for %s\n",tokens[0]);
	      return CON_CMD_ERROR;
	    }
	  for(o=0; o < ntokens; o++)
	    {
	      p.options[o] = tokens[o+1];
	    }
	  
	  p.nopt = ntokens - 1;
	  return cs->commands[i].fun(&p);
	}
    }

  console_write(cs,"   unknown command %s\n\n",tokens[0]);

  return CON_CMD_ERROR;
}

/* ************************************************** */
/* ************************************************** */

int console_mode_main(void)
{
  struct console_state_t cs;
  
  //  DBG_PRINT("wsim:con:main: enter\n");
  console_init(&cs);
  //  DBG_PRINT("wsim:con:main:init done\n");

  console_write(&cs,cs.ps1);
  while (1)
    {
      switch (console_command(&cs))
	{
	case CON_CMD_NONE:
	  console_write(&cs,cs.ps1); /* node number */
	  break;
	case CON_CMD_OK:
	  DBG_PRINT("wsim:con:main: return OK\n");
	  console_write(&cs,cs.ps1); /* node number */
	  break;
	case CON_CMD_OK_SIG:
	  DBG_PRINT("wsim:con:main: return OK_SIG\n");
	  break;
	case CON_CMD_ERROR:
	  DBG_PRINT("wsim:con:main: return ERROR\n");
	  console_write(&cs,cs.ps1); /* add node number */
	  break;
	case CON_CMD_ERROR_SIG:
	  DBG_PRINT("wsim:con:main: return ERROR_SIG\n");
	  break;
	case CON_CMD_QUIT:
	  DBG_PRINT("wsim:con:main:exit\n");
	  return CON_QUIT;
	  break;
	}
    }

  return CON_ERROR;
}

/* ************************************************** */
/* ************************************************** */

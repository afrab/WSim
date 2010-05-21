
/**
 *  \file   tracer_bin.c
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2010
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>   /* basename */
#include <inttypes.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>

#ifndef WSNET3
#include "arch/common/hardware.h"
#include "liblogger/logger.h"
#include "src/options.h"
#else
#include <config.h>
#endif

#include "tracer.h"
#include "tracer_int.h"
#include "tracer_vcd.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static FILE* tracer_datafile;
#define VCDOUT(x...)  fprintf(tracer_datafile,x)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_vcd_open(char* filename)
{
  if ((tracer_datafile = fopen(filename,"wb")) == NULL)
    {
      ERROR("tracer: ***********************************\n");
      ERROR("tracer: %s\n",strerror(errno));
      ERROR("tracer: ***********************************\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_vcd_close()
{
  if (tracer_datafile)
    {
      fclose(tracer_datafile);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* long long to char* binary representation */
char*
tracer_lldbin(tracer_val_t v)
{
  int i;
  static char s[65];
  
  memset(s,'0',sizeof(s));
  s[64] = '\0';
  for(i=0; i < 63; i++)
    {
      s[63-i] = '0' +  ((v >> i) & 0x1);
    }
  return s;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * VCD file header
 *
 * date
 * version
 * timsescale
 * FIXED_TIME is used for regression tests 
 */

static void vcd_dump_header()
{
  VCDOUT("$date\n");
#define NO_FIXED_TIME
#if defined(FIXED_TIME)
  VCDOUT("   May 13, 2005\t14:25:30"); 
#else
  {
    char outstr[200];
    time_t time_value;
    struct tm *tmp;
    
    time_value = time(NULL);
    tmp = localtime(&time_value);
    if (tmp == NULL)
      {
	strcpy(outstr,"   May 13, 2005\t14:25:30");
      }
    else
      {
	if (strftime(outstr, sizeof(outstr), "  %B %d, %Y\t%T", tmp) == 0) 
	  {
	    strcpy(outstr,"   May 13, 2005\t14:25:30");
	  }
      }
    VCDOUT("%s\n",outstr);
  }
#endif

  VCDOUT("$end\n");
  VCDOUT("\n");
  VCDOUT("$version %s $end\n",PACKAGE_VERSION);
  VCDOUT("\n");
  VCDOUT("$timescale 1ns $end\n");
  VCDOUT("\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void numid_to_str(char *str, int num, int id)
{
  sprintf(str,"%c%c%c%c",
	  ((num / 25) % 25) + 'a', (num % 25) + 'a',
	  ((id  / 25) % 25) + 'a', (id  % 25) + 'a');
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static char tracer_id_done[TRACER_MAX_ID];
static char tracer_id_var [TRACER_MAX_ID][10];

void vcd_dump_modules()
{
  tracer_id_t  mod;
  tracer_id_t  id;

  for(mod=0; mod < TRACER_MAX_ID; mod ++)
    {
      if ((tracer_id_done[mod] == 0) &&
	  (strcmp(tracer_id_module[mod],"") != 0))
	{
	  VCDOUT("$scope module %s $end\n", tracer_id_module[mod]);
	  for(id=mod; id < TRACER_MAX_ID; id ++)
	    {
	      if ((tracer_id_done[id] == 0) &&
		  (strcmp(tracer_id_module[mod],tracer_id_module[id]) == 0))
		{
		    numid_to_str(tracer_id_var[id], mod, id);
		    VCDOUT("    $var integer %d %s %s $end\n", 
			   tracer_width[id], tracer_id_var[id], tracer_id_name[id]);
		    tracer_id_done[id] = 1;
		}
	    }
	  VCDOUT("$upscope $end\n\n");
	}
    }
}

void vcd_dump_variables()
{
  tracer_id_t     id;
  for(id=0; id < TRACER_MAX_ID; id ++)
    {
      if ((tracer_id_done[id] == 0) && (strcmp(tracer_id_name[id],"") != 0))
	{
	  numid_to_str(tracer_id_var[id], TRACER_MAX_ID, id);
	  VCDOUT("$var integer %d %s %s $end\n", 
		  tracer_width[id], tracer_id_var[id], tracer_id_name[id]);
	}
    }
}

void vcd_dump_scopes()
{
  int i;
  for(i=0;i<TRACER_MAX_ID;i++) 
    tracer_id_done[i] = 0;

  vcd_dump_modules   ();
  vcd_dump_variables ();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void vcd_dump_init_vars()
{
  tracer_id_t     id;

  VCDOUT("$dumpvars\n");
  for(id = 0; id < TRACER_MAX_ID; id++)
    {
      if (tracer_id_name[id][0] != '\0')
	{
	  VCDOUT("b%s %s\n",tracer_lldbin(0),tracer_id_var[id]);
	}
    }
  VCDOUT("$end\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_vcd_start()
{
  vcd_dump_header();                                    /* header             */
  vcd_dump_scopes();                                    /* scopes             */
  VCDOUT("$enddefinitions $end\n\n");                   /* header end         */
  VCDOUT("$comment\n  %s\n$end\n\n", PACKAGE_STRING);   /* comments           */
  vcd_dump_init_vars();                                 /* dumpvars init to 0 */
  VCDOUT("\n\n\n");                                     /* start data         */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_vcd_dump_data()
{
  tracer_ev_t      i;
  tracer_time_t    curr_time = 0;
  tracer_sample_t *curr_smpl;

  curr_smpl = tracer_buffer;
  for(i=0; i<EVENT_TRACER.ev_count; i++)
    {
      if (curr_time != curr_smpl->time)
	{
	  VCDOUT("\n#%" PRId64 "\n", curr_smpl->time);
	}
      VCDOUT("b%s %s\n", tracer_lldbin(curr_smpl->val), tracer_id_var[curr_smpl->id]);
      curr_time = curr_smpl->time;
      curr_smpl ++;
      // id_last_smpl[curr_smpl.id] = curr_smpl;
    }
  EVENT_TRACER.ev_count = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_vcd_finish()
{
  /* nothing to do */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

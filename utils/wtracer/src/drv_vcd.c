
/**
 *  \file   drv_vcd.h
 *  \brief  Tracer VCD driver
 *  \author Antoine Fraboulet
 *  \date   2006
 **/


#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "log.h"
#include "wsim_endian.h"
#include "tracer.h"
#include "drv_raw.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define VCD_TRC_MAX 200

static int       vcd_trc_count;
static tracer_t *vcd_traces[VCD_TRC_MAX];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_init    (tracer_t *t);
int drv_vcd_process (tracer_t *t);
int drv_vcd_finalize(tracer_t *t);

tracer_driver_t tracer_driver_vcd = 
  { 
    .name     = "vcd",
    .ext      = ".vcd",
    .init     = drv_vcd_init,     /* init           */
    .process  = drv_vcd_process,  /* process 1 file */
    .finalize = drv_vcd_finalize  /* finalize       */
  };

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* long long to char* binary representation */
static char*
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

static void vcd_dump_header(tracer_t *t)
{
  fprintf(t->out_fd,"$date\n");
#define NO_FIXED_TIME
#if defined(FIXED_TIME)
  fprintf(t->out_fd,"   May 13, 2005\t14:25:30"); 
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
    fprintf(t->out_fd,"%s\n",outstr);
  }
#endif

  fprintf(t->out_fd,"$end\n");
  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"$version %s $end\n",PACKAGE_VERSION);
  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"$timescale 1ns $end\n");
  fprintf(t->out_fd,"\n");
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

/**
 * find all modules listed in a TRC header
 **/
void vcd_find_modules(tracer_t UNUSED *t_out, tracer_t *t, int UNUSED tnum)
{
  tracer_id_t     id;

  t->modmax = 0;
  memset(t->id_module, '\0',sizeof(t->id_module));
  memset(t->id_var,    '\0',sizeof(t->id_var));

  for(id=0; id < TRACER_MAX_ID; id++)
    {
      if (t->hdr.id_module[id][0] != '\0')
	{
	  int mod;
	  int found = 0;

	  /* search in list of already seen modules */
	  for(mod=0; mod < t->modmax; mod++)
	    {
	      if (strcmp(t->hdr.id_module[id], t->id_module[mod]) == 0)
		{
		  found = 1;
		}
	    }

	  if (found == 0)
	    {
	      DMSG(t,"          new module %s\n", t->hdr.id_module[id]);
	      strcpy(t->id_module[t->modmax++], t->hdr.id_module[id]);
	    }
	}
    }
}


void vcd_dump_modules(tracer_t *t_out, tracer_t *t, int tnum)
{
  int  mod;
  tracer_id_t     id;
  for(mod=0; mod < t->modmax; mod++)
    {
      fprintf(t_out->out_fd,"$scope module %s $end\n", t->id_module[mod]);
      for(id=0; id < TRACER_MAX_ID; id ++)
	{
	  if (t->hdr.id_name[id][0] != '\0')
	    {
	      if (strcmp(t->hdr.id_module[id],t->id_module[mod]) == 0)
		{
		  numid_to_str(t->id_var[id], tnum, id);
		  fprintf(t_out->out_fd,"    $var integer %d %s %s $end\n", 
			  t->hdr.id_width[id], t->id_var[id], t->hdr.id_name[id]);
		}
	    }
	}
      fprintf(t_out->out_fd,"$upscope $end\n\n");
    }
}


void vcd_dump_variables(tracer_t *t_out, tracer_t* t, int tnum)
{
  tracer_id_t     id;
  for(id=0; id < TRACER_MAX_ID; id ++)
    {
      if ((t->hdr.id_name[id][0] != '\0') && (t->id_var[id][0] == '\0'))
	{
	  numid_to_str(t->id_var[id], tnum, id);
	  fprintf(t_out->out_fd,"$var integer %d %s %s $end\n", 
		  t->hdr.id_width[id], t->id_var[id], t->hdr.id_name[id]);
	}
    }
}


/**
 * splits a name
 *
 **/

static void tr_split_name(char* dst, char *src)
{
  int i;
  for(i=0; src[i] != '\0'; i++)
    {
      char c = src[i];
      switch (src[i])
	{
	case '.': c = '-'; break;
	case '_': c = '-'; break;
	default :          break;
	}
      dst[i] = c;
    }
  dst[i] = '\0';
}

/**
 * VCD file scopes
 *
 * if multiple files are merged a scope for each
 * file is created.
 *  
 * for each file: 
 *    1 find list of all modules.
 *    2 dump each modules and their signals
 *    3 dump variables
 **/

void vcd_dump_scopes(tracer_t *t, tracer_t *trc[], int nb_trc_files)
{
  int i;
  for(i=0; i<nb_trc_files; i++)
    {
      char module_name[FILENAME_MAX];

      DMSG(t,"tracer:vcd: dump scopes for file %s (%d/%d)\n",trc[i]->in_filename,i,nb_trc_files);

      if (nb_trc_files > 1)
	{
	  tr_split_name(module_name,trc[i]->in_filename);
	  fprintf(t->out_fd,"$scope module %s $end\n\n", module_name);
	}

      DMSG(t,"   vcd: find modules\n");
      vcd_find_modules   (t, trc[i], i);
      DMSG(t,"   vcd: dump modules\n");
      vcd_dump_modules   (t, trc[i], i);
      DMSG(t,"   vcd: dump variables\n");
      vcd_dump_variables (t, trc[i], i);

      /* */
      if (nb_trc_files > 1)
	{
	  fprintf(t->out_fd,"\n$upscope $end\n");
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void vcd_dump_init_vars(tracer_t *t, tracer_t *trc[], int nb_trc_files)
{
  int i;
  tracer_id_t     id;

  fprintf(t->out_fd,"$dumpvars\n");
  for(i=0; i < nb_trc_files; i++)
    {
      for(id = 0; id < TRACER_MAX_ID; id++)
	{
	  if (trc[i]->hdr.id_name[id][0] != '\0')
	    fprintf(t->out_fd,"b%s %s\n",tracer_lldbin(0),trc[i]->id_var[id]);
	}
    }
  fprintf(t->out_fd,"$end\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_process_file(tracer_t *t)
{
  tracer_ev_t     ev;
  tracer_time_t   curr_time = 0;
  tracer_sample_t curr_smpl;
  tracer_sample_t id_last_smpl[TRACER_MAX_ID];
  int             is_first_sample;

  vcd_dump_header(t);                                              /* header             */
  vcd_dump_scopes(t,&t,1);                                         /* scopes             */

  fprintf(t->out_fd,"\n");                                         /* header end         */
  fprintf(t->out_fd,"$enddefinitions $end\n\n");                   /* header end         */
  fprintf(t->out_fd,"$comment\n  %s\n$end\n\n", PACKAGE_STRING);   /* comments           */

  vcd_dump_init_vars(t,&t,1);                                      /* dumpvars init to 0 */
  fprintf(t->out_fd,"\n\n\n");                                     /* start data         */

  /* data */

  if (t->hdr.ev_count_total > 0)
    {
      is_first_sample = 1;
      
      for(ev=1; ev < t->hdr.ev_count_total; ev++)
	{
	  tracer_read_sample(t,&curr_smpl);

	  if ((t->start_time <= curr_smpl.time) && (curr_smpl.time <= t->stop_time))
	    {
	      if (is_first_sample)
		{
		  fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
		  fprintf(t->out_fd,"b%s %s\n", tracer_lldbin(curr_smpl.val), t->id_var[curr_smpl.id]);
		  id_last_smpl[curr_smpl.id] = curr_smpl;
		  
		  is_first_sample = 0;
		}
	      else
		{
		  if (id_last_smpl[curr_smpl.id].val != curr_smpl.val)
		    {
		      if (curr_smpl.time != curr_time)
			fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
		      
		      fprintf(t->out_fd,"b%s %s\n",tracer_lldbin(curr_smpl.val), t->id_var[curr_smpl.id]);
		      id_last_smpl[curr_smpl.id] = curr_smpl;
		    }
		}
	    }
	  curr_time = curr_smpl.time;
	}
    }
  return 0;
}

/* ************************************************** */
/* ** MERGE MODE ************************************ */
/* ************************************************** */

int drv_vcd_trc_get_next_evtrc(tracer_t *trc[], int nbtrc, 
			       int *ev_valid, tracer_ev_t *ev_num, tracer_sample_t *ev_smpl)
{
  int i;
  int min_trc            = -1;
  tracer_time_t min_time = DEFAULT_STOP_TIME;

  /* reload events */
  for(i=0; i < nbtrc; i++)
    {
      switch (ev_valid[i])
	{
	case -1: /* eof */
	  break;
	case 0:  /* read a sample */
	  if (ev_num[i] < trc[i]->hdr.ev_count_total)
	    {
	      if (tracer_read_sample(trc[i],& (ev_smpl[i])) == 0)
		{
		  ERROR("read sample on file %s, ev %d\n",trc[i]->in_filename,ev_num[i]);
		  return -1;
		}
	      ev_smpl [i].time += trc[i]->hdr.initial_time;
	      ev_valid[i]       = 1;
	      ev_num  [i]       ++;
	    }
	  else
	    {
	      ev_valid[i] = -1;
	    }
	  break;
	case 1:  /* sample available */
	  break;
	default:
	  break;
	}
    }

  /* find first event */
  for(i=0; i < nbtrc; i++)
    {
      if (ev_valid[i] == 1)
	{
	  if (ev_smpl[i].time <= min_time)
	    {
	      min_trc = i;
	      min_time = ev_smpl[i].time;
	    }
	}
    }

  return min_trc;
}

static int drv_vcd_dump_merge(tracer_t *t, tracer_t *trc[], int nbtrc)
{
  int itrc;
  int ntotal;
  tracer_time_t   curr_time;                                 /* current time             */
  tracer_sample_t curr_smpl;                                 /* current sample           */
  int             ev_valid    [VCD_TRC_MAX];                 /* there is a valid ev in I */
  tracer_ev_t     ev_number   [VCD_TRC_MAX];                 /* current ev on file I     */
  tracer_sample_t ev_smpl     [VCD_TRC_MAX];                 /* current sample on file I */
  tracer_sample_t id_last_smpl[VCD_TRC_MAX][TRACER_MAX_ID];  /* last seen id value in I  */

  memset(ev_valid,     0, sizeof(ev_valid));
  memset(ev_number,    0, sizeof(ev_number));
  memset(ev_smpl,      0, sizeof(ev_smpl));
  memset(id_last_smpl, 0, sizeof(id_last_smpl));

  /* header */

  vcd_dump_header(t);                                              /* header             */
  vcd_dump_scopes(t,trc,nbtrc);                                    /* scopes             */
  fprintf(t->out_fd,"$enddefinitions $end\n\n");                   /* header end         */
  fprintf(t->out_fd,"$comment\n  %s\n$end\n\n", PACKAGE_STRING);   /* comments           */
  vcd_dump_init_vars(t,trc,nbtrc);                                 /* dumpvars init to 0 */
  fprintf(t->out_fd,"\n\n\n");                                     /* start data         */

  /* data */

  ntotal = 0;
  curr_time = 0;
  while ((itrc = drv_vcd_trc_get_next_evtrc(trc, nbtrc, ev_valid, ev_number, ev_smpl)) != -1)
    {
      curr_smpl = ev_smpl[itrc];
      ev_valid[itrc] = 0;       /* need to be refilled */

      if (ntotal == 0)
	{
	  fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
	  fprintf(t->out_fd,"b%s %s\n", tracer_lldbin(curr_smpl.val), trc[itrc]->id_var[curr_smpl.id]);
	  id_last_smpl[itrc][curr_smpl.id] = curr_smpl;
	  curr_time = curr_smpl.time;
	}
      else
	{
	  if (id_last_smpl[itrc][curr_smpl.id].val != curr_smpl.val)
	    {
	      if (curr_smpl.time != curr_time)
		{
		  fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
		}
	      fprintf(t->out_fd,"b%s %s\n", tracer_lldbin(curr_smpl.val), trc[itrc]->id_var[curr_smpl.id]);
	      id_last_smpl[itrc][curr_smpl.id] = curr_smpl;
	      ntotal ++;
	    }
	}
      curr_time = curr_smpl.time;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_init    (tracer_t *t)
{
  DMSG(t,"tracer:drv:vcd: init\n");
  if (t->merge)
    {
      vcd_trc_count = 0;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_process(tracer_t *t)
{
  int ret;
  if (t->merge)
    {
      if (vcd_trc_count == VCD_TRC_MAX)
	{
	  ERROR("tracer:drv:vcd: maximum number of source files reached\n");
	  return 1;
	}
      vcd_traces[vcd_trc_count] = tracer_create();
      if (tracer_dup(vcd_traces[vcd_trc_count],t) != 0)
	{
	  tracer_delete(vcd_traces[vcd_trc_count]);
	  return 1;
	}
      vcd_trc_count++;
      ret = 0;
    }
  else
    {
      ret = drv_vcd_process_file(t);
    }
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_finalize(tracer_t *t)
{
  int i;
  DMSG(t,"tracer:drv:vcd: finalize\n");
  if (t->merge)
    {
      drv_vcd_dump_merge(t, vcd_traces, vcd_trc_count); 
      for (i=0; i < vcd_trc_count; i++)
	{
	  fclose(vcd_traces[i]->in_fd);
	  tracer_delete(vcd_traces[i]);
	}
    }
  else
    {
      /* nothing to do */
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

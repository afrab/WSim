
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
#include "endian.h"
#include "tracer.h"
#include "drv_raw.h"


int drv_vcd_init    (tracer_t *t);
int drv_vcd_process (tracer_t *t);
int drv_vcd_finalize(tracer_t *t);

tracer_driver_t tracer_driver_vcd = 
  { 
    .name     = "vcd",
    .init     = drv_vcd_init,
    .process  = drv_vcd_process,
    .finalize = drv_vcd_finalize
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

int drv_vcd_process(tracer_t *t)
{
  tracer_id_t     id;
  tracer_ev_t     ev;
  tracer_time_t   curr_time;
  tracer_sample_t curr_smpl;
  char            id_var [TRACER_MAX_ID][6];
  tracer_sample_t id_last[TRACER_MAX_ID];

  DMSG(t,"tracer:drv:vcd: process %s\n",t->in_filename);
  if (tracer_file_out_open(t,".vcd") != 0)
    {
      DMSG(t,"tracer:drv:vcd: open out error\n");
      return 1;
    }

  memset(id_var, '\0',sizeof(id_var));
  memset(id_last,'\0',sizeof(id_last));

  /* *** */
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
  fprintf(t->out_fd,"$scope module WSim $end\n");
  fprintf(t->out_fd,"\n");




#if defined(NO_SUBMODULE)
  for(id=0; id < TRACER_MAX_ID; id ++)
    {
      sprintf(id_var[id],"%c%c%c",(id / 100) + 'a', ((id / 10) % 10) + 'a', (id % 10) + 'a');
      if (t->hdr.id_name[id][0] != '\0')
	{
	  fprintf(t->out_fd,"$var integer %d %s %s $end\n", t->hdr.id_width[id], id_var[id], t->hdr.id_name[id]);
	}
    }
#else
  {
    int  mod;
    int  modmax = 0;
    char id_module [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
    for(id=0; id < TRACER_MAX_ID; id++)
      {
	if (t->hdr.id_name[id][0] != '\0')
	  {
	    char *ptr;
	    char name[TRACER_MAX_NAME_LENGTH];
	    strncpy(name,t->hdr.id_name[id],TRACER_MAX_NAME_LENGTH);
	    ptr = strtok(name,"_.");
	    DMSG(t,"vcd: looking for module %s in %s\n",ptr,t->hdr.id_name[id]);
	    if ((ptr != NULL) && (strcmp(ptr,t->hdr.id_name[id]) != 0))
	      {
		int mod;
		int found = 0;
		for(mod=0; mod<modmax; mod++)
		  {
		    if (strcmp(ptr,id_module[mod]) == 0)
		      {
			found = 1;
		      }
		  }
		if (found == 0)
		  {
		    DMSG(t,"vcd: find module %s\n",ptr);
		    strcpy(id_module[modmax++], ptr);
		  }
	      }
	  }
      }

    for(mod=0; mod<modmax; mod++)
      {
	fprintf(t->out_fd,"$scope module %s $end\n",id_module[mod]);
	for(id=0; id < TRACER_MAX_ID; id ++)
	  {
	    if (t->hdr.id_name[id][0] != '\0')
	      {
		char *ptr;
		char name[TRACER_MAX_NAME_LENGTH];
		strncpy(name,t->hdr.id_name[id],TRACER_MAX_NAME_LENGTH);
		ptr = strtok(name,"_.");
		if (strcmp(ptr,id_module[mod]) == 0)
		  {
		    sprintf(id_var[id],"%c%c%c", mod + 'a', ((id / 10) % 10) + 'a', (id % 10) + 'a');
		    fprintf(t->out_fd,"    $var integer %d %s %s $end\n", t->hdr.id_width[id], id_var[id], t->hdr.id_name[id]);
		  }
	      }
	  }
	fprintf(t->out_fd,"$upscope $end\n\n");
      }

    for(id=0; id < TRACER_MAX_ID; id ++)
      {
	if ((t->hdr.id_name[id][0] != '\0') && (id_var[id][0] == '\0'))
	  {
	    sprintf(id_var[id],"%c%c%c", mod + 'a', ((id / 10) % 10) + 'a', (id % 10) + 'a');
	    fprintf(t->out_fd,"$var integer %d %s %s $end\n", t->hdr.id_width[id], id_var[id], t->hdr.id_name[id]);
	  }
      }
  }

#endif

  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"$upscope $end\n");
  fprintf(t->out_fd,"$enddefinitions $end\n");
  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"$comment\n");
  // fprintf(t->out_fd,"  feu rouge le rouge, feu vert la bière !!\n"); 
  fprintf(t->out_fd,"  %s\n",PACKAGE_STRING); 
  fprintf(t->out_fd,"$end\n");
  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"$dumpvars\n");
  for(id = 0; id < TRACER_MAX_ID; id++)
    {
      if (t->hdr.id_name[id][0] != '\0')
	fprintf(t->out_fd,"b%s %s\n",tracer_lldbin(0),id_var[id]);
    }
  fprintf(t->out_fd,"$end\n");

  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"\n");
  fprintf(t->out_fd,"\n");

  if (t->hdr.ev_count_total > 0)
    {
      tracer_read_sample(t,&curr_smpl);

      fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
      fprintf(t->out_fd,"b%s %s\n",tracer_lldbin(curr_smpl.val), id_var[curr_smpl.id]);
      id_last[curr_smpl.id] = curr_smpl;
      curr_time = curr_smpl.time;

      for(ev=1; ev < t->hdr.ev_count_total; ev++)
	{
	  tracer_read_sample(t,&curr_smpl);
	  
	  if (id_last[curr_smpl.id].val != curr_smpl.val)
	    {
	      if (curr_smpl.time != curr_time)
		fprintf(t->out_fd,"\n#%" PRId64 "\n", curr_smpl.time);
	      
	      fprintf(t->out_fd,"b%s %s\n",tracer_lldbin(curr_smpl.val), id_var[curr_smpl.id]);
	      id_last[curr_smpl.id] = curr_smpl;
	    }
	  curr_time = curr_smpl.time;
	}
    }

  tracer_file_out_close(t);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_init    (tracer_t *t)
{
  DMSG(t,"tracer:drv:vcd: init\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_vcd_finalize(tracer_t *t)
{
  DMSG(t,"tracer:drv:vcd: finalize\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

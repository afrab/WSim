
/**
 *  \file   drv_sitc.c
 *  \brief  Sense In The Citi driver
 *  \author Tanguy Risser
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "log.h"
#include "endian.h"
#include "tracer.h"
#include "drv_sitc.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SITC_TRC_MAX 200

static int       sitc_trc_count;
static tracer_t *sitc_traces[SITC_TRC_MAX];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_sitc_init    (tracer_t *t);
int drv_sitc_process (tracer_t *t);
int drv_sitc_finalize(tracer_t *t);

tracer_driver_t tracer_driver_sitc = 
  { 
    .name     = "sitc",
    .init     = drv_sitc_init,
    .process  = drv_sitc_process,
    .finalize = drv_sitc_finalize
  };

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* copy everything except file descriptors */
static int sitc_tracer_copy(tracer_t *dest,tracer_t *source)
{

  memcpy(dest,source,sizeof(struct tracer_struct_t));
  dest->in_fd     = fdopen(dup(fileno(source->in_fd)),"r");
  dest->out_fd    = NULL;
  dest->file_mode = 1;
  dest->dir       = NULL;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int end_of_traces(tracer_t **trc_traces,tracer_ev_t *ev_count,int nb_trc_files)
{
  int i;

  for (i=0; i<nb_trc_files; i++)
    {
      if (ev_count[i]<trc_traces[i]->hdr.ev_count_total)
	return(0);
    }
  return(1);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static tracer_ev_t select_next_sample(tracer_t *t,tracer_ev_t ev_count,int id,tracer_sample_t *s)
{
  tracer_ev_t count;

  count = ev_count;
  while(count<t->hdr.ev_count_total)
    {
      tracer_read_sample(t,s);
      count++;
      if (s->id == id)
	{
	  return count;
	}
    }
  /* trace ended */
  return count;
      
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void drv_sitc_dump_signal(tracer_t *t, tracer_t *trc_traces[],int nb_trc_files,int id)
{
  tracer_time_t       time;
  tracer_sample_t    *s;
  tracer_ev_t        *ev_count;
  int i;
  int selected_trace = 0;

  s = (tracer_sample_t *)malloc(nb_trc_files*sizeof(tracer_sample_t));
  ev_count = (tracer_ev_t *)malloc(nb_trc_files*sizeof(tracer_ev_t));

  for (i=0; i<nb_trc_files; i++)
    {
      ev_count[i]=0;
    }

  time=DEFAULT_STOP_TIME;
  for (i=0; i<nb_trc_files; i++)
    {
      ev_count[i]=select_next_sample(trc_traces[i],ev_count[i],id,&s[i]);
      /* select the earliest event */
      if ((ev_count[i]<trc_traces[i]->hdr.ev_count_total) && (s[i].time<time))
	{
	  selected_trace=i;
	  time=s[i].time;
	}
    }

  while(!end_of_traces(trc_traces,ev_count,nb_trc_files))
    {
      fprintf(t->out_fd,
	      " %d %d  %" PRId64 " %" PRId64 "\n",
	      selected_trace,
	      id,
	      s[selected_trace].time,
	      s[selected_trace].val);

      ev_count[selected_trace]=
	select_next_sample(trc_traces[selected_trace],
			   ev_count[selected_trace],
			   id,&s[selected_trace]);

      /* select the earliest next event to be recorded */
      time=DEFAULT_STOP_TIME;
      for (i=0; i<nb_trc_files; i++)
	{
	  if ((ev_count[i]<trc_traces[i]->hdr.ev_count_total) && 
	       (s[i].time<time))
	    {
	      selected_trace=i;
	      time=s[i].time;
	    }
	}

    }

  free(s);
  free(ev_count);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void drv_sitc_dump(tracer_t *t, tracer_t *trc_traces[],int nb_trc_files)
{
  tracer_id_t   id;
  int i,id_count;

  fprintf(t->out_fd,"============================\n");
  fprintf(t->out_fd,"= idExpe:%d \n",1);
  fprintf(t->out_fd,"= nbSensor:%d\n",t->hdr.node_id);
  fprintf(t->out_fd,"============================\n");

  for(id=0; id < TRACER_MAX_ID; id++)
    {
      /* TODO only the all option is implemented currently */
      for (i=0; i<nb_trc_files; i++)
	{
	  tracer_file_rewind(trc_traces[i]);
	}

      if (t->hdr.id_name[id][0] != '\0' && 
	  ((strcmp(t->out_signal_name,"all") == 0) || 
	   (strcmp(t->out_signal_name,t->hdr.id_name[id]) == 0)))
	{
	  fprintf(t->out_fd,"============================\n");
	  fprintf(t->out_fd,"= id:%s %3d 0x%02x\n",t->hdr.id_name[id],id,id);

	  id_count=0;
	  for (i=0; i<nb_trc_files; i++)
	    {
	      id_count+=trc_traces[i]->hdr.id_count[id];
	    }

	  fprintf(t->out_fd,"= count: %d\n",id_count);
	  fprintf(t->out_fd,"= min: %" PRId64 "\n",t->hdr.id_val_min[id]);
	  fprintf(t->out_fd,"= max: %" PRId64 "\n",t->hdr.id_val_max[id]);
	  fprintf(t->out_fd,"= syntax:  idCapteur idMeasure time value\n");
	  fprintf(t->out_fd,"============================\n");

	  drv_sitc_dump_signal(t,trc_traces,nb_trc_files,id);
	}
    }

}

  
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_sitc_process(tracer_t *t)
{
  DMSG(t,"tracer:drv:sitc process %s\n",t->in_filename);

  if (sitc_trc_count == SITC_TRC_MAX)
    {
      ERROR("tracer:drv:sitc: maximum number of source files reached\n");
      return 0;
    }

  /* second fill the filename array */
  sitc_traces[sitc_trc_count] = tracer_create();
  if (sitc_tracer_copy(sitc_traces[sitc_trc_count],t) != 0)
    {
      tracer_delete(sitc_traces[sitc_trc_count]);
      return 1;
    }

  sitc_trc_count++;
  return 0;
}
  
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_sitc_init(tracer_t *t)
{
  DMSG(t,"tracer:drv:sitc: init\n");
  if (tracer_file_out_open(t,".sitc") != 0)
    {
      DMSG(t,"tracer:drv:gplot: open out error\n");
      return 1;
    }
  DMSG(t,"tracer:drv:sitc: open %s\n",t->out_filename);
  sitc_trc_count = 0;
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_sitc_finalize(tracer_t *t)
{
  int i;
  DMSG(t,"tracer:drv:sitc finalize\n");
  /* four: close all the traces  */
  drv_sitc_dump(t,sitc_traces,sitc_trc_count);
  for (i=0; i < sitc_trc_count; i++)
    {
      if (sitc_traces[i]->in_fd)
	{
	  fclose(sitc_traces[i]->in_fd);
	}
      tracer_delete(sitc_traces[i]);
    }
  tracer_file_out_close(t);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

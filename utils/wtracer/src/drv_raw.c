
/**
 *  \file   drv_raw.c
 *  \brief  Tracer raw driver
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "endian.h"
#include "tracer.h"
#include "drv_raw.h"

int drv_raw_init    (tracer_t *t);
int drv_raw_process (tracer_t *t);
int drv_raw_finalize(tracer_t *t);

tracer_driver_t tracer_driver_raw = 
  { 
    .name     = "raw",
    .ext      = ".raw",
    .init     = drv_raw_init,
    .process  = drv_raw_process,
    .finalize = drv_raw_finalize
  };

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint64_t drv_raw_bitmask(int size)
{
  int i;
  uint64_t bm = 0;
  for(i=0; i<size; i++)
    {
      bm = (bm << 1) | 1;
    }
  return bm;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void drv_raw_dump_signal(tracer_t *t, tracer_id_t id)
{
  uint64_t bitmask;
  tracer_ev_t   ev;
  tracer_sample_t s;

  bitmask = drv_raw_bitmask(t->hdr.id_width[id]);

  fprintf(t->out_fd,"id:%s\n",t->hdr.id_name[id]);
  fprintf(t->out_fd,"count: %d\n",t->hdr.id_count[id]);
  fprintf(t->out_fd,"size: %d\n",t->hdr.id_width[id]);
  fprintf(t->out_fd,"mask: %" PRIu64 "\n",bitmask);
  fprintf(t->out_fd,"min: %" PRIu64 "\n",t->hdr.id_val_min[id]);
  fprintf(t->out_fd,"max: %" PRIu64 "\n",t->hdr.id_val_max[id]);
  fprintf(t->out_fd,"\n");

  for(ev=0; ev < t->hdr.ev_count_total; ev++)
    {
      tracer_read_sample(t,&s);
      if (s.id == id)
	{
	  fprintf(t->out_fd,"%" PRIu64 ": %" PRIu64 "\n",
		  s.time, s.val & bitmask);
	}
    }
  fprintf(t->out_fd,"\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_raw_process(tracer_t *t)
{
  tracer_id_t   id;

  fprintf(t->out_fd,"# ============================\n");
  for(id=0; id < TRACER_MAX_ID; id++)
    {
      if (t->hdr.id_name[id][0] != '\0' && 
	  ((strcmp(t->out_signal_name,"all") == 0) || 
	   (strcmp(t->out_signal_name,t->hdr.id_name[id]) == 0)))
	{
	  tracer_file_in_rewind(t);
	  drv_raw_dump_signal(t,id);
	  fprintf(t->out_fd,"# ============================\n");
	}
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_raw_init(tracer_t *t)
{
  DMSG(t,"tracer:drv:raw: init\n");
  if (t->merge)
    {
      ERROR("tracer:drv:raw: merge mode is not supported\n");
      return 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_raw_finalize(tracer_t *t)
{
  DMSG(t,"tracer:drv:raw: finalize\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

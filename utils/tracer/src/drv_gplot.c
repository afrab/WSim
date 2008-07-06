
/**
 *  \file   drv_gplot.c
 *  \brief  Tracer gplot driver
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "endian.h"
#include "tracer.h"
#include "drv_raw.h"

int drv_gplot_init    (tracer_t *t);
int drv_gplot_process (tracer_t *t);
int drv_gplot_finalize(tracer_t *t);

tracer_driver_t tracer_driver_gplot = 
  { 
    .name     = "gplot",
    .init     = drv_gplot_init,
    .process  = drv_gplot_process,
    .finalize = drv_gplot_finalize
  };

tracer_time_t inline min(tracer_time_t t1, tracer_time_t t2)
{
  return t1 < t2 ? t1 : t2;
}

tracer_time_t inline max(tracer_time_t t1, tracer_time_t t2)
{
  return t1 < t2 ? t2 : t1;
}



/*
            1-------
   0 -------        0------------
*/

static void
tracer_dump_gplot_id(tracer_t *t, int id)
{
  int nplot = 0;

  tracer_time_t     t1;
  tracer_time_t     t2;

  tracer_time_t     xrange_min;
  tracer_time_t     xrange_max;
  tracer_time_t     yrange_min;
  tracer_time_t     yrange_max;

  tracer_val_t      val;
  tracer_ev_t       i;
  tracer_ev_t       left  = 0;
  tracer_ev_t       right = 0;

  tracer_sample_t   left_sample;
  tracer_sample_t   right_sample;

  /* time is supposed to be given in nano */
#define FACTOR 1000000.0
#define TITLE  "time (ms)"

  if (t->hdr.ev_count_total > 0)
    {
      xrange_min = max(0,t->start_time);
      xrange_max = min(t->hdr.sim_time_total,t->stop_time);
    }
  else
    {
      xrange_min = 0;
      xrange_max = FACTOR;
    }

  yrange_min = t->hdr.id_val_min[id];
  yrange_max = t->hdr.id_val_max[id] + 1;

  fprintf(t->out_fd,"set encoding iso_8859_1\n");
  fprintf(t->out_fd,"set style fill solid .5 noborder\n");
  fprintf(t->out_fd,"set datafile separator whitespace\n");
  fprintf(t->out_fd,"set grid\n");
  fprintf(t->out_fd,"set style line 1 linetype 1 lw 1 pt 1 ps 1\n");
  fprintf(t->out_fd,"set style line 2 linetype 2 lw 1 pt 0 ps 0\n");
  fprintf(t->out_fd,"set style line 3 linetype 1 lw 5.0 pt 0 ps 0\n");
  fprintf(t->out_fd,"unset autoscale\n");
  fprintf(t->out_fd,"set xrange [%g:%g]\n",xrange_min / FACTOR,xrange_max / FACTOR);
  fprintf(t->out_fd,"set yrange [%"PRId64":%"PRId64"]\n",yrange_min,yrange_max);
  fprintf(t->out_fd,"set xlabel \"%s\"\n",TITLE);
  fprintf(t->out_fd,"set ylabel \"%s\"\n",t->hdr.id_name[id]);
  fprintf(t->out_fd,"set terminal postscript eps\n");
  fprintf(t->out_fd,"set size ratio 0.5\n");
  fprintf(t->out_fd,"set output \"%s.%s.eps\"\n",t->out_filename,t->hdr.id_name[id]);

  fprintf(t->out_fd,"plot ");
  /* first pass counts the number of segments */
  nplot = t->hdr.id_count[id];
  for(i=0; i < nplot; i++)
    {
      fprintf(t->out_fd," \"-\" notitle with linespoints ls 3");
      if (i == (nplot - 1))
	fprintf(t->out_fd,"\n");
      else
	fprintf(t->out_fd,", \\\n");
    }

  
  /* find first ref */
  for(left = 0; left < t->hdr.ev_count_total; left++)
    {
      tracer_read_sample(t,&left_sample);
      if (left_sample.id == id)
	break;
    }

  assert(left_sample.time == 0); // first event time should be 0

  /* outputs segment data */
  for(right=left+1 ; right < t->hdr.ev_count_total; right++)
    {
      tracer_read_sample(t,&right_sample);
      if (right_sample.id == id)
	{
	  t1  = left_sample.time  / FACTOR;
	  t2  = right_sample.time / FACTOR;
	  val = left_sample.val;
	  fprintf(t->out_fd,"%g %g\n%g %g\ne\n",(double)t1,(double)val,(double)t2,(double)val);
	  left = right;
	  left_sample = right_sample;
	}
    }

  /* outputs last segment */
  if (left_sample.time <= t->hdr.sim_time_total)
    {
      t1  = left_sample.time      / FACTOR;
      t2  = t->hdr.sim_time_total / FACTOR;
      val = left_sample.val;
      fprintf(t->out_fd,"%g %g\n%g %g\ne\n",(double)t1,(double)val,(double)t2,(double)val);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_gplot_init(tracer_t *t)
{
  DMSG(t,"tracer:drv:gplot: init\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_gplot_process(tracer_t *t)
{
  tracer_id_t   id;
  DMSG(t,"tracer:drv:gplot: process %s\n",t->in_filename);
  if (tracer_file_out_open(t,".gp") != 0)
    {
      DMSG(t,"tracer:drv:gplot: open out error\n");
      return 1;
    }

  for(id=0; id < TRACER_MAX_ID; id++)
    {
      if (t->hdr.id_name[id][0] != '\0' && 
	  ((strcmp(t->out_signal_name,"all") == 0) || 
	   (strcmp(t->out_signal_name,t->hdr.id_name[id]) == 0)))
	{
	  tracer_file_rewind(t);
	  tracer_dump_gplot_id(t,id);
	}
    }

  tracer_file_out_close(t);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int drv_gplot_finalize(tracer_t *t)
{
  DMSG(t,"tracer:drv:gplot: finalize\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

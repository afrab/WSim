
/**
 *  \file   tracer_main.c
 *  \brief  Worlsens tracer main file
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>

#include "log.h"
#include "endian.h"
#include "tracer.h"
#include "drv_raw.h"
#include "drv_vcd.h"
#include "drv_gplot.h"
#include "drv_sitc.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void usage(char* prog)
{
  fprintf(stdout,"usage: %s           \n\
  --in=filename    default: wsim.trc  \n\
  --out=filename   default: wsim.fmt  \n\
  --dir=directory  default: .         \n\
  --signal=name    default: all       \n\
  --format=name    default: raw       \n\
  --begin=time     default: 0         \n\
  --end=time       default: max       \n\
  --debug          default: no        \n\
  --merge          default: no        \n\
  --help                              \n", prog);

  exit(1);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int options_parse(tracer_t *t, int argc, char* argv[])
{
  int c;
  
  strncpy(t->in_filename,  "wsim.trc", FILENAME_MAX);
  strncpy(t->in_Dir,       ".",        FILENAME_MAX);
  strncpy(t->out_filename, "wsim.fmt", FILENAME_MAX);
  t->file_mode       = 0;
  t->dir_mode        = 0;
  t->out_signal_name = "all";
  t->out_format_name = "raw";
  t->start_time      = 0;
  t->stop_time       = DEFAULT_STOP_TIME;
  t->merge           = 0;
#ifdef DEBUG
  t->debug           = 1;
#else
  t->debug           = 0;
#endif

  while (1)
    {
      // int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] =
	{
	  {"in",      required_argument, 0, 'i'},
	  {"out",     required_argument, 0, 'o'},
	  {"dir",     required_argument, 0, 'd'},
	  {"format",  required_argument, 0, 'f'},
	  {"signal",  required_argument, 0, 's'},
	  {"begin",   required_argument, 0, 'b'},
	  {"end",     required_argument, 0, 'e'},
	  {"merge",   no_argument,       0, 'm'},
	  {"debug",   no_argument,       0, 'D'},
	  {"help",    no_argument,       0, 'h'},   
	  {0, 0, 0, 0}
	};
      
      c = getopt_long (argc, argv, "i:o:d:f:s:b:e:Dmh",
		       long_options, &option_index);
      if (c == -1)
	break;
      
      switch (c)
	{
	case 'i':
	  strncpy(t->in_filename,optarg,FILENAME_MAX);
	  t->file_mode       = 1;
	  break;
	case 'o':
	  strncpy(t->out_filename,optarg,FILENAME_MAX);
	  break;
	case 'd':
	  strncpy(t->in_Dir,optarg,FILENAME_MAX);
	  t->dir_mode        = 1;
	  break;
	case 'f':
	  t->out_format_name = optarg;
	  break;
	case 's':
	  t->out_signal_name = optarg;
	  break;
	case 'b':
	  t->start_time      = atoll(optarg);
	  break;
	case 'e':
	  t->stop_time       = atoll(optarg);
	  break;
	case 'm':
	  t->merge           = 1;
	case 'D':
	  t->debug           = 1;
	  break;
	case 'h':
	  usage(argv[0]);
	  break;
	  
	case '?':
	  break;

	default:
	  ERROR("tracer: unknown option %d %c %s\n", c, c, long_options[option_index].name);
	}
    } /* while(1) */

  if ((t->file_mode == 0) && (t->dir_mode == 0))
    {
      DMSG(t,"tracer:opt: defaults to file mode\n");
      t->file_mode = 1;
    }

  return 0; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void options_print(tracer_t *t)
{
  DMSG(t,"tracer:opt: in filename  : %s\n",t->in_filename);
  DMSG(t,"tracer:opt: in directory : %s\n",t->in_Dir);
  DMSG(t,"tracer:opt: out filename : %s\n",t->out_filename);
  DMSG(t,"tracer:opt: out format   : %s\n",t->out_format_name);
  DMSG(t,"tracer:opt: signal name  : %s\n",t->out_signal_name);
  DMSG(t,"tracer:opt: start time   : %"PRId64"\n",t->start_time);
  DMSG(t,"tracer:opt: stop time    : %"PRId64"\n",t->stop_time);
  DMSG(t,"tracer:opt: merge        : %d\n",t->merge);
  DMSG(t,"tracer:opt: debug        : %d\n",t->debug);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int options_validation(tracer_t *t)
{
  if (t->file_mode && t->dir_mode)
    {
      ERROR("tracer: cannot combine --in and --dir option\n");
      return 1;
    }

  if ((t->stop_time != DEFAULT_STOP_TIME) && (t->stop_time <= t->start_time))
    {
      ERROR("tracer: end time before start time\n");
      return 2;
    }

  if (t->merge && !t->dir_mode)
    {
      ERROR("tracer: must combine --merge with --dir\n");
      return 3;
    }

  if (t->merge)
    {
      ERROR("tracer: merge is not yet implemented\n");
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int main(int argc, char* argv[])
{
  tracer_t        *trc;
  tracer_driver_t *drv;

  tracer_driver_register(&tracer_driver_raw);
  tracer_driver_register(&tracer_driver_vcd);
  tracer_driver_register(&tracer_driver_gplot);
  tracer_driver_register(&tracer_driver_sitc);

  trc = tracer_create();

  options_parse(trc, argc, argv);
  options_print(trc);

  if (options_validation(trc))
    {
      ERROR("tracer: option validation error\n");
      return 1;
    }
  
  if ((drv = tracer_driver_get_by_name(trc->out_format_name)) != NULL)
    {
      if (trc->file_mode)
	{
	  DMSG(trc,"tracer: file mode\n");
	  if (tracer_file_open(trc) == 0)
	    {
	      if (drv->init(trc) == 0)
		{
		  if (drv->process(trc) == 0)
		    {
		      drv->finalize(trc);
		    }
		}
	      tracer_file_close(trc);
	    }
	}
      else
	{
	  if (tracer_dirmode_init(trc,".trc") == 0)
	    {
	      DMSG(trc,"tracer: dir mode, scanning *.trc files\n");
	      drv->init(trc);
	      while(tracer_dirmode_next(trc) == 0)
		{
		  if (tracer_file_open(trc) == 0)
		    {
		      drv->process(trc);
		      tracer_file_close(trc);
		    }
		}
	      drv->finalize(trc);
	      tracer_dirmode_close(trc);
	    }
	}
    }
  else
    {
      ERROR("tracer: format %s not registered\n",trc->out_format_name);
    }

  tracer_delete(trc);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

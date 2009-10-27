
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
  --dir=directory  default: .         \n\
  --multi          default: wsim.trc  \n\
  --out=filename   default: wsim.fmt  \n\
  --signal=name    default: all       \n\
  --format=name    default: raw       \n\
  --begin=time     default: 0         \n\
  --end=time       default: max       \n\
  --debug          default: no        \n\
  --merge          default: no        \n\
  --help                              \n\
\n\
  ex:\n\
    wtracer --in=wsim.trc --out=wsim.vcd --format=vcd\n\
    wtracer --out=glob.vcd --format=vcd --merge --multi wsim*.trc\n", prog);

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
  t->mode            = TRC_NONE;
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
	  {"begin",   required_argument, 0, 'b'},
	  {"debug",   no_argument,       0, 'D'},
	  {"dir",     required_argument, 0, 'd'},
	  {"end",     required_argument, 0, 'e'},
	  {"format",  required_argument, 0, 'f'},
	  {"help",    no_argument,       0, 'h'},   
	  {"in",      required_argument, 0, 'i'},
	  {"merge",   no_argument,       0, 'M'},
	  {"multi",   no_argument,       0, 'm'},
	  {"out",     required_argument, 0, 'o'},
	  {"signal",  required_argument, 0, 's'},
	  {0, 0, 0, 0}
	};
      
      c = getopt_long (argc, argv, "i:o:d:f:s:b:e:Dmh",
		       long_options, &option_index);
      if (c == -1)
	{
	  return optind;
	}
      
      switch (c)
	{
	case 'i':
	  if (t->mode != TRC_NONE) {
	    ERROR("tracer: cannot combine --in, --dir and --multi options\n");
	    return 1;
	  }
	  strncpy(t->in_filename,optarg,FILENAME_MAX);
	  t->mode            = TRC_FILE;
	  break;
	case 'm':
	  if (t->mode != TRC_NONE) {
	    ERROR("tracer: cannot combine --in, --dir and --multi options\n");
	    return 1;
	  }
	  t->mode            = TRC_MULTI;
	  break;
	case 'd':
	  if (t->mode != TRC_NONE) {
	    ERROR("tracer: cannot combine --in, --dir and --multi options\n");
	    return 1;
	  }
	  strncpy(t->in_Dir,optarg,FILENAME_MAX);
	  t->mode            = TRC_DIR;
	  break;
	case 'o':
	  strncpy(t->out_filename,optarg,FILENAME_MAX);
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
	case 'M':
	  t->merge           = 1;
	  break;
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

  return argc;
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
  if ((t->stop_time != DEFAULT_STOP_TIME) && (t->stop_time <= t->start_time))
    {
      ERROR("tracer: end time before start time\n");
      return 2;
    }

  if (t->merge && (t->mode == TRC_FILE))
    {
      ERROR("tracer: must combine --merge with --dir or --multi\n");
      return 3;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ERR_EXIT(x...)				\
  do {						\
  ERROR(x);					\
  exit(1);					\
  } while (0)

int main(int argc, char* argv[])
{
  int              next_arg;
  tracer_t        *trc;
  tracer_driver_t *drv;

  tracer_driver_register(&tracer_driver_raw);
  tracer_driver_register(&tracer_driver_vcd);
  tracer_driver_register(&tracer_driver_gplot);
  tracer_driver_register(&tracer_driver_sitc);

  trc = tracer_create();

  next_arg = options_parse(trc, argc, argv);
  options_print(trc);

  if (options_validation(trc))
    {
      ERROR("tracer: option validation error\n");
      return 1;
    }

  
  if ((drv = tracer_driver_get_by_name(trc->out_format_name)) != NULL)
    {
      switch (trc->mode)
	{

	  /* ********************** */
	  /* ********************** */

	case TRC_FILE:
	  DMSG(trc,"tracer: file mode %s\n",trc->in_filename);
	  tracer_file_in_open(trc);
	  tracer_file_out_open(trc);
	  drv->init(trc);
	  drv->process(trc);
	  drv->finalize(trc);
	  tracer_file_in_close(trc);
	  tracer_file_out_close(trc);
	  break;

	  /* ********************** */
	  /* ********************** */

	case TRC_DIR:
	  DMSG(trc,"tracer: dir mode, scanning *.trc files [merge=%d]\n",trc->merge);

	  drv->init(trc);
	  if (trc->merge) 
	    { 
	      tracer_file_out_open(trc); 
	    }

	  {
	    tracer_dirmode_init(trc,".trc");
	    while (tracer_dirmode_next(trc) == 0)
	      {
		DMSG(trc,"tracer: processing file %s\n",argv[next_arg], trc->in_filename);
		if (trc->merge == 0)
		  {
		    snprintf(trc->out_filename,FILENAME_MAX,"%s.%s", trc->in_filename, drv->ext);
		    tracer_file_out_open(trc);
		  }
		tracer_file_in_open(trc);
		drv->process(trc);
		tracer_file_in_close(trc);
		if (trc->merge == 0)
		  {
		    tracer_file_out_close(trc);
		  }
	      }
	    tracer_dirmode_close(trc);
	  }

	  drv->finalize(trc);
	  if (trc->merge) 
	    { 
	      tracer_file_out_close(trc); 
	    }
	  break;

	  /* ********************** */
	  /* ********************** */

	case TRC_MULTI:
	  DMSG(trc,"tracer: multi file mode [merge=%d]\n",trc->merge);
	  drv->init(trc);
	  if (trc->merge) 
	    { 
	      tracer_file_out_open(trc); 
	    }
	  
	  {
	    while (next_arg < argc)
	      {
		DMSG(trc,"tracer: processing file %s\n",argv[next_arg]);
		strncpy(trc->in_filename,argv[next_arg],FILENAME_MAX);
		if (trc->merge == 0)
		  {
		    snprintf(trc->out_filename,FILENAME_MAX,"%s.%s", trc->in_filename, drv->ext);
		    tracer_file_out_open(trc);
		  }
		tracer_file_in_open(trc);
		drv->process(trc);
		tracer_file_in_close(trc);
		if (trc->merge == 0)
		  {
		    tracer_file_out_close(trc);
		  }
		next_arg++;
	      }
	  }

	  drv->finalize(trc);
	  if (trc->merge) 
	    { 
	      tracer_file_out_close(trc); 
	    }
	  break;

	  /* ********************** */
	  /* ********************** */

	default:
	  ERROR("tracer: should not come here, default mode for DRV\n");
	  break;

	  /* ********************** */
	  /* ********************** */
	} /* trc->mode */
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

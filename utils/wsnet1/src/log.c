/*
 * log.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */

#include <public/types.h>

#include <private/models_private.h>
#include <private/log_private.h>
#include <private/command_line.h>

#include "liblogger/logger.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int g_log_to_file = 0;
FILE *g_log_file  = NULL;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

char event_strings[][256]  = {"Layer       ",
			      "Instantiate ",
			      "Medium      ", 
			      "Worldsens   ",
			      "Complete    "};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int init_log_sys(const char * logpath) 
{
  
  if ((g_log_file = fopen(logpath,"w")) == NULL) 
    {
      ERROR ("wsnet: unable to open log file %s\n",optarg);
      g_log_to_file = 0;
      return -1;
    }
  
  g_log_to_file = 1;
  return 0;
} 

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int flogf2 (int level, int node_id, int packet_id, 
	    int event_type, int packet_size, 
	    char * layer, char * key, const char * format, ...) 
{
  int done = 0;

  if (!g_log_to_file || !g_log_file)
    return 0;

  fprintf(g_log_file, "%s\t% "PRId64"\t%2d\t%2d\t%2d\t%2d\t%s\t%s\t", event_strings[level], g_time, 
	  node_id, packet_id, event_type, packet_size, layer, key);

  va_list(arg);

  va_start(arg,format);
  done = vfprintf(g_log_file,format,arg);
  va_end(arg);

  fprintf(g_log_file, "\n");
  return done;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

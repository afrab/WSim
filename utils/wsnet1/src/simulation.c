/*
 *  simulation.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/core_private.h>
#include <private/sim_conf.h>
#include <private/command_line.h>
#include <private/log_private.h>

#include "libtracer/tracer.h"
#include "liblogger/logger.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int g_m_nodes = -1;
int g_c_nodes = 0;
	
double g_x    = -1.0;
double g_y    = -1.0;
double g_z    = -1.0;

struct _worldsens_s worldsens;



uint64_t g_sim_time = 0;

uint64_t get_global_time()               
{ 
  return g_sim_time; 
}

void set_global_time(uint64_t time)  
{ 
  g_sim_time = time; 
  WSNET_S_DBG_TIME ("WSNET:: ===\n");
  WSNET_S_DBG_TIME ("WSNET:: === TIME  %"PRId64" (seq: %d)\n", time, worldsens.rp_seq);
  WSNET_S_DBG_TIME ("WSNET:: ===\n");
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int simulation_start(int argc, char* argv[]) 
{
  logger_init("stdout",4);
  tracer_init("wsnet1.trc", 0);
  tracer_set_timeref(get_global_time);
  tracer_start();

  if (command_line (argc, argv)) 
    {
      return -1;
    }
  
  if (sim_config ()) 
    {
      return -1;
    }

  if (worldsens_s_initialize(&worldsens))
    {
      return -1;
    }

  core_start(&worldsens);

  tracer_close(); 
  logger_close();

  if (g_log_to_file) 
    {
      fclose(g_log_file);
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/*
 *  simulation.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/core_private.h>
#include <private/parse.h>
#include <private/command_line.h>
#include <private/log_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int g_m_nodes = -1;
int g_c_nodes = 0;
	
double g_x = -1.0;
double g_y = -1.0;
double g_z = -1.0;


#ifndef WORLDSENS
int g_error = -1;
uint64_t g_simend = 0;

#else //WORLDSENS
struct _worldsens_s worldsens;
#endif //WORLDSENS

uint64_t g_time = 0;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int simulation_init(int argc, char *argv[]) {
  if (command_line (argc, argv)) {
    return -1;
  }
	
  if (parse_config ()) {
    return -1;
  }
	
#ifdef WORLDSENS
  if (worldsens_s_initialize(&worldsens))
    return 0;	/* Initialize simulation */
#endif //WORLDSENS
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int simulation_start(void) {

#ifndef WORLDSENS
  core_start();
#else //WORLDSENS
  core_start(&worldsens);
#endif //WORLDSENS
	
  if (g_log_to_file) {
    fclose(g_log_file);
  }

  return 0;
}

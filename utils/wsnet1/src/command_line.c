/*
 * command_line.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/log_private.h>
#include <private/models_private.h>
#include <private/command_line.h>

#ifdef WORLDSENS
#include "worldsens.h"
#endif //WORLDSENS


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int g_seed = -1;
char g_config_repository[256];


#ifndef WORLDSENS
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void usage(void) {
  fprintf(stderr, "Usage: simulation [-F config_repertory] [-s seed] [-l logfile]\n");
  return;
}
#else //WORLDSENS


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void usage(void) {
  fprintf(stderr, "Usage: simulation [-F config_repertory] [-s seed] [-l logfile] [-p local_port] [-P multicast_port] [-m multicast_addr]\n");
  return;
}
#endif //WORLDSENS


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int command_line(int argc, char *argv[]) {
  int c;
	
  strcpy(g_config_repository, "config");
	
  while ((c = getopt(argc, argv, "F:s:l:d:")) != -1) {
    switch (c) {
#ifdef WORLDSENS
    case 'p': 
      g_lport = atoi(optarg);
      break;
    case 'P': 
      g_mport = atoi(optarg);
      break;
    case 'm': 
      g_maddr = optarg;
      break;
#endif //WORLDSENS
    case 's':
      g_seed = atoi(optarg);
      break;				
    case 'F':
      strcpy(g_config_repository, optarg);
      break;
    case 'l':
      if (init_log_sys(optarg)) {
	usage();
	return -1;
      }
      break;
    default: 
      usage();
      return -1;
    }
  }
	
  return 0;
}

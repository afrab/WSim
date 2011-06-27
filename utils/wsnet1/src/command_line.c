/*
 * command_line.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/log_private.h>
#include <private/command_line.h>

#include "worldsens.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int g_seed = -1;
char g_config_repository[256];


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void usage(void) {
  fprintf(stderr, "Usage: simulation [-S seed] [-l logfile] [-p local_port] [-P multicast_port] [-m multicast_addr] [-D synchro_period]\n");
  return;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int command_line(int argc, char *argv[]) {
  int c;
	
  while ((c = getopt(argc, argv, "p:P:m:S:l:D:F:")) != -1) {
    switch (c) {
    case 'p': 
      g_lport = atoi(optarg);
      break;
    case 'P': 
      g_mport = atoi(optarg);
      break;
    case 'm': 
      g_maddr = optarg;
      break;
    case 'S':
      g_seed = atoi(optarg);
      break;				
    case 'D':
      WORLDSENS_SYNCH_PERIOD = atoi(optarg);
      break;
    case 'F':       /* Deprecated option: here to avoid errors */
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

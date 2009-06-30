/*
 *  sim_conf.c
 *  
 *
 *  Created by Loic Lemaitre on 29/06/09. Inspired from previous file parce.c.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <time.h>

#include <private/sim_conf.h>
#include <private/nodes_private.h>
#include <private/command_line.h>


#include <public/simulation.h>
#include <public/types.h>

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define MAX_NODES 128  /* 128 nodes max */
#define X_LENGTH  100  /* x length      */
#define Y_LENGTH  100  /* y length      */
#define Z_LENGTH  100  /* z length      */

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int sim_config(void) 
{
  int loop;
	

  /********************* Initialize seed value ***********************/
  if (g_seed == -1) 
    {
      struct timeval tp;
      gettimeofday(&tp, NULL);
      srand48(tp.tv_sec + tp.tv_usec);
    } 
  else 
    {
      srand48(g_seed);
    }
	
	
  /******************** Simulation parameters ************************/

 /* Set informations*/
  g_m_nodes = MAX_NODES;
  g_x       = X_LENGTH;
  g_y       = Y_LENGTH;
  g_z       = Z_LENGTH;

	
	

  /* Allocate node space */
  if ((g_nodes = (struct _node *) malloc(g_m_nodes * sizeof(struct _node))) == NULL) {
    fprintf(stderr, "malloc error\n");
    return -1;
  }
  memset(g_nodes, 0, sizeof(struct _node) * g_m_nodes);
	
  /* For each node */
  loop = g_m_nodes;
  while (loop--) 
    {
      struct _node *node = &(g_nodes[loop]);
      
      node->addr = loop;
    }

  fprintf(stdout, "Simulation setup: %d nodes, (%lfm, %lfm, %lfm) field\n", 
	  g_m_nodes, g_x, g_y, g_z);

  return 0;
}

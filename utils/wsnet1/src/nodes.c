/*
 *  nodes.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/nodes_private.h>
#include <private/simulation_private.h>

#include "public/log.h"
#include "libtracer/tracer.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct _node *g_nodes = NULL;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int node_create(int addr) 
{
  char mylabel[30];
	
  if (g_c_nodes == g_m_nodes) 
    {
      fprintf(stderr, "WARNING: simulation full, node refused\n");
      return -1;
    }
	
  if (g_nodes[addr].active == 0) 
    {
      g_nodes[addr].active = 1;
      g_c_nodes++;
      g_nodes[addr].addr = addr;
      flogf2(LEVEL_WORLDSENS, g_nodes[addr].addr, -1, EVENT_CREATE, -1, WORLDSENS_LAYER, KEY_CORE, "Node connection");
      /* TRACER */
      sprintf(mylabel, "node %d tx", addr);
      g_nodes[addr].trc_id = tracer_event_add_id(32, mylabel, "wsnet1");
    }

  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int node_delete (int addr) 
{
  if (g_nodes[addr].active == 1) 
    {
      g_nodes[addr].active = 0;
      g_c_nodes--;
      flogf2(LEVEL_WORLDSENS, g_nodes[addr].addr, -1, EVENT_DESTROY, -1, WORLDSENS_LAYER, KEY_CORE, "Node disconnection");
    }
	
  if (g_c_nodes == 0) 
    {
      return -1;
    }
	
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

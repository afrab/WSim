/*
 *  static_static_mobility.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "static_static_mobility.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _static_static_mobility_private {
  uint64_t last_update;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "STATIC_STATIC";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_static_mobility_instantiate(struct _node *node, FILE *config_fd) 
{
  struct _static_static_mobility_private * private = (struct _static_static_mobility_private *) node->mobility_private;

  if (fscanf(config_fd, "%lf %lf %lf", &(node->x), &(node->y), &(node->z)) != 3)
    {
      fprintf(stderr, "static_static_mobility_instantiate : Unable to read \"mobility.inst\"\n");
      return -1;
    }
  private->last_update = 0;
	
  flogf2(LEVEL_INSTANTIATE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "%lf %lf %lf", node->x, node->y, node->z);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_static_mobility_update(struct _node *node) 
{
  struct _static_static_mobility_private * private = (struct _static_static_mobility_private *)node->mobility_private;

  private->last_update = g_time;

  flogf2(LEVEL_LAYER, node->addr, -1, EVENT_MOVE, -1, MOBILITY_LAYER, key_private, "%lf %lf, %lf", node->x, node->y, node->z);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_static_mobility_complete(struct _node *node) 
{
  flogf2(LEVEL_COMPLETE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "");
  return 0;
}

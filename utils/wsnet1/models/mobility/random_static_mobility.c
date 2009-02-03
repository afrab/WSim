/*
 *  random_static_mobility.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "random_static_mobility.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _random_static_mobility_private {
  uint64_t last_update;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "RANDOM_STATIC";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int random_static_mobility_instantiate(struct _node *node, FILE UNUSED *config_fd)
{
  struct _random_static_mobility_private * private = (struct _random_static_mobility_private *)node->mobility_private;

  node->x = drand48()*g_x;
  node->y = drand48()*g_y;
  node->z = drand48()*g_z;
  private->last_update = 0;
	
  flogf2(LEVEL_INSTANTIATE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "%lf %lf %lf", node->x, node->y, node->z);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int random_static_mobility_update(struct _node *node) 
{
	
  struct _random_static_mobility_private * private = (struct _random_static_mobility_private *)node->mobility_private;

  private->last_update = g_time;
	
  flogf2(LEVEL_LAYER, node->addr, -1, EVENT_MOVE, -1, MOBILITY_LAYER, key_private, "%lf %lf, %lf", node->x, node->y, node->z);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int random_static_mobility_complete(struct _node *node) 
{
  flogf2(LEVEL_COMPLETE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "");
  return 0;
}

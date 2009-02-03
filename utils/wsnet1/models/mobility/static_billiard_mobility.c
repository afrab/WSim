/*
 *  static_biliard_mobility.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "static_billiard_mobility.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define pi 3.14


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _static_billiard_mobility_private
{
  double direction;
  double speed;
  uint64_t last_update;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "STATIC_BILIARD";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_billiard_mobility_instantiate(struct _node *node, FILE *config_fd)
{
  double speed_max, speed_min;
  struct  _static_billiard_mobility_private * private = 
    (struct  _static_billiard_mobility_private *) node->mobility_private;
	
  if (fscanf(config_fd, "%lf %lf %lf %lf %lf", &(node->x), &(node->y), &(node->z), &speed_min, &speed_max) != 5)
    {
      fprintf(stderr, "Unable to read \"mobility.inst\"\n");
      return 0;
    }
	
  private->speed = drand48()*(speed_max - speed_min) + speed_min;
  while (private->direction == 0 || private->direction == pi || private->direction == -pi/2 || private->direction == pi/2)
    private->direction = drand48()*2*pi - pi;
  private->last_update = 0;
		
  flogf2(LEVEL_INSTANTIATE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "%lf %lf %lf %lf %lf", node->x, node->y, node->z, speed_min, speed_max);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static double min_pos(double a, double b) 
{
  if (a <= (double) 0) return b;
  if (b <= (double) 0) return a;
	
  return (a > b ? b : a);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_billiard_mobility_update(struct _node *node)
{
  struct  _static_billiard_mobility_private * private = 
    (struct  _static_billiard_mobility_private *) node->mobility_private;

  while  (private->last_update < g_time) {
		
    double t_0, t_1 = -1, t_2 = -1, t_3 = -1, t_4 = -1;
		
    flogf2(LEVEL_LAYER, node->addr, -1, EVENT_MOVE, -1, MOBILITY_LAYER, key_private, "%lf %lf, %lf", node->x, node->y, node->z);

    t_1 = (0 - node->x) / (private->speed * cos(private->direction));
    t_2 = (g_x - node->x) / (private->speed * cos(private->direction));
    t_3 = (0 - node->y) / (private->speed * sin(private->direction));
    t_4 = (g_y - node->y) / (private->speed * sin(private->direction));
				
    t_0 = min_pos(t_1, min_pos(t_2, min_pos(t_3, min_pos((g_time - private->last_update), t_4))));

    node->x = private->speed * cos(private->direction)*t_0 + node->x;
    node->y = private->speed * sin(private->direction)*t_0 + node->y;


    if (node->x > g_x)
      node->x = g_x;
    if (node->y > g_y)
      node->y = g_y;
    if (node->x < 0)
      node->x = 0;
    if (node->y < 0)
      node->y = 0;

    /* if t_0 is g_time - last_update we have to leave  the loop.
     * we have to check this because of the imprecision on the double addition */

    if (t_0 == g_time - private->last_update) {
      private->last_update = g_time;
      break;
    }

    private->last_update = private->last_update + (uint64_t) t_0;
		
    if (node->x == 0 || node->x == g_x || node->y == 0 || node->y == g_y)
      {
	if (private->direction <= pi/2 && private->direction >= 0) {
	  if (node->x == g_x)
	    private->direction = pi - private->direction;	
	  else 
	    private->direction = 0 - private->direction;	
	} else if (private->direction >= pi/2)	{				
	  if (node->x == 0)
	    private->direction = pi - private->direction;	
	  else 
	    private->direction = 0 - private->direction;	
	} else if (private->direction >= -pi/2)	{				
	  if (node->x == g_x)
	    private->direction = - pi - private->direction;	
	  else 
	    private->direction = 0 - private->direction;	
	} else	{				
	  if (node->x == 0)
	    private->direction = - pi - private->direction;	
	  else 
	    private->direction = 0 - private->direction;	
	}
      }
  }

  flogf2(LEVEL_LAYER, node->addr, -1, EVENT_MOVE, -1, MOBILITY_LAYER, key_private, "%lf %lf, %lf", node->x, node->y, node->z);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_billiard_mobility_complete(struct _node *node) 
{
  flogf2(LEVEL_COMPLETE, node->addr, -1, EVENT_NULL, -1, MOBILITY_LAYER, key_private, "");
  return 0;
}

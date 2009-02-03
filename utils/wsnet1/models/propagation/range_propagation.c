/*
 *  range_propagation.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "range_propagation.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static struct _range_propagation_private {
  double range;
} propagation_private;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "RANGE";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int range_propagation_instantiate(FILE *config_fd)
{	
  if (fscanf(config_fd, "%lf", &propagation_private.range) != 1)
    {
      fprintf(stderr, "Unable to read \"propagation.inst\"\n");
      return -1;
    }
	
  flogf2(LEVEL_INSTANTIATE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "%lf", propagation_private.range);
  return 0;
}	


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double range_propagation_propagation(double mW, 
				     double x0, double y0, double UNUSED z0, 
				     double x1, double y1, double UNUSED z1)
{
  if (sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)) > propagation_private.range)
    {
      return 0;
    } else {
    return mW;
  }
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int range_propagation_complete(void) 
{
  flogf2(LEVEL_COMPLETE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "");
  return 0;
}

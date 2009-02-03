/*
 *  pathloss_propagation.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "pathloss_propagation.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static struct _pathloss_propagation_private {
  double pathloss;
}  propagation_private;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "PATHLOSS";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int pathloss_propagation_instantiate(FILE *config_fd)
{	
  if (fscanf(config_fd, "%lf", &propagation_private.pathloss) != 1)
    {
      fprintf(stderr, "Unable to read \"propagation.inst\"\n");
      return -1;
    }
		
  flogf2(LEVEL_INSTANTIATE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "%lf", propagation_private.pathloss);
  return 0;
}	


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double pathloss_propagation_propagation(double mW, 
					double x0, double y0, double UNUSED z0, 
					double x1, double y1, double UNUSED z1)
{
  double r = sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
		
  return mW  / pow(1 + r, propagation_private.pathloss); 
}



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int pathloss_propagation_complete(void) 
{
  flogf2(LEVEL_COMPLETE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "");
  return 0;
}

/*
 *  no_fading_propagation.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "no_fading_propagation.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "NO_FADING";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int no_fading_propagation_instantiate(FILE UNUSED *config_fd)
{
  flogf2(LEVEL_INSTANTIATE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "");
  return 0;
}	


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double no_fading_propagation_propagation(double mW, 
					 double UNUSED x0, double UNUSED y0, double UNUSED z0, 
					 double UNUSED x1, double UNUSED y1, double UNUSED z1)
{
  return mW;	
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int no_fading_propagation_complete(void) 
{
  flogf2(LEVEL_COMPLETE, -1, -1, EVENT_NULL, -1, PROPAGATION_LAYER, key_private, "");
  return 0;
}

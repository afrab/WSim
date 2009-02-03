/*
 *  cdma_interference.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "cdma_interference.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static struct _cdma_interference_private {
  double alpha;
} interference_private;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "CDMA";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int cdma_interference_instantiate(FILE * config_fd) 
{
  if (fscanf(config_fd, "%lf", &interference_private.alpha) != 1) {
    fprintf(stderr, "Unable to read \"interference.inst\"\n");
    return -1;
  }

  flogf2(LEVEL_INSTANTIATE, -1, -1, EVENT_NULL, -1, INTERFERENCE_LAYER, key_private, "%lf", interference_private.alpha);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double cdma_interference_correlation(double mW, int radio0, int radio1) 
{
  if (radio0 == radio1) {
    return mW;
  } else if ((radio0 == 0) || (radio1 == 0)) {
    return 0.0;
  } else {
    return (interference_private.alpha * mW);		
  }
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int cdma_interference_complete(void) 
{
  flogf2(LEVEL_COMPLETE, -1, -1, EVENT_NULL, -1, INTERFERENCE_LAYER, key_private, "");
  return 0;
}

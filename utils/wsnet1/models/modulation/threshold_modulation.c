/*
 *  threshold_modulation.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "threshold_modulation.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static struct _threshold_modulation_private {
  double thres;
} modulation_private;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "THRESHOLD";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int threshold_modulation_instantiate(FILE *config_fd) 
{
  if (fscanf(config_fd, "%lf", &modulation_private.thres) != 1) {
    fprintf(stderr, "Unable to read \"interference.inst\"\n");
    return -1;
  }
	
  flogf2(LEVEL_INSTANTIATE, -1, -1, EVENT_NULL, -1, MODULATION_LAYER, key_private, "%lf", modulation_private.thres);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double threshold_modulation_compute_BER(double SiNR)
{
  if (SiNR <= modulation_private.thres)
    return 1;
  else
    return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int threshold_modulation_complete(void) 
{

  flogf2(LEVEL_COMPLETE, -1, -1, EVENT_NULL, -1, MODULATION_LAYER, key_private, "");
  return 0;
}

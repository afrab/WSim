/*
 *  omni_antenna.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */

#include "omni_antenna.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _omni_antenna_private {
  double min_rx_mW;
  double noise_mW;
  double degree;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static char key_private[] = "OMNI";


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int omni_antenna_instantiate(struct _node * node, FILE * config_fd) 
{
  struct _omni_antenna_private * private = (struct _omni_antenna_private *)(node->antenna_private);
  double min_rx_mW, noise_mW;

  if (fscanf(config_fd, "%lf %lf", &min_rx_mW, &noise_mW) != 2) {
    fprintf(stderr, "omni_antenna_instantiate : failed to read \"antemna.inst\"\n");
    return -1;
  }
	
  private->min_rx_mW = min_rx_mW; 
  private->noise_mW = noise_mW; 
	
  flogf2(LEVEL_INSTANTIATE, node->addr, -1, EVENT_NULL, -1, ANTENNA_LAYER, key_private, "%lf %lf", noise_mW, min_rx_mW);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double omni_antenna_compute_tx(struct _node UNUSED *node, double mW, double UNUSED x, double UNUSED y, double UNUSED z) 
{
  return mW;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double omni_antenna_compute_rx(struct _node UNUSED *node, double mW, double UNUSED x, double UNUSED y, double UNUSED z) 
{
  return mW;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int omni_antenna_get_ioctl(struct _node UNUSED *node, int UNUSED ioctl, void UNUSED *arg) 
{
  return -1;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int omni_antenna_set_ioctl(struct _node UNUSED *node, int UNUSED ioctl, void UNUSED *arg) 
{
  return -1;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int omni_antenna_complete(struct _node *node) 
{
  flogf2(LEVEL_COMPLETE, node->addr, -1, EVENT_NULL, -1, ANTENNA_LAYER, key_private, "");
  return 0;
}

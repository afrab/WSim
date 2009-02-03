/*
 *  threshold_modulation.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _THRESHOLD_MODULATION_H_
#define _THRESHOLD_MODULATION_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int threshold_modulation_instantiate (FILE * config_fd);
double threshold_modulation_compute_BER (double SiNR);
int threshold_modulation_complete (void);


#endif

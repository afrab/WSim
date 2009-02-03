/*
 *  bpsk_modulation.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _BPSK_MODULATION_H_
#define _BPSK_MODULATION_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int bpsk_modulation_instantiate (FILE * config_fd);
double bpsk_modulation_compute_BER (double SiNR);
int bpsk_modulation_complete (void);


#endif

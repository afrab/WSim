/*
 *  modulation_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _MODULATION_PRIVATE_H
#define _MODULATION_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>

#include <public/modulation.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern struct _model_interference * g_interference;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int modulation_instantiate (char * key, FILE * config_fd); 
double modulation_compute_BER (double SiNR);
int modulation_complete (void);


#endif //_MODULATION_PRIVATE_H

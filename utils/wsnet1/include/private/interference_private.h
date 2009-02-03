/*
 *  interference_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _INTERFERENCE_PRIVATE_H
#define _INTERFERENCE_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>

#include <public/interference.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern struct _model_interference * g_interference;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int interference_instantiate (char * key, FILE * config_fd); 
double interference_correlation	(double mW, int radio0, int radio1);
int interference_complete (void);


#endif //_INTERFERENCE_PRIVATE_H

/*
 *  mobility_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _MOBILITY_PRIVATE_H
#define _MOBILITY_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>

#include <public/mobility.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int mobility_instantiate (struct _node * node, char * key, FILE * config_fd); 
int mobility_update (struct _node * node);


#endif //_MOBILITY_PRIVATE_H

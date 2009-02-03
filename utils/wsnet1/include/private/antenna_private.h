/*
 *  antenna_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _ANTENNA_PRIVATE_H
#define _ANTENNA_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>

#include <public/antenna.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_instantiate	(struct _node * node, char * key, FILE * config_fd); 
void antenna_rx (struct _node *  node, struct _packet *  packet);
void antenna_complete (struct _node * node); 


#endif //_ANTENNA_PRIVATE_H

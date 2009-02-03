/*
 *  queue_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _QUEUE_PRIVATE_H
#define _QUEUE_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int queue_instantiate (struct _node * node, char * key, FILE * config_fd); 
void queue_complete (struct _node * node); 


#endif //_QUEUE_PRIVATE_H

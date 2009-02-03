/*
 *  application_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _APPLICATION_PRIVATE_H
#define _APPLICATION_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int application_instantiate (struct _node * node, char * key, FILE * config_fd); 
void application_complete (struct _node * node); 


#endif //_APPLICATION_PRIVATE_H

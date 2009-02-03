/*
 *  mac_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _MAC_PRIVATE_H
#define _MAC_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int mac_instantiate (struct _node * node, char * key, FILE * config_fd); 
int mac_rx (struct _node * node, struct _packet * packet);
void mac_complete (struct _node * node); 


#endif //_MAC_PRIVATE_H

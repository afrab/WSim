/*
 *  static_static_mobility.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _STATIC_STATIC_MOBILITY_H_
#define _STATIC_STATIC_MOBILITY_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_static_mobility_instantiate(struct _node * node, FILE * config_fd);
int static_static_mobility_update (struct _node * node);
int static_static_mobility_complete (struct _node * node);

#endif

/*
 *  static_biliard_mobility.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _STATIC_BILLIARD_MOBILITY_H_
#define _STATIC_BILLIARD_MOBILITY_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int static_billiard_mobility_instantiate (struct _node * node, FILE * config_fd);
int static_billiard_mobility_update (struct _node * node);
int static_billiard_mobility_complete (struct _node * node);


#endif

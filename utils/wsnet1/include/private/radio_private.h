/*
 *  radio_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _RADIO_PRIVATE_H
#define _RADIO_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/radio.h>

#include <private/models_private.h>
#include <private/tracer_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int radio_instantiate (struct _node * node, char * key, FILE * config_fd); 
void radio_rx (struct _node * node, struct _packet * packet);
void radio_carrier_sense (struct _node * node, int p_id, double rx_mW, double SiNR, int radio);
void radio_complete (struct _node * node); 


#endif //_RADIO_PRIVATE_H

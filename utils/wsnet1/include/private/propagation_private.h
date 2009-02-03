/*
 *  propagation_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _PROPAGATION_PRIVATE_H
#define _PROPAGATION_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <private/models_private.h>
#include <private/packets_private.h>

#include <public/propagation.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern struct _model_propagation * g_propagation;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int propagation_instantiate (char * key, FILE * config_fd); 
double propagation_propagation (double mW, double x0, double y0, double z0, double x1, double y1, double z1);
double propagation_compute_BER (struct _node * node, struct _packet * packet);
double propagation_compute_rx_mW (struct _node * node, struct _packet * packet);
double propagation_compute_noise (struct _node * node, int radio, int packet_id, uint64_t time);
int propagation_complete (void);


#endif //_PROPAGATION_PRIVATE_H

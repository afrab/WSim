/*
 *  radio.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _RADIO_H
#define _RADIO_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int radio_tx (struct _node * node, struct _packet * packet);
int radio_get_resource (struct _node * node);
void radio_set_resource	(struct _node * node, int radio);
double radio_get_tx_mW (struct _node * node);
void radio_set_tx_mW (struct _node * node, double tx_mW);
int radio_get_N_s (struct _node * node);
void radio_set_N_s (struct _node * node, int N_s);
double radio_get_T_s (struct _node * node);
void radio_set_T_s (struct _node * node, double T_s);
int radio_get_ioctl (struct _node * node, int ioctl, void * val);
int radio_set_ioctl (struct _node * node, int ioctl, void * val);
double radio_get_cca (struct _node * node);
void radio_on(struct _node * node);
void radio_off(struct _node * node);


#endif //_RADIO_H

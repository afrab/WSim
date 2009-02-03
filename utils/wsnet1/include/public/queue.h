/*
 *  queue.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _QUEUE_H
#define _QUEUE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int queue_tx (struct _node * node, char * data, int size, int dst, int priority);
struct _txbuf * queue_get (struct _node * node);
int queue_get_ioctl (struct _node * node, int ioctl, void * arg);
int queue_set_ioctl (struct _node * node, int ioctl, void * arg);


#endif

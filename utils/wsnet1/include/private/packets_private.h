/*
 *  packets.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __KoBaFiS__. All rights reserved.
 *
 */

#ifndef _PACKETS_PRIVATE_H
#define _PACKETS_PRIVATE_H


/****************************/
/****************************/
/****************************/
extern struct _packet *  g_packets;


/****************************/
/****************************/
/****************************/
void packet_destroy (struct _packet * packet);
struct _packet * packet_duplicate (struct _packet * packet);


#endif //_PACKETS_H


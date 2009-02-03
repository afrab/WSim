/*
 * log.h
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _LOG_H_
#define _LOG_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <stdio.h>

#include <public/simulation.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define LEVEL_LAYER         0
#define LEVEL_INSTANTIATE   1
#define LEVEL_MEDIUM        2
#define LEVEL_WORLDSENS	    3
#define LEVEL_COMPLETE      4


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define EVENT_RX        0
#define EVENT_TX        1
#define EVENT_CREATE    2
#define EVENT_DESTROY   3
#define EVENT_MOVE      4
#define EVENT_IOCTL     5
#define EVENT_NULL      6


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define ANTENNA_LAYER       "ANTENNA     "
#define RADIO_LAYER	    "RADIO       "
#define APPLICATION_LAYER   "APPLICATION "
#define MAC_LAYER           "MAC         "
#define INTERFERENCE_LAYER  "INTERFERENCE"
#define PROPAGATION_LAYER   "PROPAGATIONL"
#define MOBILITY_LAYER      "MOBILITY    "
#define MODULATION_LAYER    "MODULATION  "
#define QUEUE_LAYER         "QUEUE       "
#define WORLDSENS_LAYER     "WORLDSENS   "
#define BATTERY_LAYER       "BATTERY     "

#define KEY_CORE            "CORE        "


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int flogf2 (int level, int node_id, int packet_id, int event_type, int packet_size, char * layer, char * key, const char * format, ...);


#endif

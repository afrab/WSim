/*
 *  types.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _TYPES_H
#define _TYPES_H

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sched.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#undef PRIu64

#ifndef PRIu64
#define PRIu64 "lld"
#endif 


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define APPLICATION_PRIVATE_SIZE    2000
#define QUEUE_PRIVATE_SIZE          2000
#define MAC_PRIVATE_SIZE            2000
#define RADIO_PRIVATE_SIZE          2000
#define ANTENNA_PRIVATE_SIZE        2000
#define MOBILITY_PRIVATE_SIZE       2000
#define BATTERY_PRIVATE_SIZE        2000


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _txbuf {
  char * data;
  int size;
  int dst;	
  int priority;

  struct _txbuf * next;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _radio_public {
  int activity;
  int resource;
  double tx_mW;
  int N_s;
  double T_s;
};

struct _antenna_public {
  int orientation;
  double noise_mW;
  double sensibility;
};

struct _battery_public {
  double mW;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _node {
  int addr;
  int active;
  int trc_id;
  double x, y, z;
	
  struct _model_application * application;
  char application_private [APPLICATION_PRIVATE_SIZE];

  struct _model_mac * mac;
  char mac_private [MAC_PRIVATE_SIZE];

  struct _model_queue * queue;
  char queue_private [QUEUE_PRIVATE_SIZE];

  struct _model_radio * radio;
  struct _radio_public radio_public;
  char radio_private [RADIO_PRIVATE_SIZE];

  struct _model_antenna * antenna;
  struct _antenna_public antenna_public;
  char antenna_private [ANTENNA_PRIVATE_SIZE];

  struct _model_mobility * mobility;
  char mobility_private	[MOBILITY_PRIVATE_SIZE];

  struct _model_battery * battery;
  struct _battery_public battery_public;
  char battery_private [BATTERY_PRIVATE_SIZE];
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _packet {

  unsigned int id;
#ifdef WORLDSENS
  unsigned int seq;
#endif //WORLDSENS
	
  char * data;
  int size;
	
  struct _node * node;
  double x, y, z;
	
  int radio;
  int modulation;
	
  double antenna_orientation;
	
  double tx_mW;
  uint64_t tx_start;
  uint64_t tx_end;
	
  double * BER;
  double PER;
  double * SiNR;
  double rx_mW;

  struct _packet * next;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void free_txbuf(struct _txbuf * txbuf);


#endif //_TYPES_H



/*
 *  core_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _CORE_PRIVATE_H
#define _CORE_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#ifdef WORLDSENS
#include <worldsens.h>
#endif //WORLDSENS

#include <public/core.h>
#include <public/types.h>


/************************/
/************************/
/************************/
extern uint64_t g_simend;


/************************/
/************************/
/************************/
#define TX_EVENT_TYPE		1
#define SCHED_EVENT_TYPE	2


/****************************/
/****************************/
/****************************/
struct _event {

  char type;

  uint64_t time;
  int node;
	
  void (* callback) (void *);
  void * arg;
	
  struct _packet * packet;
	
  struct _event *	 next;
};


/****************************/
/****************************/
/****************************/
extern struct _event *g_events;


/****************************/
/****************************/
/****************************/
#ifdef WORLDSENS 
int core_start (struct _worldsens_s *worldsens);
void core_runtime_end (void);
#else //WORLDSENS
int core_start (void);
void core_runtime_end (void);
#endif //WORLDSENS

int core_backtrack (uint64_t time);
int core_add_packet (struct _packet *packet);
int core_update_packet_list ();

#endif

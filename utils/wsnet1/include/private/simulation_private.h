/*
 *  simulation_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _SIMULATION_PRIVATE_H
#define _SIMULATION_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/types.h>
#include <public/simulation.h>

#ifdef WORLDSENS
#include <worldsens.h>
#endif //WORLDSENS


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern int g_m_nodes;
extern int g_c_nodes;
#ifndef WORLDSENS
extern uint64_t	g_end;
extern int g_error;
#endif //WORLDSENS


#endif //_SIMULATION_PRIVATE_H


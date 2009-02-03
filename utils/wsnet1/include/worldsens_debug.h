/*
 *  worldsens_debug.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _WORLDSENS_DEBUG_H
#define _WORLDSENS_DEBUG_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define WSNET_S_DBG
#define WSNET_S_EXC_DBG


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#ifdef WSNET_S_DBG
#    define WSNET_S_DBG_DBG(x...) printf(x)
#else
#    define WSNET_S_DBG_DBG(x...) do { } while (0)
#endif

#ifdef WSNET_S_EXC_DBG
#    define WSNET_S_DBG_EXC(x...) printf(x)
#else
#    define WSNET_S_DBG_EXC(x...) do { } while (0)
#endif


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define _WSNET_C_BCK_DBG
#define _WSNET_C_EXC_DBG
#define _WSNET_C_DBG


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#ifdef WSNET_C_BCK_DBG
#    define WSNET_C_DBG_BCK(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_BCK(x...) do { } while (0)
#endif

#ifdef WSNET_C_EXC_DBG
#    define WSNET_C_DBG_EXC(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_EXC(x...) do { } while (0)
#endif

#ifdef WSNET_C_DBG
#    define WSNET_C_DBG_DBG(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_DBG(x...) do { } while (0)
#endif


#endif

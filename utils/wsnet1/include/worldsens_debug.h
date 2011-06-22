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

#if defined(DEBUG)
#define WSNET_S_TIME      1  /* WSNet global TIME */
#define WSNET_S_TX        1  /* data RX/TX        */
#endif

#if WSNET_S_TIME != 0
#    define WSNET_S_DBG_TIME(x...) printf(x)
#else
#    define WSNET_S_DBG_TIME(x...) do { } while (0)
#endif

#if WSNET_S_TX != 0
#    define WSNET_S_DBG_TX(x...) printf(x)
#else
#    define WSNET_S_DBG_TX(x...) do { } while (0)
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define WSNET_S_ATTR      1  /* client node connections */

#if defined(DEBUG)
#define WSNET_S_SYNC      1  /* SYNC + RP + BACKTRACK   */
#endif


#if WSNET_S_ATTR != 0
#    define WSNET_S_DBG_ATTR(x...) printf(x)
#else
#    define WSNET_S_DBG_ATTR(x...) do { } while (0)
#endif

#if WSNET_S_SYNC != 0
#    define WSNET_S_DBG_SYNC(x...) printf(x)
#else
#    define WSNET_S_DBG_SYNC(x...) do { } while (0)
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if defined(DEBUG)
#define WSNET_C_BCK_DBG   0
#define WSNET_C_EXC_DBG   0
#define WSNET_C_DBG       0
#endif

#if WSNET_C_BCK_DBG != 0
#    define WSNET_C_DBG_BCK(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_BCK(x...) do { } while (0)
#endif

#if WSNET_C_EXC_DBG != 0
#    define WSNET_C_DBG_EXC(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_EXC(x...) do { } while (0)
#endif

#if WSNET_C_DBG != 0
#    define WSNET_C_DBG_DBG(x...) HW_DMSG(x)
#else
#    define WSNET_C_DBG_DBG(x...) do { } while (0)
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

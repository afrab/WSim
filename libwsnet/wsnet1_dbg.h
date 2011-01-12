
/**
 *  \file   wsnet1_dbg.h
 *  \brief  WorldSens client debug msg v1 (deprecated)
 *  \author Guillaume Chelius
 *  \date   2005
 **/


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

#if DEBUG
#define WSNET_DEBUG(x...)     DMSG_LIB_WSNET(x)
#else
#define WSNET_DEBUG(x...)     do { } while (0)
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define WSNET_DEBUG_BKTRK 1
#define WSNET_DEBUG_EXC   1
#define WSNET_DEBUG_DBG   1
#define WSNET_DEBUG_TX    1
#define WSNET_DEBUG_RX    1

#if WSNET_DEBUG_BKTRK
#    define WSNET_BKTRK(x...) WSNET_DEBUG(x)
#else
#    define WSNET_BKTRK(x...) do { } while (0)
#endif

#if WSNET_DEBUG_EXC
#    define WSNET_EXC(x...)   WSNET_DEBUG(x)
#else
#    define WSNET_EXC(x...)   do { } while (0)
#endif

#if WSNET_DEBUG_DBG
#    define WSNET_DBG(x...)   WSNET_DEBUG(x)
#else
#    define WSNET_DBG(x...)   do { } while (0)
#endif

#if WSNET_DEBUG_TX
#    define WSNET_TX(x...)    WSNET_DEBUG(x)
#else
#    define WSNET_TX(x...)    do { } while (0)
#endif

#if WSNET_DEBUG_RX
#    define WSNET_RX(x...)    WSNET_DEBUG(x)
#else
#    define WSNET_RX(x...)    do { } while (0)
#endif

#define WSNET_ERROR(x...) ERROR(x)

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

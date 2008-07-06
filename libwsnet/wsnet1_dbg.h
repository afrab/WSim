
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

#define WSNET_DMSG_BKTRK
#define WSNET_DMSG_EXC
#define WSNET_DMSG_DBG
#define WSNET_DMSG_TX
#define WSNET_DMSG_RX

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#ifdef WSNET_DMSG_BKTRK
#    define WSNET_BKTRK(x...) HW_DMSG(x)
#else
#    define WSNET_BKTRK(x...) do { } while (0)
#endif

#ifdef WSNET_DMSG_EXC
#    define WSNET_EXC(x...) HW_DMSG(x)
#else
#    define WSNET_EXC(x...) do { } while (0)
#endif

#ifdef WSNET_DMSG_DBG
#    define WSNET_DBG(x...) HW_DMSG(x)
#else
#    define WSNET_DBG(x...) do { } while (0)
#endif

#ifdef WSNET_DMSG_TX
#    define WSNET_TX(x...) HW_DMSG(x)
#else
#    define WSNET_TX(x...) do { } while (0)
#endif

#ifdef WSNET_DMSG_RX
#    define WSNET_RX(x...) HW_DMSG(x)
#else
#    define WSNET_RX(x...) do { } while (0)
#endif

#define WSNET_ERROR(x...) ERROR(x)

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

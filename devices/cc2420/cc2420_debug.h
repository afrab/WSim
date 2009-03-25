
/**
 *  \file   cc2420_debug.h
 *  \brief  CC2420 debug messages 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc1100_debug.h
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_DEBUG_H_
#define _CC2420_DEBUG_H_

/***************************************************/
/***************************************************/
/***************************************************/

//#define DEBUG

#if defined(DEBUG)
#define CC2420_DEBUG(x...)     VERBOSE(2,x)
#define CC2420_DBG_RX(x...)    VERBOSE(2,x)
#define CC2420_DBG_TX(x...)    VERBOSE(2,x)
#else
#define CC2420_DEBUG(x...)     do { } while (0)
#define CC2420_DBG_RX(x...)    do { } while (0)
#define CC2420_DBG_TX(x...)    do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

#define CC2420_PINS_DEBUG
#define CC2420_ACCESS_DEBUG
#define CC2420_STROBE_DEBUG
#define CC2420_STATE_DEBUG

#ifdef CC2420_PINS_DEBUG
#    define CC2420_DBG_PINS(x...) HW_DMSG(x)
#else
#    define CC2420_DBG_PINS(x...) do { } while (0)
#endif

#ifdef CC2420_ACCESS_DEBUG
#    define CC2420_DBG_ACCESS(x...) HW_DMSG(x)
#else
#    define CC2420_DBG_ACCESS(x...) do { } while (0)
#endif

#ifdef CC2420_STROBE_DEBUG
#    define CC2420_DBG_STROBE(x...) HW_DMSG(x)
#else
#    define CC2420_DBG_STROBE(x...) do { } while (0)
#endif

#ifdef CC2420_STATE_DEBUG
#    define CC2420_DBG_STATE(x...) HW_DMSG(x)
#else
#    define CC2420_DBG_STATE(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

#endif

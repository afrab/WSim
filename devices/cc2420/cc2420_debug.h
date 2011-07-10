
/**
 *  \file   cc2420_debug.h
 *  \brief  CC2420 debug messages 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_debug.h
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

#define CC2420_DBG(x...)       HW_DMSG_DEV(x)

/***************************************************/
/***************************************************/
/***************************************************/

#define CC2420_DEBUG_FIFO    0
#define CC2420_DEBUG_PINS    0
#define CC2420_DEBUG_ACCESS  0
#define CC2420_DEBUG_STROBE  0
#define CC2420_DEBUG_STATE   0
#define CC2420_DEBUG_MUX     0
#define CC2420_DEBUG_RX      0
#define CC2420_DEBUG_TX      0

#if CC2420_DEBUG_FIFO != 0
#    define CC2420_DBG_FIFO(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_FIFO(x...) do { } while (0)
#endif

#if CC2420_DEBUG_PINS != 0
#    define CC2420_DBG_PINS(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_PINS(x...) do { } while (0)
#endif

#if CC2420_DEBUG_ACCESS != 0
#    define CC2420_DBG_ACCESS(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_ACCESS(x...) do { } while (0)
#endif

#if CC2420_DEBUG_STROBE != 0
#    define CC2420_DBG_STROBE(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_STROBE(x...) do { } while (0)
#endif

#if CC2420_DEBUG_STATE != 0
#    define CC2420_DBG_STATE(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_STATE(x...) do { } while (0)
#endif

#if CC2420_DEBUG_MUX != 0
#    define CC2420_DBG_MUX(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_MUX(x...) do { } while (0)
#endif

#if CC2420_DEBUG_RX != 0
#    define CC2420_DBG_RX(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_RX(x...) do { } while (0)
#endif

#if CC2420_DEBUG_TX != 0
#    define CC2420_DBG_TX(x...) CC2420_DBG(x)
#else
#    define CC2420_DBG_TX(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

#endif

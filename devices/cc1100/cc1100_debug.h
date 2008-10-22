
/**
 *  \file   cc1100_debug.h
 *  \brief  CC1100 debug messages
 *  \author Guillaume Chelius
 *  \date   2006
 **/


/*
 *  cc1100_debug.h
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC1100_DEBUG_H
#define _CC1100_DEBUG_H

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_IMPLEMENTATION_DEBUG
#define CC1100_EXCEPTION_DEBUG
#define _CC1100_STATE_DEBUG
#define CC1100_STATUS_DEBUG
#define CC1100_STROBE_DEBUG
#define _CC1100_GDO_DEBUG
#define _CC1100_SPI_DEBUG
#define _CC1100_PINS_DEBUG
#define _CC1100_FIFO_DEBUG
#define _CC1100_ACCESS_DEBUG
#define CC1100_REGISTER_DEBUG
#define CC1100_TX_DEBUG
#define CC1100_RX_DEBUG
#define CC1100_PACKET_DEBUG

#ifdef CC1100_STATE_DEBUG
#    define CC1100_DBG_STATE(x...) VERBOSE(4,x)
#else
#    define CC1100_DBG_STATE(x...) do { } while (0)
#endif

#ifdef CC1100_STATUS_DEBUG
#    define CC1100_DBG_STATUS(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_STATUS(x...) do { } while (0)
#endif

#ifdef CC1100_STROBE_DEBUG
#    define CC1100_DBG_STROBE(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_STROBE(x...) do { } while (0)
#endif

#ifdef CC1100_FIFO_DEBUG
#    define CC1100_DBG_FIFO(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_FIFO(x...) do { } while (0)
#endif

#ifdef CC1100_IMPLEMENTATION_DEBUG
#    define CC1100_DBG_IMPL(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_IMPL(x...) do { } while (0)
#endif

#ifdef CC1100_GDO_DEBUG
#    define CC1100_DBG_GDO(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_GDO(x...) do { } while (0)
#endif

#ifdef CC1100_SPI_DEBUG
#    define CC1100_DBG_SPI(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_SPI(x...) do { } while (0)
#endif

#ifdef CC1100_PINS_DEBUG
#    define CC1100_DBG_PINS(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_PINS(x...) do { } while (0)
#endif

#ifdef CC1100_EXCEPTION_DEBUG
#    define CC1100_DBG_EXC(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_EXC(x...) do { } while (0)
#endif

#ifdef CC1100_ACCESS_DEBUG
#    define CC1100_DBG_ACCESS(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_ACCESS(x...) do { } while (0)
#endif

#ifdef CC1100_REGISTER_DEBUG
#    define CC1100_DBG_REG(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_REG(x...) do { } while (0)
#endif

#ifdef CC1100_TX_DEBUG
#    define CC1100_DBG_TX(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_TX(x...) do { } while (0)
#endif

#ifdef CC1100_RX_DEBUG
#    define CC1100_DBG_RX(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_RX(x...) do { } while (0)
#endif

#ifdef CC1100_PACKET_DEBUG
#    define CC1100_DBG_PKT(x...) HW_DMSG(x)
#else
#    define CC1100_DBG_PKT(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

char* cc1100_status_to_str   (int status);
char* cc1100_state_to_str    (int state);
char* cc1100_strobe_to_str   (int strb);
char* cc1100_register_to_str (int regs);

/***************************************************/
/***************************************************/
/***************************************************/

#endif ///_CC1100_DEBUG_H


/**
 *  \file   wsnet2_dbg.h (inspired from wsnet1_dbg.h)
 *  \brief  WorldSens client debug msg v2
 *  \author Loic Lemaitre
 *  \date   2009
 **/


#ifndef _WSNET2_DEBUG_H
#define _WSNET2_DEBUG_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define WSNET2_DMSG_BKTRK  /* Backtrack debug messages  */
#define WSNET2_DMSG_EXC    /* Exceptions debug messages */
#define WSNET2_DMSG_DBG    /* General debug messages    */
#define WSNET2_DMSG_TX     /* TX debug messages         */
#define WSNET2_DMSG_RX     /* RX debug messages         */

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#ifdef WSNET2_DMSG_BKTRK
#    define WSNET2_BKTRK(x...) HW_DMSG(x)
#else
#    define WSNET2_BKTRK(x...) do { } while (0)
#endif

#ifdef WSNET2_DMSG_EXC
#    define WSNET2_EXC(x...) HW_DMSG(x)
#else
#    define WSNET2_EXC(x...) do { } while (0)
#endif

#ifdef WSNET2_DMSG_DBG
#    define WSNET2_DBG(x...) HW_DMSG(x)
#else
#    define WSNET2_DBG(x...) do { } while (0)
#endif

#ifdef WSNET2_DMSG_TX
#    define WSNET2_TX(x...) HW_DMSG(x)
#else
#    define WSNET2_TX(x...) do { } while (0)
#endif

#ifdef WSNET2_DMSG_RX
#    define WSNET2_RX(x...) HW_DMSG(x)
#else
#    define WSNET2_RX(x...) do { } while (0)
#endif

#define WSNET2_ERROR(x...) ERROR(x)

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

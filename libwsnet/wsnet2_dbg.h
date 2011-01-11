
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

#define WSNET2_DMSG_BKTRK 0  /* Backtrack debug messages                  */
#define WSNET2_DMSG_EXC   0  /* Exceptions debug messages                 */
#define WSNET2_DMSG_DBG   0  /* General debug messages                    */
#define WSNET2_DMSG_TX    0  /* TX debug messages                         */
#define WSNET2_DMSG_RX    0  /* RX debug messages                         */
#define WSNET2_DMSG_CNCT  0  /* Connection/disconnection messages         */

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if WSNET2_DMSG_BKTRK != 0
#    define WSNET2_BKTRK(x...) DMSG_LIB_WSNET(x)
#else
#    define WSNET2_BKTRK(x...) do { } while (0)
#endif

#if WSNET2_DMSG_EXC != 0
#    define WSNET2_EXC(x...)   DMSG_LIB_WSNET(x)
#else
#    define WSNET2_EXC(x...)   do { } while (0)
#endif

#if WSNET2_DMSG_DBG != 0
#    define WSNET2_DBG(x...)   DMSG_LIB_WSNET(x)
#else
#    define WSNET2_DBG(x...) do { } while (0)
#endif

#if WSNET2_DMSG_TX != 0
#    define WSNET2_TX(x...)    DMSG_LIB_WSNET(x)
#else
#    define WSNET2_TX(x...) do { } while (0)
#endif

#if WSNET2_DMSG_RX != 0
#    define WSNET2_RX(x...)    DMSG_LIB_WSNET(x)
#else
#    define WSNET2_RX(x...) do { } while (0)
#endif

#if WSNET2_DMSG_CNCT != 0
#    define WSNET2_CNCT(x...)  DMSG_LIB_WSNET(x)
#else
#    define WSNET2_CNCT(x...) do { } while (0)
#endif

#define WSNET2_ERROR(x...) ERROR(x)

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif


/**
 *  \file   wsnet1.h
 *  \brief  Worldsens client communication protocol v1 (deprecated)
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef WSNET1_H
#define WSNET1_H


#ifndef SOL_IP
#define SOL_IP 0
#endif //SOL_IP

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#if defined(__MINGW32__)
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <sys/signal.h>
  #include <sched.h>
#endif

#include <stdarg.h>
#include <time.h>

#include "wsnet1_dbg.h"

#define PACKED __attribute__((packed))

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* Maximum worldsens packet length */
#define WORLDSENS_MAX_PKTLENGTH		100

/* Control packet type */
#define WORLDSENS_C_CONNECT		0x01
#define WORLDSENS_C_SYNCHED		0x02
#define WORLDSENS_C_TX			0x04
#define WORLDSENS_C_DISCONNECT		0x08

#define WORLDSENS_S_ATTRADDR		0x01
#define WORLDSENS_S_NOATTRADDR		0x02
#define WORLDSENS_S_SYNCH_REQ		0x04
#define WORLDSENS_S_BACKTRACK		0x08
#define WORLDSENS_S_RX			0x10


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/* packet sent by a client to the server */

struct PACKED _worldsens_c_connect_pkt 
{
  char            type;
  int             node;
};

struct PACKED _worldsens_c_disconnect_pkt 
{
  char            type;
  int             node;
};

struct PACKED _worldsens_c_synched_pkt 
{
  char            type;
  int             rp_seq;
};

struct PACKED _worldsens_c_tx_pkt 
{
  char	          type;
  int	          node;
  uint64_t        period;
  uint64_t        duration;
  uint32_t        frequency;
  int             modulation;
  double          tx_mW;
  int	          pkt_seq;
  char	          data;
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/* packet sent by the server to a client */

struct PACKED _worldsens_s_header {         /* size 5 */
  char	          type;
  int	          pkt_seq;
};

struct PACKED _worldsens_s_connect_pkt {    /* size 17 */
  char	          type;
  int	          pkt_seq;
  uint64_t        period;
  int	          rp_seq;
  uint64_t        cnx_time;
};

struct PACKED _worldsens_s_backtrack_pkt {  /* size 17 */
  char	          type;
  int	          pkt_seq;
  uint64_t        period;
  int	          rp_seq;
};

struct PACKED _worldsens_s_saverel_pkt {    /* size 21 */
  char	          type;
  int	          pkt_seq;
  int	          c_rp_seq;
  uint64_t        period;
  int	          n_rp_seq;
};

struct PACKED _worldsens_s_srrx_pkt {       /* size 37 */
  char	          type;
  int	          pkt_seq;
  int	          c_rp_seq;
  uint64_t        period;
  int	          n_rp_seq;

  int	          size;
  int	          node;
  uint32_t        frequency;
  int	          modulation;
};

struct PACKED _worldsens_s_rx_pkt {         /* size 21 */
  char	          type;
  int	          pkt_seq;

  int	          size;
  int	          node;
  uint32_t        frequency;
  int	          modulation;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct _worldsens_c_nobacktrack {
  int	                  u_fd;            /* unicast                         */
  int	                  m_fd;            /* multicast                       */
  int	                  my_addr;         /* my address                      */
  wsnet_callback_rx_t     callback_rx;     /* backtrack from radio            */
  void                   *arg;             /* argument to backtracks          */
};


struct _worldsens_c_backtrack {
  int	                  tx_pkt_seq;      /* pkt_seq, Tx  to  WSNet          */
  int	                  rx_pkt_seq;      /* pkt_seq, Rx from WSNet          */

  int                     rp_seq;          /* sequence                        */
  uint64_t                next_rp;         /* time                            */
  uint64_t                last_rp;         /* time                            */

  char                    pending;
  char                    tx_backtracked;
  int64_t                 min_duration;    /* minimal duration after callback */

  struct _pktlist_t       pktlist;
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* wireless communication data and parameters */
/* information is received from the server    */

struct PACKED _worldsens_data {
  int             node;
  char		  data;
  double	  rx_mW;
  double	  SiNR;
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/



#endif

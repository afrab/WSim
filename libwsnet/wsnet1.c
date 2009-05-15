
/**
 *  \file   wsnet1.c
 *  \brief  WorldSens client communication protocol v1 (deprecated)
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2006 / 2007
 **/

/*
 *  wsnet1_clt.c
 *
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *  Created by Guillaume Chelius on 20/11/05.
 *  Rewrite Antoine Fraboulet, 2007
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "machine/machine.h"
#include "libselect/libselect.h"
#include "src/options.h"
#include "src/mgetopt.h"

#include "libwsnet.h"
#include "pktlist.h"
#include "wsnet1.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define DEBUG_PROTOCOL 1
#define VLVL 5
#define SNDSTR "snd --> "
#define RCVSTR "rcv <-- "

#if defined(DEBUG_PROTOCOL)
#  define PKT_DMP_CNX    1
#  define PKT_DMP_SYNCH  1
#  define PKT_DMP_TX     1
#  define PKT_DMP_RX     1
#  define PKT_DMP_DISCO  1
#else
#  define PKT_DMP_CNX    0
#  define PKT_DMP_SYNCH  0
#  define PKT_DMP_TX     0
#  define PKT_DMP_RX     0
#  define PKT_DMP_DISCO  0
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if defined(WORDS_BIGENDIAN)

#define SWAP8(name,type)           \
static inline type name(type v)    \
{                                  \
	return v;                  \
}

#else

#define SWAP8(name,type)           \
static inline type name (type v)   \
{                                  \
	type r;                    \
	uint8_t *pv, *pr;          \
	                           \
	pv = (uint8_t *) &v;       \
	pr = (uint8_t *) &r;       \
                                   \
	pr[0] = pv[7];             \
	pr[1] = pv[6];             \
	pr[2] = pv[5];             \
	pr[3] = pv[4];             \
	pr[4] = pv[3];             \
	pr[5] = pv[2];             \
	pr[6] = pv[1];             \
	pr[7] = pv[0];             \
                                   \
	return r;                  \
}

#endif

SWAP8( ntohll,  uint64_t )
SWAP8( htonll,  uint64_t )
SWAP8( ntohdbl, double   )
SWAP8( htondbl, double   )

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* public */
int             worldsens_c_options_add           (void);
void            worldsens_c_state_save            (void);
void            worldsens_c_state_restore         (void);
int             worldsens_c_get_node_id           (void);
void            worldsens_c_rx_register           (void* arg, wsnet_callback_rx_t cbrx);

int             worldsens_c_initialize            (void);
int             worldsens_c_close                 (void);
int             worldsens_c_tx                    (struct wsnet_tx_info *info);
int             worldsens_c_update                (void);

/* private */
static int      worldsens_option_validate         (void);   /* validate options                       */
static int      worldsens_close_fds               (void);   /* close socket fds                       */
static int      worldsens_connect                 (void);   /* snd connect pkt,    called by c_init.  */
static int      worldsens_connect_parse_reply     (char *msg, int UNUSED len);
static int      worldsens_disconnect              (void);   /* snd disconnect pkt, called by c_close  */
static ssize_t  worldsens_packet_send             (int fd, char* msg, size_t len, int flags, int dump);
static ssize_t  worldsens_packet_recv             (int fd, char* msg, size_t len, int flags, int dump);
static int      worldsens_packet_waiting_in_queue (int fd); /* pkt waiting in fd,  called by c_update */

static int64_t  worldsens_packet_parse_data       (char *msg, int pkt_seq, int line);
                                                            /* called by packet_parse                */
static int64_t  worldsens_packet_parse_rp         (char *msg, int pkt_seq, int line);
                                                            /* called by packet_parse                */
static int64_t  worldsens_packet_parse            (char *msg, int UNUSED len);
                                                            /* called by 
							     *    - c_update
							     *    - synched_parse_reply              
                                                             *    - tx_parse_reply 
							     */

static int64_t  worldsens_tx_parse_reply          (char *msg, int UNUSED len);
                                                            /* called by c_tx                         */
static int      worldsens_synched                 (int dmp);/* blocking synch,     called by c_update */

#if defined(DEBUG_PROTOCOL)
static void     worldsens_packet_dump_recv        (char *msg, int len);
static void     worldsens_packet_dump_send        (char *msg, int len);
#define WPDBG(x...) 
#else
#define worldsens_packet_dump_recv(x...) do { } while (0)
#define worldsens_packet_dump_send(x...) do { } while (0)
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static struct moption_t node_id_opt = {
  .longname    = "node-id",
  .type        = required_argument,
  .helpstring  = "worldsens node id",
  .value       = NULL
};

static struct moption_t server_ip_opt = {
  .longname    = "server-ip",
  .type        = required_argument,
  .helpstring  = "server ip address",
  .value       = NULL
};

static struct moption_t server_port_opt = {
  .longname    = "server-port",
  .type        = required_argument,
  .helpstring  = "server udp port",
  .value       = NULL
};

static struct moption_t multicast_ip_opt = {
  .longname    = "multicast-ip",
  .type        = required_argument,
  .helpstring  = "multicast ip address",
  .value       = NULL
};

static struct moption_t multicast_port_opt = {
  .longname    = "multicast-port",
  .type        = required_argument,
  .helpstring  = "multicast udp port",
  .value       = NULL
};

static struct _worldsens_c_nobacktrack  worldsens_nobacktrack;
static struct _worldsens_c_backtrack    worldsens_backtracked;
static struct _worldsens_c_backtrack    worldsens_backup;

#define WSENS_UNICAST           worldsens_nobacktrack.u_fd
#define WSENS_MULTICAST         worldsens_nobacktrack.m_fd
#define WSENS_MYADDR            worldsens_nobacktrack.my_addr

#define WSENS_CBRX_FUNC         worldsens_nobacktrack.callback_rx
#define WSENS_CBRX_ARG          worldsens_nobacktrack.arg

#define WSENS_SEQ_PKT_TX        worldsens_backtracked.tx_pkt_seq
#define WSENS_SEQ_PKT_RX        worldsens_backtracked.rx_pkt_seq
#define WSENS_SEQ_RDV           worldsens_backtracked.rp_seq     /* next RDV */

#define WSENS_RDV_LAST_TIME     worldsens_backtracked.last_rp
#define WSENS_RDV_NEXT_TIME     worldsens_backtracked.next_rp
#define WSENS_RDV_PENDING       worldsens_backtracked.pending

#define WSENS_TX_BACKTRACKED    worldsens_backtracked.tx_backtracked
#define WSENS_TIME_TO_WAIT      worldsens_backtracked.min_duration

#define WSENS_PKT_LIST          worldsens_backtracked.pktlist

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_options_add(void)
{
  options_add(& server_ip_opt      );
  options_add(& server_port_opt    );
  options_add(& multicast_ip_opt   );
  options_add(& multicast_port_opt );
  options_add(& node_id_opt        );
  return 0;
}

void worldsens_c_state_save     (void)
{
  memcpy(&worldsens_backup, &worldsens_backtracked, sizeof(struct _worldsens_c_backtrack));
}

void worldsens_c_state_restore  (void)
{
  memcpy(&worldsens_backtracked, &worldsens_backup, sizeof(struct _worldsens_c_backtrack));
}

int worldsens_c_get_node_id(void)
{
  return WSENS_MYADDR;
}

void worldsens_c_rx_register(void* arg, wsnet_callback_rx_t cbrx)
{
  WSENS_CBRX_ARG  = arg;	
  WSENS_CBRX_FUNC = cbrx;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_initialize(void)
{	
  int ret_connect;
  int on = 1;
  struct sockaddr_in addr;
  struct ip_mreq mreq;

  char *srv_addr;
  uint16_t srv_port;
  char *mul_addr;
  uint16_t mul_port;
  int node_id;

  /* parse options */
  worldsens_option_validate();

  srv_addr = server_ip_opt.value;
  srv_port = atoi(server_port_opt.value);
  mul_addr = multicast_ip_opt.value;
  mul_port = atoi(multicast_port_opt.value);
  node_id  = atoi(node_id_opt.value);

  /* Initialize */
  memset(&worldsens_nobacktrack, 0, sizeof(struct _worldsens_c_nobacktrack));
  memset(&worldsens_backtracked, 0, sizeof(struct _worldsens_c_backtrack));
  memset(&worldsens_backup,      0, sizeof(struct _worldsens_c_backtrack));

  WSENS_MYADDR = node_id;
	
  /* ************************************************** */
  /* Multicast socket                                   */
  /* ************************************************** */
  if ((WSENS_MULTICAST = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:ini:multicast:socket creation error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (socket):");
      return -1;
    }
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(mul_port);
  addr.sin_addr.s_addr = INADDR_ANY;
	
  /* Allow several bindings */
  if (setsockopt(WSENS_MULTICAST, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0 ) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:multicast:setsockopt REUSEADDR error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_initialize (setsockopt REUSEADDR):");
      close(WSENS_MULTICAST);
      return -1;
    }
	
#if !defined(LINUX)
  /* Allow several bindings */
  if (setsockopt(WSENS_MULTICAST, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) != 0 ) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:multicast:setsockopt REUSEPORT error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_initialize (setsockopt):"); 
      close(WSENS_MULTICAST); 
      return -1; 
    } 
#endif
	
  /* Bind */
  if (bind(WSENS_MULTICAST, (struct sockaddr *) (&addr), sizeof(addr)) != 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:multicast:bind error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (bind):");
      close(WSENS_MULTICAST);
      return -1;
    }
	
  /* Join */
  if (inet_aton(mul_addr, &mreq.imr_multiaddr) == 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:multicast:join address error on %s\n",mul_addr);
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (inet_aton):");
      close(WSENS_MULTICAST);
      return -1;
    }
  mreq.imr_interface.s_addr = INADDR_ANY;
  if (setsockopt(WSENS_MULTICAST, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 ) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:setsockopt IP_ADD_MEMBERSHIP error\n");
      ERROR("*   multicast configuration problem                \n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (setsockopt):");
      close(WSENS_MULTICAST);
      return -1;
    }
	
  /* ************************************************** */
  /* Unicast UDP socket                                 */
  /* ************************************************** */
  if ((WSENS_UNICAST = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:unicast:socket creation error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (socket):");
      return -1;
    }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0);
  addr.sin_addr.s_addr = INADDR_ANY;
	
  /* Bind */
  if (bind(WSENS_UNICAST, (struct sockaddr *) (&addr), sizeof(addr)) != 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:unicast:bind error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (bind):");
      worldsens_close_fds();
      return -1;
    }
	
  /* Connect */	
  addr.sin_port = htons(srv_port);
  if (inet_aton(srv_addr, &addr.sin_addr) == 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:unicast:address error on %s\n",srv_addr);
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (inet_aton):");
      worldsens_close_fds();
      return -1;
    }
  if (connect(WSENS_UNICAST, (struct sockaddr *) (&addr), sizeof(addr)) != 0) 
    {
      ERROR("* =================================================\n");
      ERROR("* worldsens:init:unicast:connect error\n");
      ERROR("* =================================================\n");
      perror("worldsens_c_intialize (connect):");
      worldsens_close_fds();
      return -1;
    }
  
  /* ************************************************** */
  /* Worldsens Connect                                  */
  /* ************************************************** */
  ret_connect = worldsens_connect();
  if (ret_connect == 0)
    {
      WSNET_DBG("WSNet:connect:ok, registering fd %d\n",WSENS_MULTICAST);
      assert(libselect_fd_register(WSENS_MULTICAST, SIG_WORLDSENS_IO) != -1);
    }
  return ret_connect;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_close(void) 
{
  worldsens_disconnect();
  worldsens_close_fds();
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_tx(struct wsnet_tx_info *info) 
{
  char data           = info->data;
  uint32_t frequency  = info->freq_mhz * 1000000;
  int modulation      = info->modulation;
  double tx_dBm       = info->power_dbm;
  uint64_t duration   = info->duration;

  struct _worldsens_c_tx_pkt pkt;
  int len = sizeof(pkt);

  /* Format */
  pkt.type            = WORLDSENS_C_TX;
  pkt.node            = htonl(WSENS_MYADDR);
  pkt.period          = htonll(MACHINE_TIME_GET_NANO() - WSENS_RDV_LAST_TIME);
  pkt.data            = data;
  pkt.frequency       = htonl(frequency);
  pkt.modulation      = htonl(modulation);
  pkt.tx_dBm          = htondbl(tx_dBm);
  pkt.pkt_seq         = htonl(WSENS_SEQ_PKT_TX);
  pkt.duration        = htonll(duration);

  /* Send */
  if (worldsens_packet_send(WSENS_UNICAST,(char*)(&pkt), len, 0, PKT_DMP_TX) <= 0)
    {
      ERROR("WSNet:tx: error during packet send\n");
      return -1;
    }

  WSENS_SEQ_PKT_TX ++;
				
  /* Wait */
  WSENS_TX_BACKTRACKED = 1;
  while (((MACHINE_TIME_GET_NANO() + duration) < WSENS_RDV_NEXT_TIME) && (WSENS_TX_BACKTRACKED == 1)) 
    {    	
      int  len;
      char msg[WORLDSENS_MAX_PKTLENGTH];

      /* Receive */
      if ((len = worldsens_packet_recv(WSENS_MULTICAST, msg, WORLDSENS_MAX_PKTLENGTH, 0, PKT_DMP_TX)) <= 0) 
	{
	  ERROR("WSNet:tx: error during packet recv\n");
	  return -1;
	}
		
      /* Parse */
      if (worldsens_tx_parse_reply(msg, len) == -1 ) 
	{
	  ERROR("WSNet:tx: error during packet parse\n");
	  return -1;      
	}
    }
	
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_packet_waiting_in_queue(int fd)
{
  int            res;
  fd_set         readfds;
  struct timeval timeout;
  
  FD_ZERO (&readfds);
  FD_SET  (fd, &readfds);
  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;
  
  if ((res = select(fd+1, &readfds, NULL, NULL, &timeout)) < 0) 
    {
      ERROR("WSNet:update: (%"PRIu64") error during select() - %s\n",MACHINE_TIME_GET_NANO(),strerror(errno));
      worldsens_close_fds();
      return -1;
    }
  return (res != 0) && FD_ISSET(fd, &readfds);
}
 
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_update(void) 
{
  int64_t duration;

  /* 
   *
   * 0 - fetch from socket if there is something to read 
   *      empties the input socket fifo and fills wsnetclient fifo
   *      return
   * 1 - wait minimum time for current wsnet action
   *      return
   * 2 - look for something in queue, most of the time packet continuation
   *      return
   * 3 - look for time synchro / Rendez-vous point
   *      return
   *
   */

  if ((mcu_signal_get() & SIG_WORLDSENS_IO) != 0)
    {
      int  len;
      char msg[WORLDSENS_MAX_PKTLENGTH];

      mcu_signal_remove(SIG_WORLDSENS_IO);
      do {
	if ((len = worldsens_packet_recv(WSENS_MULTICAST, msg, WORLDSENS_MAX_PKTLENGTH, 0, PKT_DMP_RX)) <= 0)  
	  {
	    ERROR("WSNet:update: error during packet receive\n");
	    return -1;
	  }
	else 
	  {
	    WSNET_DBG("WSNet:update:read: msg size %d\n",len);
	    if (pktlist_enqueue(& WSENS_PKT_LIST, msg, len) == -1)
	      {
		ERROR("WSNet:update:pktlist: error during enqueue\n");
		return -1;
	      }
	  }
      } while (worldsens_packet_waiting_in_queue(WSENS_MULTICAST));
    }

  /* **** 
   * minimum duration from previous packet == time precision 
   * 250ns == 2 cycles @ 8MHz
   **** */

#define WSNET_TIME_PRECISION 250

  /* *** Something to finish ? wait until simul is at least on time for a read **/
  if (WSENS_TIME_TO_WAIT >= WSNET_TIME_PRECISION)
    {
      WSENS_TIME_TO_WAIT -= MACHINE_TIME_GET_INCR();
      if (WSENS_TIME_TO_WAIT < WSNET_TIME_PRECISION)
	{
	  WSENS_TIME_TO_WAIT = 0;
	  WSNET_DBG("WSNet:update:%"PRIu64": min_duration is now below threshold\n",MACHINE_TIME_GET_NANO());
	}
      return 0;
    }

  /* *** Something waiting ? **/
  if (! pktlist_empty(& WSENS_PKT_LIST))
    {
      int len;
      char msg[WORLDSENS_MAX_PKTLENGTH];
      uint64_t current_rp = WSENS_RDV_NEXT_TIME;

      WSNET_DBG("WSNet:update:%"PRIu64": ==== update msg start =======\n",MACHINE_TIME_GET_NANO());

      /* get packet from the list */
      len = pktlist_dequeue(& WSENS_PKT_LIST, msg);

      if ((duration = worldsens_packet_parse(msg, len)) == -1) 
	{
	  ERROR("WSNet:update: error during packet parse\n");
	  return -1;
	}
      
      if (WSENS_RDV_NEXT_TIME != current_rp) 
	{
	  WSNET_DBG("WSNet:update: RP moved from %"PRIu64" to %"PRIu64"\n", 
		    MACHINE_TIME_GET_NANO(), current_rp, WSENS_RDV_NEXT_TIME);
	}

      WSENS_TIME_TO_WAIT = duration;
      if (duration > 0)
	{
	  WSNET_DBG("WSNet:update: event duration %"PRIu64"\n", duration);
	}
      
      WSNET_DBG("WSNet:update:%"PRIu64": ==== update msg stop =======\n",MACHINE_TIME_GET_NANO());
      return 0;
    }    

  /* *** Time to synchronize with WSNet **/
  if (MACHINE_TIME_GET_NANO() >= WSENS_RDV_NEXT_TIME) 
    {
      WSNET_DBG("WSNet:update:%"PRIu64": ==== update synched start ======\n",MACHINE_TIME_GET_NANO());
      if (worldsens_synched(PKT_DMP_SYNCH) < 0)
	{
	  WSNET_DBG("WSNET (%"PRIu64"): update synched error sigpipe\n",MACHINE_TIME_GET_NANO());
	  mcu_signal_add(SIG_HOST | SIGPIPE);
	}
      WSNET_DBG("WSNet:update:%"PRIu64": ==== update synched stop =======\n",MACHINE_TIME_GET_NANO());
    }
  return 0;
}

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

static int worldsens_option_validate(void)
{
  if (node_id_opt.value != NULL) {
    HW_DMSG_DEV(" - node id option value %s\n",node_id_opt.value);
  } else {
    node_id_opt.value = "1";
  }

  if (server_ip_opt.value != NULL) {
    HW_DMSG_DEV(" - server ip option value %s\n",server_ip_opt.value);
  } else {
    server_ip_opt.value = "127.0.0.1";
  }

  if (server_port_opt.value != NULL) {
    HW_DMSG_DEV(" - server port option value %d\n",atoi(server_port_opt.value));
  } else {
    server_port_opt.value = "9998";
  }

  if (multicast_ip_opt.value != NULL) {
    HW_DMSG_DEV(" - multicast ip option value %s\n",multicast_ip_opt.value);
  } else {
    multicast_ip_opt.value = "224.0.0.1";
  }

  if (multicast_port_opt.value != NULL) {
    HW_DMSG_DEV(" - multicast port option value %d\n",atoi(multicast_port_opt.value));
  } else {
    multicast_port_opt.value = "9999";
  }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_close_fds(void)
{
  if (WSENS_MULTICAST != -1)
    {
      close(WSENS_MULTICAST);
    }
  if (WSENS_UNICAST != -1)
    {
      close(WSENS_UNICAST);
    }

  WSENS_MULTICAST = -1;
  WSENS_UNICAST   = -1;
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_connect(void)
{
  int                              len,snd_len,rcv_len;
  struct _worldsens_c_connect_pkt  pkt;
  char                             msg[WORLDSENS_MAX_PKTLENGTH];
  
  WSNET_DBG("WSNet:connect:%"PRIu64": --> CONNECT (ip: %d) \n", MACHINE_TIME_GET_NANO(), WSENS_MYADDR);

  /* connect */
  pkt.type = WORLDSENS_C_CONNECT;
  pkt.node = htonl(WSENS_MYADDR);
  len = sizeof(struct _worldsens_c_connect_pkt);

  /* Send */
  if ((snd_len = worldsens_packet_send(WSENS_UNICAST,(char*)(&pkt), len, 0, PKT_DMP_CNX)) <= 0)
    {
      ERROR("WSNet:connect:pkt: send connect request error (len=%d)\n", snd_len);
      return -1;
    }
  	
  /* Receive */
  if ((rcv_len = worldsens_packet_recv(WSENS_UNICAST, msg, WORLDSENS_MAX_PKTLENGTH, 0, PKT_DMP_CNX)) <= 0)
    {
      ERROR("WSNet:connect:pkt: receive connect response error (len=%d)\n", rcv_len);
      return -1;
    }

  /* Parse */
  return worldsens_connect_parse_reply(msg, rcv_len);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_connect_parse_reply(char *msg, int UNUSED len)
{
  struct _worldsens_s_header *s_header  = (struct _worldsens_s_header *) msg;
  int                          pkt_type = s_header->type;
  int                          pkt_seq  = ntohl(s_header->pkt_seq);

  switch (pkt_type)
    {
    case WORLDSENS_S_NOATTRADDR: /* Connection refused */
      {
	WSNET_EXC("WSNet:connect: refused\n");
	worldsens_close_fds();
	return -1;
      }
    
    case WORLDSENS_S_ATTRADDR:   /* Connection accepted */
      {
	struct _worldsens_s_connect_pkt *pkt = (struct _worldsens_s_connect_pkt *) msg;
	WSENS_SEQ_PKT_RX     = pkt_seq;
	WSENS_SEQ_RDV        = ntohl(pkt->rp_seq);
	WSENS_RDV_LAST_TIME  = MACHINE_TIME_GET_NANO();
	WSENS_RDV_PENDING    = 0;
	WSENS_RDV_NEXT_TIME  = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);

	machine_state_save();
	WSNET_BKTRK("WSNet:backtrack: connect forces a state save at (time:%"PRIu64", seq:%d)\n",
		    MACHINE_TIME_GET_NANO(), pkt_seq);
	return 0;
      } 

    default:
      ERROR("WSNet:connect: reply packet parse error\n");
      worldsens_close_fds();
      return -1;
    }
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_disconnect() 
{
  int ret = 0;
  int len, slen;
  struct _worldsens_c_connect_pkt pkt;
	
  /* Send */
  pkt.type = WORLDSENS_C_DISCONNECT;
  pkt.node = htonl(WSENS_MYADDR);
  len = sizeof(struct _worldsens_c_connect_pkt);
  if ((slen = worldsens_packet_send(WSENS_UNICAST,(char*)(&pkt), len, 0, PKT_DMP_DISCO)) <= 0)
    {
      ERROR("WSNet:pkt: disconnect send error (len=%d)\n",slen);
      ret = -1;
    }
  else
    {
      WSNET_DBG("WSNet:disconnect:%"PRIu64": --> DISCONNECT (addr: %d) \n", 
		MACHINE_TIME_GET_NANO(), WSENS_MYADDR);
    }
  assert(libselect_fd_unregister(WSENS_MULTICAST) != -1);
  return ret;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static ssize_t worldsens_packet_send(int fd, char* msg, size_t len, int flags, int dump)
{
  int slen = 0;

  slen = send(fd,msg,len,flags);
  if (slen < (int)len)
    {
      ERROR("===================================================\n");
      ERROR("= worldsens:send error - %s\n", strerror(errno));
      ERROR("= fd=%d, msg=0x%x, len=%d, slen=%d, flags=%d, dump=%d\n", fd, msg, len, slen, flags, dump); 
      ERROR("= current time = %"PRIu64" ns\n",MACHINE_TIME_GET_NANO());
      ERROR("===================================================\n");
      perror("worldsens_packet_send");
      worldsens_close_fds();
    }
  if (dump)
    {
      worldsens_packet_dump_send(msg,slen);
    }
  return slen;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static ssize_t worldsens_packet_recv(int fd, char* msg, size_t len, int flags, int dump)
{
  ssize_t srec = 0;
  
  srec = recv(fd,msg,len,flags);
  if (srec <= 0)
    {
      ERROR("===================================================\n");
      ERROR("= worldsens:recv error - %s\n", strerror(errno));
      ERROR("= fd=%d, msg=0x%x, len=%d, srec=%d, flags=%d, dump=%d\n", fd, msg, len, srec, flags, dump); 
      ERROR("= current time = %"PRIu64" ns\n",MACHINE_TIME_GET_NANO());
      ERROR("===================================================\n");
      perror("worldsens_packet_recv");
      worldsens_close_fds();
    }
  if (dump)
    {
      worldsens_packet_dump_recv(msg,srec);
    }
  return srec;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int64_t worldsens_packet_parse_data(char *msg, int UNUSED pkt_seq, int UNUSED line)
{      
  struct _worldsens_s_srrx_pkt  *pkt = (struct _worldsens_s_srrx_pkt *) msg;
  struct _worldsens_data       *data = (struct _worldsens_data *)      (msg + sizeof(struct _worldsens_s_srrx_pkt));
  unsigned int                length = ntohl(pkt->size);
  int                         c_node = 0;
  int64_t                   duration = 0;

  /* c_node = number of extra '_worldsens_data' packet sent in the same rx_pkt, must be at least 1 */
  while (((c_node * sizeof(struct _worldsens_data)) + sizeof(struct _worldsens_s_rx_pkt)) < length) 
    {
      
      if (ntohl(data->node) == (unsigned)WSENS_MYADDR) 
	{
	  struct wsnet_rx_info info;
	  WSNET_RX("WSNET (%"PRIu64", %d): <-- RX src=%d[%d] (data: [0x%02x,%c], freq: %gMHz, mod: %d, rx: %lgdBm, SiNR: %lg)\n",
		   MACHINE_TIME_GET_NANO(), pkt_seq, 
		   /* RX_line[c_node] */ line, c_node,
		   data->data & 0xff, isprint(data->data & 0xff ) ? data->data & 0xff : '.',
		   ntohl(pkt->frequency) / 1000000.0f, 
		   ntohl(pkt->modulation), 
		   ntohdbl(data->rx_dBm), 
		   ntohdbl(data->SiNR));
	  
	  info.data       = data->data;
	  info.freq_mhz   = (double)ntohl(pkt->frequency) / 1000000.0;
	  info.modulation = ntohl(pkt->modulation);
	  info.power_dbm  = ntohdbl(data->rx_dBm);
	  info.SiNR       = ntohdbl(data->SiNR);

	  duration += WSENS_CBRX_FUNC(WSENS_CBRX_ARG, &info);
	}
      
      c_node++;
      data++; /* next data */
    }
  return duration;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int64_t worldsens_packet_parse_rp(char *msg, int UNUSED pkt_seq, int UNUSED line)
{
  struct _worldsens_s_saverel_pkt *pkt = (struct _worldsens_s_saverel_pkt *) msg;
  
  if (ntohl(pkt->c_rp_seq) == (unsigned)WSENS_SEQ_RDV) 
    {
      
      /* WSNET_DBG("WSNET (%"PRIu64", %d): <-- REL%d (seq: %d)\n",
	 MACHINE_TIME_GET_NANO(), pkt_seq, line, worldsens.rp_seq); */

      WSENS_RDV_LAST_TIME = MACHINE_TIME_GET_NANO();
      WSENS_RDV_PENDING   = 0;

      WSENS_RDV_NEXT_TIME = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);
      WSENS_SEQ_RDV       = ntohl(pkt->n_rp_seq);

      machine_state_save();
      WSNET_BKTRK("WSNet:backtrack: next RP forces a save state at (time:%"PRIu64", seq:%d)\n",
		  MACHINE_TIME_GET_NANO(), pkt_seq); 

      /* WSNET_DBG ("WSNET (%"PRIu64", %d): <-- RP%d (seq: %d, period:
	 %"PRIu64", next_rp: %"PRIu64")\n", MACHINE_TIME_GET_NANO(),
	 pkt_seq, line, worldsens.rp_seq, WSENS_NEXT_RDV_TIME -
	 MACHINE_TIME_GET_NANO(), WSENS_NEXT_RDV_TIME); */
    } 
  else 
    {
      ERROR("WSNet:rendez-vous: Deprecated RP = %d (received: %d, expected: %d)\n", 
	    pkt_seq, pkt->c_rp_seq, WSENS_SEQ_RDV);
    }
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int64_t worldsens_packet_parse(char *msg, int UNUSED len) 
{
  struct _worldsens_s_header *s_header  = (struct _worldsens_s_header *) msg;
  int                          pkt_type = s_header->type;
  int                          pkt_seq  = ntohl(s_header->pkt_seq);
  int64_t                      duration = 0;

  /* Packet sequence check */
  if (pkt_seq != WSENS_SEQ_PKT_RX)
    {
      if (pkt_seq > WSENS_SEQ_PKT_RX)
	{
	  WSNET_EXC("WSNET (%"PRIu64", %d): <-- Lost packet (expected: %d)\n", 
		    MACHINE_TIME_GET_NANO(), pkt_seq, WSENS_SEQ_PKT_RX);
	  return -1;
	}  
      else if (pkt_seq < WSENS_SEQ_PKT_RX)
	{
	  WSNET_EXC("WSNET (%"PRIu64", %d): <-- Deprecated packet (expected: %d)\n", 
		    MACHINE_TIME_GET_NANO(), pkt_seq, WSENS_SEQ_PKT_RX);
	  return 0;
	} 
    }

  /* next packet sequence */
  WSENS_SEQ_PKT_RX ++;

  switch (pkt_type)
    {
    case WORLDSENS_S_NOATTRADDR: /* Connection refused */
      ERROR("worldsens:parse: wrong packet type received (NOATTRADDR)\n");
      return -1;
    case WORLDSENS_S_ATTRADDR:   /* Connection accepted */
      ERROR("worldsens:parse: wrong packet type received (ATTRADDR)\n");
      return -1;

    case WORLDSENS_S_BACKTRACK:  /* RP point with backtrack */
      {
	struct _worldsens_s_backtrack_pkt *pkt = (struct _worldsens_s_backtrack_pkt *) msg;
	
	/*
	WSNET_DBG("WSNET (%"PRIu64", %d): <-- BACKTRACK (period: %"PRIu64"; time: %"PRIu64", seq: %d)\n", 
		  MACHINE_TIME_GET_NANO(), pkt_seq, 
		  ntohll(pkt->period), worldsens.last_rp + ntohll(pkt->period), pkt->rp_seq);
	*/

	if (MACHINE_TIME_GET_NANO() > (WSENS_RDV_LAST_TIME + ntohll(pkt->period))) 
	  {
	    WSNET_BKTRK("WSNet:backtrack: ");
	    WSNET_BKTRK("from (time:%"PRIu64",seq:%d) ", MACHINE_TIME_GET_NANO(), pkt_seq);
	    machine_state_restore(); /* backtracks also struct _worldsens_c */
	    WSNET_BKTRK(" to (time:%"PRIu64",seq:%d)\n", MACHINE_TIME_GET_NANO(), pkt_seq);
	  }
	
	WSENS_RDV_NEXT_TIME        = WSENS_RDV_LAST_TIME + ntohll(pkt->period);
	WSENS_SEQ_RDV              = ntohl(pkt->rp_seq);
	WSENS_SEQ_PKT_RX           = pkt_seq + 1;
	WSENS_TX_BACKTRACKED       = 0;
	
	/*
	WSNET_DBG("WSNET (%"PRIu64", %d): <-- RP (seq: %d, period: %"PRIu64", time: %"PRIu64")\n", 
		MACHINE_TIME_GET_NANO(), pkt_seq, worldsens.rp_seq, 
		ntohll(pkt->period), WSENS_NEXT_RDV_TIME);
	*/

	return 0;
      }
	
    case WORLDSENS_S_RX:                           /* Rx data */
      duration = worldsens_packet_parse_data(msg,pkt_seq,1);
      break;
  
    case WORLDSENS_S_SYNCH_REQ:                    /* RP point */
      worldsens_packet_parse_rp(msg,pkt_seq,1);
      return 0;

    case (WORLDSENS_S_SYNCH_REQ | WORLDSENS_S_RX): /* RP Point and Rx data */
      duration = worldsens_packet_parse_data  (msg,pkt_seq,2);
      worldsens_packet_parse_rp(msg,pkt_seq,2);
      break;

    default:
      WSNET_ERROR("WSNET:unknown packet type\n");
      break;
    }
  
  return duration;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int64_t worldsens_tx_parse_reply(char* msg, int UNUSED len)
{
  return worldsens_packet_parse(msg,len);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static int worldsens_synched(int dmp) 
{
  struct _worldsens_c_synched_pkt pkt;
  int len = sizeof(pkt);
	
  /* Send */
  pkt.type   = WORLDSENS_C_SYNCHED;
  pkt.rp_seq = htonl(WSENS_SEQ_RDV);
  if (worldsens_packet_send(WSENS_UNICAST,(char*)(&pkt), len, 0, dmp) <= 0)
    {
      ERROR("WSNet:synched: error during packet send\n");
      return -1;
    }

  /* Wait */
  WSENS_RDV_PENDING = 1;
  while (WSENS_RDV_PENDING)
    {
      char msg[WORLDSENS_MAX_PKTLENGTH];
      /* Receive */
      if ((len = worldsens_packet_recv(WSENS_MULTICAST, msg, WORLDSENS_MAX_PKTLENGTH, 0, dmp)) <= 0) 
	{
	  ERROR("WSNet:synched: error during packet receive\n");
	  return -1;
	}
      
      /* Parse */
      //      if (worldsens_synched_parse_reply(msg, len) == -1 ) 
      if (worldsens_packet_parse(msg, len) == -1 ) 
	{
	  ERROR("WSNet:synched: error during packet parse\n");
	  return -1;      
	}
    }
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* static int      worldsens_synched_parse_reply     (char *msg, int UNUSED len);  */
/* static int worldsens_synched_parse_reply(char *msg, int UNUSED len) */
/* { */
/*   struct _worldsens_c_connect_pkt *sh = (struct _worldsens_c_connect_pkt *)msg; */
/*   if ((sh->type & WORLDSENS_S_SYNCH_REQ) == 0) */
/*     { */
/*       ERROR("WSNet:pkt: wrong packet type during synched_parse_reply (%d,0x%02x)\n", */
/* 	    sh->type, sh->type); */
/*     } */
/*   return worldsens_packet_parse(msg,len); */
/* } */


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if defined(DEBUG_PROTOCOL)

static char *wsnet_modulation_name(int n)
{
  switch (n)
    {
    case WSNET_MODULATION_UNKNOWN    : return "Unknown"; 
    case WSNET_MODULATION_2FSK       : return "2FSK"; 
    case WSNET_MODULATION_GFSK       : return "GFSK"; 
    case WSNET_MODULATION_ASK_OOK    : return "OOK"; 
    case WSNET_MODULATION_MSK        : return "MSK"; 
    case WSNET_MODULATION_OQPSK      : return "OQPSK"; 
    case WSNET_MODULATION_OQPSK_REV  : return "OQPSK_REV"; 
    case WSNET_MODULATION_802_15_4   : return "802_15_4"; 
    }
  return "???";
}

/***************************************************/
/***************************************************/

static void wsnet_packet_dump(char *msg, int len, char *prfx)
{
  int i,c;
  for(i=0, c=0; i<len; i++)
    {
      switch (c)
	{
	case 0:
	  VERBOSE(VLVL,"WSNet:pkt:%s: %02x ", prfx, msg[i] & 0xff);
	  c = c+1;
	  break;
	case 4:
	  VERBOSE(VLVL," %02x ",  msg[i] & 0xff);
	  c = c+1;
	  break;
	case 7:
	  VERBOSE(VLVL,"%02x\n", msg[i] & 0xff);
	  c = 0;
	  break;
	default:
	  VERBOSE(VLVL,"%02x ",  msg[i] & 0xff);
	  c = c+1;
	  break;
	}
    }
 if (c != 0)
    {
      VERBOSE(VLVL,"\n");
    }
}

/***************************************************/
/***************************************************/

static void worldsens_packet_dump_send(char *msg, int len)
{
  char prfx[] = SNDSTR;
  struct _worldsens_c_connect_pkt *sh = (struct _worldsens_c_connect_pkt *)msg;

  VERBOSE(VLVL,"WSNet:pkt:%s: start ====================\n", prfx);
  switch (sh->type)
    {
    case WORLDSENS_C_CONNECT:
      {
	struct _worldsens_c_connect_pkt *pkt = (struct _worldsens_c_connect_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type %s\n",              prfx, "WORLDSENS_C_CONNECT");
	VERBOSE(VLVL,"WSNet:pkt:%s:   node %d\n",              prfx, ntohl(pkt->node));
      }
      break;
    case WORLDSENS_C_SYNCHED:
      {
	struct _worldsens_c_synched_pkt *pkt = (struct _worldsens_c_synched_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type   %s\n",            prfx, "WORLDSENS_C_SYNCHED");
	VERBOSE(VLVL,"WSNet:pkt:%s:   rp_seq %d\n",            prfx, ntohl(pkt->rp_seq));
      }
      break;
    case WORLDSENS_C_TX:
      {
	struct _worldsens_c_tx_pkt *pkt = (struct _worldsens_c_tx_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type       %s\n",          prfx, "WORLDSENS_C_TX");
	VERBOSE(VLVL,"WSNet:pkt:%s:   node       %d\n",          prfx, ntohl (pkt->node)     );
	VERBOSE(VLVL,"WSNet:pkt:%s:   period     %"PRIu64"ns\n", prfx, ntohll(pkt->period)   );
	VERBOSE(VLVL,"WSNet:pkt:%s:   duration   %"PRIu64"ns\n", prfx, ntohll(pkt->duration) );
	VERBOSE(VLVL,"WSNet:pkt:%s:   freq       %g MHz\n",      prfx, ntohl (pkt->frequency) / 1000000.0);
	VERBOSE(VLVL,"WSNet:pkt:%s:   modulation %s (%d)\n",     prfx, 
		wsnet_modulation_name(ntohl(pkt->modulation)), 
		ntohl(pkt->modulation));
	VERBOSE(VLVL,"WSNet:pkt:%s:   tx_dBm     %g\n",          prfx, ntohdbl(pkt->tx_dBm)  );
	VERBOSE(VLVL,"WSNet:pkt:%s:   pkt_seq    %d\n",          prfx, ntohl (pkt->pkt_seq)  );
	VERBOSE(VLVL,"WSNet:pkt:%s:   data       0x%02x (%c)\n", prfx, pkt->data & 0xff,
		isprint( pkt->data & 0xff) ? pkt->data & 0xff : '.');
      }
      break;
    case WORLDSENS_C_DISCONNECT:
      {
	struct _worldsens_c_disconnect_pkt *pkt = (struct _worldsens_c_disconnect_pkt *)msg; 
	VERBOSE(VLVL,"WSNet:pkt:%s:   type %s\n",              prfx, "WORLDSENS_C_DISCONNECT");
	VERBOSE(VLVL,"WSNet:pkt:%s:   node %d\n",              prfx, ntohl(pkt->node));
      }
      break;
    default:
      {
	VERBOSE(VLVL,"WSNet:pkt:%s:   %s\n", prfx, "UNKNOWN PACKET");
	VERBOSE(VLVL,"WSNet:pkt:%s:   type %d\n",              prfx, sh->type);
	VERBOSE(VLVL,"WSNet:pkt:%s:   size %d\n",              prfx, len);
	wsnet_packet_dump(msg, len, SNDSTR);
      }
      break;
    }
  VERBOSE(VLVL,"WSNet:pkt:%s: stop =====================\n", prfx);
}

/***************************************************/
/***************************************************/

static void worldsens_packet_dump_recv(char *msg, int len)
{
  char prfx[] = RCVSTR;
  struct _worldsens_s_connect_pkt *sh = (struct _worldsens_s_connect_pkt *)msg;

  VERBOSE(VLVL,"WSNet:pkt:%s: start ====================\n", prfx);
  switch (sh->type)
    {
    case WORLDSENS_S_ATTRADDR:
      {
	struct _worldsens_s_connect_pkt *pkt = (struct _worldsens_s_connect_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type    %s\n",           prfx, "WORLDSENS_S_ATTRADDR");
	VERBOSE(VLVL,"WSNet:pkt:%s:   pkt_seq %d\n",           prfx, ntohl (pkt->pkt_seq));
	VERBOSE(VLVL,"WSNet:pkt:%s:   period  %"PRIu64"\n",    prfx, ntohll(pkt->period));
	VERBOSE(VLVL,"WSNet:pkt:%s:   rp_seq  %d\n",           prfx, ntohl (pkt->rp_seq));
      }
      break;
    case WORLDSENS_S_NOATTRADDR:
      {
	struct _worldsens_s_header *pkt = (struct _worldsens_s_header *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type    %s\n",           prfx, "WORLDSENS_S_NOATTRADDR");
	VERBOSE(VLVL,"WSNet:pkt:%s:   pkt_seq %d\n",           prfx, ntohl(pkt->pkt_seq));
	VERBOSE(VLVL,"WSNet:pkt:%s:   no information\n",       prfx);
      }
      break;
    case WORLDSENS_S_RX:
    case WORLDSENS_S_SYNCH_REQ:
    case WORLDSENS_S_RX | WORLDSENS_S_SYNCH_REQ:
      {
	char *str;
	switch (sh->type) {
	case WORLDSENS_S_RX:        str = "WORLDSENS_S_RX";        break;
	case WORLDSENS_S_SYNCH_REQ: str = "WORLDSENS_S_SYNCH_REQ"; break;
	default:                    str = "WORLDSENS_S_ [ RX + SYNCH_REQ ]"; break;
	}
	VERBOSE(VLVL,"WSNet:pkt:%s:   type     %s\n",          prfx, str);
      };
      {
	struct _worldsens_s_saverel_pkt *pkt = (struct _worldsens_s_saverel_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   pkt_seq  %d\n",          prfx, ntohl(pkt->pkt_seq));
	VERBOSE(VLVL,"WSNet:pkt:%s:   c_rp_seq %d\n",          prfx, ntohl(pkt->c_rp_seq));
	VERBOSE(VLVL,"WSNet:pkt:%s:   period %"PRIu64"\n",     prfx, ntohll(pkt->period));
	VERBOSE(VLVL,"WSNet:pkt:%s:   n_rp_seq %d\n",          prfx, ntohl(pkt->n_rp_seq));
      }
      if ((sh->type & WORLDSENS_S_RX) != 0)
      {
	struct _worldsens_s_srrx_pkt *pkt = (struct _worldsens_s_srrx_pkt *)msg; 
	VERBOSE(VLVL,"WSNet:pkt:%s:   size       %d\n",        prfx, ntohl(pkt->size));
	VERBOSE(VLVL,"WSNet:pkt:%s:   node       %d\n",        prfx, ntohl(pkt->node));
	VERBOSE(VLVL,"WSNet:pkt:%s:   frequency  %g MHz\n",    prfx, ntohl(pkt->frequency) / 1000000.0f);
	VERBOSE(VLVL,"WSNet:pkt:%s:   modulation %s (%d)\n",   prfx, 
		wsnet_modulation_name(ntohl(pkt->modulation)), 
		ntohl(pkt->modulation));
      }
      break;
    case WORLDSENS_S_BACKTRACK:
      {
	struct _worldsens_s_backtrack_pkt *pkt = (struct _worldsens_s_backtrack_pkt *)msg;
	VERBOSE(VLVL,"WSNet:pkt:%s:   type     %s\n",          prfx, "WORLDSENS_S_BACKTRACK");
	VERBOSE(VLVL,"WSNet:pkt:%s:   pkt_seq  %d\n",          prfx, ntohl (pkt->pkt_seq));
	VERBOSE(VLVL,"WSNet:pkt:%s:   period   %"PRIu64"\n",   prfx, ntohll(pkt->period));
	VERBOSE(VLVL,"WSNet:pkt:%s:   rp_seq   %d\n",          prfx, ntohl (pkt->rp_seq));
      }
      break;
    default:
      {
	VERBOSE(VLVL,"WSNet:pkt:%s:   %s\n",                   prfx, "UNKNOWN PACKET");
	VERBOSE(VLVL,"WSNet:pkt:%s:   type %02x\n",            prfx, sh->type);
	VERBOSE(VLVL,"WSNet:pkt:%s:   size %d\n",              prfx, len);
	wsnet_packet_dump(msg, len, RCVSTR);
      }
      break;
    }
  VERBOSE(VLVL,"WSNet:pkt:%s: stop =====================\n", prfx);
}
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


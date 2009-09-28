/**
 *  \file   wsnet2_net.c
 *  \brief  WorldSens client v2, network layer
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2007
 **/

#ifndef SOL_IP
#define SOL_IP 0
#endif //SOL_IP

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sched.h>
#include <sys/signal.h>
#include <time.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "machine/machine.h"
#include "libselect/libselect.h"
#include "liblogger/logger.h"

#include "pktlist.h"
#include "wsnet2_pkt.h"
#include "wsnet2_net.h"
#include "wsnet2_dbg.h"

/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define DMSG(x...) VERBOSE(3,x)
#else
#define DMSG(x...) do {} while(0)
#endif

/* ************************************************** */
/* ************************************************** */

/*  wsnet1 modulations ids  :      0     1     2     3     4     5     6     7    */
/*  wsnet2 modulations names:   |name0|name1|name2|name3|name4|name5|name6|name7| */
char *wsnet_mod_name_map[WSNET_MAX_MODULATIONS] = {"modulation_none",   /* WSNET_MODULATION_UNKNOWN   */
						   "modulation_fsk",    /* WSNET_MODULATION_2FSK      */
						   "modulation_fsk",    /* WSNET_MODULATION_GFSK      */
						   "modulation_none",   /* WSNET_MODULATION_ASK_OOK   */
						   "modulation_none",   /* WSNET_MODULATION_MSK       */
						   "modulation_oqpsk",  /* WSNET_MODULATION_OQPSK     */
						   "modulation_oqpsk",  /* WSNET_MODULATION_OQPSK_REV */
						   "modulation_oqpsk"}; /* WSNET_MODULATION_802_15_4  */

/*  wsnet1 modulations ids  :      0     1     2     3     4     5     6     7    */
/*  wsnet2 modulations ids  :   | id0 | id1 | id2 | id3 | id4 | id5 | id6 | id7 | */
int wsnet_mod_id_map[WSNET_MAX_MODULATIONS] =     {-1,                  /* WSNET_MODULATION_UNKNOWN   */
						   -1,                  /* WSNET_MODULATION_2FSK      */
						   -1,                  /* WSNET_MODULATION_GFSK      */
						   -1,                  /* WSNET_MODULATION_ASK_OOK   */
						   -1,                  /* WSNET_MODULATION_MSK       */
						   -1,                  /* WSNET_MODULATION_OQPSK     */
						   -1,                  /* WSNET_MODULATION_OQPSK_REV */
						   -1};                 /* WSNET_MODULATION_802_15_4  */

/* ************************************************** */
/* ************************************************** */

#define WORLDSENS_SAVE_STATE() {            \
    wsens.l_rp  = MACHINE_TIME_GET_NANO();  \
    wsens.state = WORLDSENS_CLT_STATE_IDLE; \
    machine_state_save();                   \
    WSNET2_DBG("Libwsnet2:WORLDSENS_SAVE_STATE: Last restoration point sets at %lld\n",wsens.l_rp);	\
}   

/* ************************************************** */
/* ************************************************** */

static int      wsnet2_sync           (void);
static int      wsnet2_parse          (char *);
static int      wsnet2_seq            (char *);
static int      wsnet2_published      (char *);
static int      wsnet2_backtrack      (char *);
static int      wsnet2_sync_release   (char *);
static int      wsnet2_rx             (char *);
static int      wsnet2_sr_rx          (char *);
static int      wsnet2_measure_rsp    (char *);
static int      wsnet2_measure_sr_rsp (char *);
static int      wsnet2_subscribe      (void);

/* ************************************************** */
/* ************************************************** */

struct _worldsens_clt wsens;


void wsnet2_init(void) {
    int i = MAX_CALLBACKS;

    wsens.u_fd        = -1;
    wsens.m_fd        = -1;
    wsens.n_update    =  0;
    wsens.rpseq       =  1;
    wsens.l_rp        =  0;
    wsens.nb_radios   =  0;
    wsens.nb_measures =  0;

    while (i--) {
        wsens.radio[i].callback   = NULL;
        wsens.measure[i].callback = NULL;
    }
    WSNET2_DBG("Libwsnet2:wsnet2_init: WSNet2 interface initialized\n");

}


void wsnet2_finalize(void) {

    if (wsens.u_fd > 0)
        close(wsens.u_fd);
    if (wsens.m_fd > 0)
        close(wsens.m_fd);

    wsens.u_fd = -1;
    wsens.m_fd = -1;
    WSNET2_DBG("Libwsnet2:wsnet2_finalize: WSNet2 interface closed\n");
}


/* ************************************************** */
/* ************************************************** */
uint32_t wsnet2_get_node_id(void)
{
  return wsens.id;
}


/* ************************************************** */
/* ************************************************** */

int wsnet2_register_radio(char *antenna, wsnet_callback_rx_t callback, void *arg) {
    int i = 0;

    while ((wsens.radio[i].callback != NULL) && (++i < MAX_CALLBACKS)) ;

    if (i == MAX_CALLBACKS) {
        ERROR("Libwsnet2:wsnet2_register_radio: Too many registered radio callbacks\n");
        return -1;
    }
 
    wsens.radio[i].callback   = callback;
    wsens.radio[i].arg        = arg;
    wsens.radio[i].antenna    = malloc(strlen(antenna));
    strcpy(wsens.radio[i].antenna, antenna);

    wsens.nb_radios++;

    WSNET2_DBG("Libwsnet2:wsnet2_register_radio: Radio with antenna '%s' registered in position %d\n", wsens.radio[i].antenna, i); 

    return i;
}

int wsnet2_register_measure(char *name, wsnet_callback_measure_t callback, void *arg) {
    int i = 0;

    while ((wsens.measure[i].callback != NULL) && (++i < MAX_CALLBACKS)) ;

    if (i == MAX_CALLBACKS)  {
        ERROR("Libwsnet2:wsnet2_register_measure: Too many registered measure callbacks\n");
        return -1;
    }
        
    wsens.measure[i].callback = callback;
    wsens.measure[i].arg      = arg;
    wsens.measure[i].name     = malloc(strlen(name));
    strcpy(wsens.measure[i].name, name);

    wsens.nb_measures++;

    WSNET2_DBG("Libwsnet2:wsnet2_register_measure: Measure '%s' registered in position %d\n", wsens.measure[i].name, i); 

    return i;
}


/* ************************************************** */
/* ************************************************** */
int wsnet2_connect(char *s_addr, uint16_t s_port, char *m_addr, uint16_t m_port, uint32_t id) {
    int on = 1, ret;
    struct sockaddr_in addr;
    struct ip_mreq     mreq;


    /* my id */
    wsens.id = id;
	
    /* multicast socket */
    if ((wsens.m_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("(socket):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during multicast socket creation\n");
        goto error;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
	
    /* allow several bindings */
    if (setsockopt(wsens.m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0 ) {
        perror("(setsockopt):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during multicast socket configuration\n");
        goto error;
    }
	
#if !defined(LINUX)
    /* allow several bindings */
    if (setsockopt(wsens.m_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) != 0 ) { 
        perror("(setsockopt):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during multicast socket configuration for Linux\n"); 
        goto error;
    } 
#endif
	
    /* bind */
    if (bind(wsens.m_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(bind):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during multicast socket binding\n");
        goto error;
    }
	
    /* join */
    if (inet_aton(m_addr, &mreq.imr_multiaddr) == 0) {
        perror("(inet_aton):");
	WSNET2_EXC("Error during multicast socket joining\n");
        goto error;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(wsens.m_fd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 ) {
        perror("(setsockopt):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during multicast socket configuration\n");
        goto error;
    }
	
    /* unicast socket */
    if ((wsens.u_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("(socket):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during unicast socket creation\n");
        goto error;
    }
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(0);
    addr.sin_addr.s_addr = INADDR_ANY;
	
    /* bind */
    if (bind(wsens.u_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(bind):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during unicast socket binding\n");
        goto error;
    }
	
    /* connect */	
    addr.sin_port = htons(s_port);
    if (inet_aton(s_addr, &addr.sin_addr) == 0) {
        perror("(inet_aton):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during unicast socket joining\n");
        goto error;
    }
    if (connect(wsens.u_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(connect):");
	WSNET2_EXC("Libwsnet2:wsnet2_connect: Error during unicast socket connection\n");
        goto error;
    }
	
    /* subscribe to server */
    ret = wsnet2_subscribe();
    if (!ret)
      libselect_fd_register(wsens.m_fd, SIG_WORLDSENS_IO);   /* TODO: find why errors occur when enabled */
 
    return ret;

 error:
    wsnet2_finalize();
    ERROR("Libwsnet2:wsnet2_connect: Error during connection initialization...\n");
    return -1;
}


/* ************************************************** */
/* ************************************************** */
/**
 * Sends registering request to wsnet, and wait for response 
 **/
int wsnet2_subscribe(void) {
    union _worldsens_pkt pkt;
    char msg[WORLDSENS_MAX_PKTLENGTH];
    int len;
    
    /* format */
    pkt.cnx_req.type    = WORLDSENS_C_CONNECT_REQ;
    pkt.cnx_req.node_id = wsens.id;
    worldsens_packet_dump(&pkt);
    worldsens_packet_hton(&pkt);

	
    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_connect_req), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    WSNET2_DBG("Libwsnet2:wsnet2_subscribe: Attempting to connect with id %d...\n", wsens.id);

    /* wait for server response */
    wsens.state = WORLDSENS_CLT_STATE_CONNECTING;
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
        /* receive */
      WSNET2_DBG("Libwsnet2:wsnet2_subscribe: Waiting for server response... \n");
        if ((len = recv(wsens.u_fd, msg, WORLDSENS_MAX_PKTLENGTH, 0)) < 0) {
            perror("(recv)");
            goto error;
        }
        if (wsnet2_parse(msg))
	  return -1;
    }

    WSNET2_DBG("Libwsnet2:wsnet2_subscribe: Connection to server successfull \n");

    return 0; 

 error:
    ERROR("Libwsnet2:wsnet2_subscribe: Error when receiving subscribe response\n");
    wsnet2_finalize();
    return -1;
}


int wsnet2_unsubscribe(void) {
    union _worldsens_pkt pkt;
    
    /* format */
    pkt.disconnect.type    = WORLDSENS_C_DISCONNECT;
    pkt.disconnect.node_id = wsens.id;
    worldsens_packet_hton(&pkt);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_disconnect), 0) < 0) {
        perror("(send)");
	worldsens_packet_dump(&pkt);
        goto error;
    }
    WSNET2_DBG("Libwsnet2:wsnet2_unsubscribe: Disconnected id %d\n", wsens.id);
	
    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_unsubscribe: Error when sending unsubscribe request\n");
    wsnet2_finalize();
    return -1;
}


/* ************************************************** */
/* ************************************************** */
int wsnet2_update(void) {
    char msg[WORLDSENS_MAX_PKTLENGTH];
    int len, ret;
    fd_set readfds;
    struct timeval timeout;

    /* handle received packets */
    if ((mcu_signal_get() & SIG_WORLDSENS_IO) != 0) {
        mcu_signal_remove(SIG_WORLDSENS_IO);
        
        do {
            /* receive */
            if ((len = recv(wsens.m_fd, msg, WORLDSENS_MAX_PKTLENGTH, 0)) <= 0) {
                perror("(recv)");
                goto error;
            }
            
            /* parse */
            if (wsnet2_parse(msg))
                return -1;
            
            /* remaining packets ? */
            FD_ZERO(&readfds);
            FD_SET(wsens.m_fd, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            
            if ((ret = select(wsens.m_fd+1, &readfds, NULL, NULL, &timeout)) < 0) {
                perror("(select)");
                goto error;
            }
        } while (ret != 0 && FD_ISSET(wsens.m_fd, &readfds));    
    }

    /* synched */
    if (MACHINE_TIME_GET_NANO() >= wsens.n_rp) {
        wsnet2_sync();
    }

    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_update: Error when updating libwsnet2");
    wsnet2_finalize();
    return -1;
}


/* ************************************************** */
/* ************************************************** */
/**
 * Called when libwsnet is sync on rp. Wait for a new rp.
 **/
static int wsnet2_sync(void) {
    char msg[WORLDSENS_MAX_PKTLENGTH];
    union _worldsens_pkt pkt;
    int len;
	
    /* format */
    pkt.sync_ack.type        = WORLDSENS_C_SYNC_ACK;
    pkt.sync_ack.node_id     = wsens.id;
    pkt.sync_ack.rp_id       = wsens.rpseq;
    worldsens_packet_dump(&pkt);
    worldsens_packet_hton(&pkt);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_sync_ack), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    WSNET2_DBG("Libwsnet2:wsnet2_sync: synched on rp %d\n", wsens.rpseq);

	
    /* wait for new rp */
    wsens.state = WORLDSENS_CLT_STATE_PENDING;
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, WORLDSENS_MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recvfrom)");
            goto error;
        }
		
        /* parse */
        if (wsnet2_parse(msg))
            return -1;      
		
    }
	
    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_sync: Error during synchronization\n");
    wsnet2_finalize();
    return -1;
}


/* ************************************************** */
/* ************************************************** */
/**
 * TX data to wsnet
 **/
int wsnet2_tx(char data, double freq, int mod, double txdB, uint64_t delay, int radio_id) {
    char msg[WORLDSENS_MAX_PKTLENGTH];
    union _worldsens_pkt pkt;
    int len;

    WSNET2_TX("Libwsnet2:entering in wsnet2_tx	\n");

    /* format */
    pkt.byte_tx.type              = WORLDSENS_C_BYTE_TX;
    pkt.byte_tx.node_id           = wsens.id;
    pkt.byte_tx.period            = MACHINE_TIME_GET_NANO() - wsens.l_rp;
    pkt.byte_tx.data              = data;
    pkt.byte_tx.freq              = freq;
    pkt.byte_tx.antenna_id        = wsens.radio[radio_id].antenna_id;
    pkt.byte_tx.wsnet_mod_id      = wsnet_mod_id_map[mod];
    pkt.byte_tx.wsim_mod_id       = mod;
    pkt.byte_tx.power_dbm         = txdB;
    pkt.byte_tx.duration          = delay;
 
    worldsens_packet_dump(&pkt);
    worldsens_packet_hton(&pkt);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_byte_tx), 0)  < 0) {
        perror("(send)");
        goto error;
    }

    WSNET2_TX("Libwsnet2:wsnet2_tx: machine time=%lld\n", MACHINE_TIME_GET_NANO());
    WSNET2_TX("Libwsnet2:wsnet2_tx: wsens last rp=%lld\n", wsens.l_rp);
    WSNET2_TX("Libwsnet2:wsnet2_tx: packet 0x%02x sent\n", data);
    
    /* wait either for backtrack or for new rp */
    wsens.state = WORLDSENS_CLT_STATE_TXING;
    WSNET2_TX("Libwsnet2:wsnet2_tx: MACHINE_TIME_GET_NANO() + delay < wsens.n_rp? : delay = %lld, wsens.n_rp = %lld\n", delay, wsens.n_rp);
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, WORLDSENS_MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recv)");
            goto error;
        }
		
        /* parse */
        if (wsnet2_parse(msg))
            return -1;
    }
	
    return 0;

 error:
    WSNET2_DBG("Libwsnet2:wsnet2_tx: error during tx\n");
    wsnet2_finalize();
    return -1;
}


/* ************************************************** */
/* ************************************************** */
/**
 * TX a measure request to wsnet
 **/
int wsnet2_tx_measure_req(int measure_pos_id) {
    char msg[WORLDSENS_MAX_PKTLENGTH];
    union _worldsens_pkt pkt;
    int len;

    /* format */
    pkt.measure_req.type       = WORLDSENS_C_MEASURE_REQ;
    pkt.measure_req.node_id    = wsens.id;
    pkt.measure_req.measure_id = wsens.measure[measure_pos_id].id;
    pkt.measure_req.period     = MACHINE_TIME_GET_NANO() - wsens.l_rp;

    worldsens_packet_dump(&pkt);
    worldsens_packet_hton(&pkt);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_byte_tx), 0)  < 0) {
        perror("(send)");
        goto error;
    }

    /* wait for new rp */
    wsens.state = WORLDSENS_CLT_STATE_TXING;

    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, WORLDSENS_MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recv)");
            goto error;
        }
		
        /* parse */
        if (wsnet2_parse(msg))
            return -1;
    }

    return 0;

 error:
    WSNET2_DBG("Libwsnet2:wsnet2_tx_measure_req: Error during tx measure req\n");
    wsnet2_finalize();
    return -1;
}


/* ************************************************** */
/* ************************************************** */
static int wsnet2_parse(char *msg) {
  union _worldsens_pkt *pkt = (union _worldsens_pkt *) msg;

  worldsens_packet_ntoh(pkt);
  worldsens_packet_dump(pkt);

  switch (pkt->s_header.type) {
 
  case WORLDSENS_S_CONNECT_RSP_OK:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_CONNECT_RSP_OK packet type\n");
      if(wsnet2_published((char *)pkt)){
	  WSNET2_DBG("Libwsnet2:wsnet2_parse: Error during publishing models\n");	
          return -1;
      }
    break;
  case WORLDSENS_S_CONNECT_RSP_NOK:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_CONNECT_RSP_NOK packet type\n");
      WSNET2_EXC("Libwsnet2:wsnet2_parse: Connection refused by wsnet2 server\n");
      wsnet2_finalize();
      return -1;
  case WORLDSENS_S_BACKTRACK:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_BACKTRACK packet type\n");
      if (wsnet2_backtrack((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_SYNC_RELEASE:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_SYNC_RELEASE packet type\n");
      if (wsnet2_sync_release((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_BYTE_RX:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_BYTE_RX packet type\n");
      if (wsnet2_rx((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_BYTE_SR_RX:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_BYTE_SR_RX packet type\n");
      if (wsnet2_sr_rx((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_MEASURE_RSP:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_MEASURE_RSP packet type\n");
      if (wsnet2_measure_rsp((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_MEASURE_SR_RSP:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_MEASURE_SR_RSP packet type\n");
      if (wsnet2_measure_sr_rsp((char *)pkt))
          goto error;
      break;
  case WORLDSENS_S_KILLSIM:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_KILLSIM packet type\n");
      wsnet2_finalize();
      break;
  default:
      ERROR("Libwsnet2:wsnet2_parse: Unknown packet type!\n");
      goto error;
  }

  return 0;

 error:
    ERROR("Libwsnet2:wsnet2_parse: Error during packet parse\n");
    wsnet2_finalize();
    return -1;	
}


/* ************************************************** */
/* ************************************************** */
/**
 * Check if packet sequence is correct
 **/
static int wsnet2_seq(char *msg) {
    struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg;

    if (header->seq > wsens.seq) {
       ERROR("Libwsnet2:wsnet2_seq: Lost wsens packet (received: %lld while expecting %lld)\n", header->seq, wsens.seq);
       return -1;
}  else if (header->seq < wsens.seq) {
       ERROR("Libwsnet2:wsnet2_seq: Deprecated wsens packet (received: %lld while expecting %lld)\n", header->seq, wsens.seq);
       return -2;
   }
   wsens.seq++;
   WSNET2_DBG("Libwsnet2:wsnet2_seq: Packet sequence incremented (seq=%lld)\n", wsens.seq);
   return 0;
}


/* ************************************************** */
/* ************************************************** */
/**
 * Deal with wsnet connection informations : available antennas, modulations, and measures.
 **/
static int wsnet2_published(char *msg) {
   union _worldsens_pkt *pkt = (union _worldsens_pkt *) msg;
   
   int offset = 0;
   int match_antennas    = 0;
   int match_modulations = 0;
   int match_measures    = 0;
   uint32_t counter  = 0;
   uint32_t nb_models;
   int i = 0;

   /* update */
   wsens.seq = pkt->cnx_rsp_ok.seq; /* initialize next expected packet sequence */
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + pkt->cnx_rsp_ok.rp_duration;
   wsens.rpseq = pkt->cnx_rsp_ok.rp_next;
   WORLDSENS_SAVE_STATE();

   WSNET2_BKTRK("Libwsnet2:wsnet2_published: Connect forces a state save at (time:%lld, seq:%d)\n",
		MACHINE_TIME_GET_NANO(), wsens.seq - 1);

   nb_models = pkt->cnx_rsp_ok.n_antenna_id + pkt->cnx_rsp_ok.n_modulation_id + pkt->cnx_rsp_ok.n_measure_id;

   /* parses wsnet models list */

   /* pkt->cnx_rsp_ok.names_and_ids format: */

/*|***************antennas***************|**************modulations*************|***************measures***************|*/
/*|ant id1|ant name1|ant id2|ant name2|..|mod id1|mod name1|mod id2|mod name2|..|mea id1|mea name1|mea id2|mea name2|..|*/
/*  *********************************************************************************************************************/

   /* ******************************antennas******************************** */
   counter = 0;
   i = 0;
   while (counter < pkt->cnx_rsp_ok.n_antenna_id) {
       WSNET2_DBG("Libwsnet2:wsnet2_published: Checking antenna '%s' with id %d\n", 
		  pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t), 
		  *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset)));
       while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	   if (strcmp(wsens.radio[i].antenna, pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t)) == 0) {
	       wsens.radio[i].antenna_id = *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset));
	       WSNET2_DBG("Libwsnet2:wsnet2_published: Antenna '%s' matches\n", wsens.radio[i].antenna);
	       match_antennas++;
	   }
	   i++;
       }
       offset += strlen(pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t)) + sizeof(uint32_t) + 1;
       counter++;
   }

   /* checks if we have as many antennas as radios registered */
   if(match_antennas != wsens.nb_radios){
       ERROR("Libwsnet2:wsnet2_published: Wrong number of antennas registered\n");
       return -1;
   }



   /* *****************************modulations******************************* */
   /* initialize a table of correspondance between wsnet modulation ids and wsim modulation ids */
   counter = 0;
   while (counter < pkt->cnx_rsp_ok.n_modulation_id) {
       WSNET2_DBG("Libwsnet2:wsnet2_published: Checking modulation '%s' with id %d\n", 
		  pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t), 
		  *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset)));
       for (i = 0; i < WSNET_MAX_MODULATIONS; i++) {
	   if (strcmp(pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t), wsnet_mod_name_map[i]) == 0) {
	       wsnet_mod_id_map[i] =  *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset));
	   }
       }
       offset += strlen(pkt->cnx_rsp_ok.names_and_ids + offset +  sizeof(uint32_t)) + sizeof(uint32_t) + 1;
       counter++;
   }

   /* ******************************measures*********************************** */
   counter = 0;
   i = 0;
   while (counter < pkt->cnx_rsp_ok.n_measure_id) {
       WSNET2_DBG("Libwsnet2:wsnet2_published: Checking measure '%s' with id %d\n", 
		  pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t), 
		  *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset)));
       while ((wsens.measure[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	   if (strcmp(wsens.measure[i].name, pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t)) == 0) {
	       wsens.measure[i].id = *((uint32_t *)(pkt->cnx_rsp_ok.names_and_ids + offset));
	       WSNET2_DBG("Libwsnet2:wsnet2_published: Measure '%s' matches\n", wsens.measure[i].name);
	       match_measures++;
	   }
	   i++;
       }
       offset += strlen(pkt->cnx_rsp_ok.names_and_ids + offset + sizeof(uint32_t)) + sizeof(uint32_t) + 1;
       counter++;
   }
   /* checks if we have as many measures as measures registered */
   if(match_measures != wsens.nb_measures){
       ERROR("Libwsnet2:wsnet2_published: Wrong number of measures registered\n");
       return -1;
   }

   return 0;

}

/* ************************************************** */
/* ************************************************** */
/**
 * Backtrack requested by wsnet
 **/
static int wsnet2_backtrack(char *msg) {
   struct _worldsens_s_backtrack *pkt = (struct _worldsens_s_backtrack *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (MACHINE_TIME_GET_NANO() > (wsens.l_rp + pkt->rp_duration)) {
     WSNET2_BKTRK("Libwsnet2:wsnet2_backtrack: Backtracking to time %lld\n", wsens.l_rp);
       machine_state_restore();   
   } else {
       WSNET2_BKTRK("Libwsnet2:wsnet2_backtrack: No need to backtrack\n");
   }

   wsens.rpseq = pkt->rp_next;
   wsens.n_rp  = wsens.l_rp + pkt->rp_duration;
   wsens.state = WORLDSENS_CLT_STATE_IDLE;

   return 0;
}


/* ************************************************** */
/* ************************************************** */
/**
 * Program next RP and save state.
 **/
static int wsnet2_sync_release(char *msg) {
   struct _worldsens_s_sync_release *pkt = (struct _worldsens_s_sync_release *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WORLDSENS_CLT_STATE_PENDING) {
       ERROR("Libwsnet2:wsnet2_sync_release: Received a release order while not synched (state: %d)\n", wsens.state);
       return -1;
   }

   WSNET2_DBG("Libwsnet2:wsnet2_sync_release: Released\n");

   wsens.rpseq = pkt->rp_next;
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + pkt->rp_duration;
   WORLDSENS_SAVE_STATE();
   
   return 0;
}


/* ************************************************** */
/* ************************************************** */
/**
 * RX data. No new RP -> future event at same time.
 **/
static int wsnet2_rx(char *msg) {
   struct _worldsens_s_byte_rx *pkt  = (struct _worldsens_s_byte_rx *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;

   /* rx data and eventually callback radio */
   int i = 0;
   while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
       if ((pkt->node_id == wsens.id) && (pkt->antenna_id == wsens.radio[i].antenna_id)) {
	   struct wsnet_rx_info info;
	   info.data       = pkt->data;
	   info.freq_mhz   = pkt->freq / 1000000.0;
	   info.modulation = pkt->wsim_mod_id;
	   info.power_dbm  = ntohdbl(pkt->power_dbm);
	   info.SiNR       = pkt->sinr;
	   WSNET2_DBG("Libwsnet2:wsnet2_sr_rx: rxing at time %lld data 0x%02x on antenna %s with power %g dbm\n", MACHINE_TIME_GET_NANO(), pkt->data, wsens.radio[i].antenna, info.power_dbm);
	   wsens.radio[i].callback(wsens.radio[i].arg, &info);
       }
       else {
	   WSNET2_DBG("Libwsnet2:wsnet2_sr_rx: Node id (%d) or antenna id (%d) don't match with pkt node id (%d) or pkt antenna id (%d) \n", wsens.id, i, wsens.radio[i].antenna_id, pkt->node_id, pkt->antenna_id);
       }
       i++;
   }
   
   return 0;
}


/* ************************************************** */
/* ************************************************** */
/**
 * RX data, takes into consideration new RP and saves state.
 **/
static int wsnet2_sr_rx(char *msg) {
   struct _worldsens_s_byte_sr_rx *pkt  = (struct _worldsens_s_byte_sr_rx *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;

   /* rx data and eventually callback radio */
   int i = 0;
   if (pkt->node_id == wsens.id) {
       while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	   if (pkt->antenna_id == wsens.radio[i].antenna_id) {
	       struct wsnet_rx_info info;
	       info.data       = pkt->data;
	       info.freq_mhz   = pkt->freq / 1000000.0;
	       info.modulation = pkt->wsim_mod_id;
	       info.power_dbm  = ntohdbl(pkt->power_dbm);
	       info.SiNR       = pkt->sinr;
	       WSNET2_DBG("Libwsnet2:wsnet2_sr_rx: rxing at time %lld data 0x%02x on antenna %s with power %g dbm\n", MACHINE_TIME_GET_NANO(), pkt->data, wsens.radio[i].antenna, info.power_dbm);
	       wsens.radio[i].callback(wsens.radio[i].arg, &info);
	   }
	   else {
	       WSNET2_DBG("Libwsnet2:wsnet2_sr_rx: Node id (%d) or antenna id (%d) don't match with pkt node id (%d) or pkt antenna id (%d) \n", wsens.id, i, wsens.radio[i].antenna_id, pkt->node_id, pkt->antenna_id);
	   }
	   i++;
       }
   }
   
   /* update */
   wsens.rpseq = pkt->rp_next;
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + pkt->rp_duration;
   wsens.state = WORLDSENS_CLT_STATE_IDLE;
   WORLDSENS_SAVE_STATE();

   return 0;
}


/* ************************************************** */
/* ************************************************** */
/**
 * Handle measure response (callback device requesting this measure).
 **/
static int wsnet2_measure_rsp(char *msg) {
    struct _worldsens_s_measure_rsp *pkt  = (struct _worldsens_s_measure_rsp *) msg;
    int ret = wsnet2_seq(msg);

    if (ret == -1)
        return -1;
    else if (ret == -2)
        return 0;

    int i = 0;
    if (pkt->node_id == wsens.id) {
        while ((wsens.measure[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	    if (pkt->measure_id == wsens.measure[i].id) {
	        WSNET2_DBG("Libwsnet2:wsnet2_measure_rsp: Measure rsp at time %lld, measure '%s', measure value %g\n", MACHINE_TIME_GET_NANO(), wsens.measure[i].name, pkt->measure_val);
		wsens.measure[i].callback(wsens.measure[i].arg, pkt->measure_val);
	    }
	    i++;
	}
    }    
    return 0;
}

/* ************************************************** */
/* ************************************************** */
/**
 * Handle measure response (callback device requesting this measure) and rp.
 **/
static int wsnet2_measure_sr_rsp(char *msg) {
    struct _worldsens_s_measure_sr_rsp *pkt  = (struct _worldsens_s_measure_sr_rsp *) msg;
    int ret = wsnet2_seq(msg);

    if (ret == -1)
        return -1;
    else if (ret == -2)
        return 0;

    int i = 0;
    if (pkt->node_id == wsens.id) {
        while ((wsens.measure[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	    if (pkt->measure_id == wsens.measure[i].id) {
	      WSNET2_DBG("Libwsnet2:wsnet2_measure_sr_rsp: Measure rsp at time %lld, measure '%s', measure value %g\n", MACHINE_TIME_GET_NANO(), wsens.measure[i].name, pkt->measure_val);
		wsens.measure[i].callback(wsens.measure[i].arg, pkt->measure_val);
	    }
	    i++;
	}
    }   

    /* update */
    wsens.rpseq = pkt->rp_next;
    wsens.n_rp  = MACHINE_TIME_GET_NANO() + pkt->rp_duration;
    wsens.state = WORLDSENS_CLT_STATE_IDLE;
    WORLDSENS_SAVE_STATE();

    return 0;
}

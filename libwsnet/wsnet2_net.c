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

#define WORLDSENS_SAVE_STATE() {            \
    wsens.l_rp  = MACHINE_TIME_GET_NANO();  \
    wsens.state = WORLDSENS_CLT_STATE_IDLE; \
    machine_state_save();                   \
}   

/* ************************************************** */
/* ************************************************** */

#define WORLDSENS_UPDATE_PERIOD 1000

/* ************************************************** */
/* ************************************************** */

static int      wsnet2_sync           (void);
static int      wsnet2_parse          (char *);
static int      wsnet2_seq            (char *);
static int      wsnet2_published      (char *);
static int      wsnet2_backtrack      (char *);
static int      wsnet2_sync_release   (char *);
static int      wsnet2_sync_req       (char *);
static int      wsnet2_rx             (char *);
static int      wsnet2_rxreq          (char *);
static int      wsnet2_subscribe      (void);

/* ************************************************** */
/* ************************************************** */


struct _worldsens_clt wsens;


void wsnet2_init(void) {
    int i = MAX_CALLBACKS;

    wsens.u_fd     = -1;
    wsens.m_fd     = -1;
    wsens.dseq     = 0;
    wsens.n_update = 0;


    while (i--) {
        wsens.radio[i].callback = NULL;
        wsens.measure[i].callback   = NULL;
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

void wsnet2_register_radio(char *antenna, radio_callback_t callback, void *arg) {
    int i = 0;

    while ((wsens.radio[i].callback != NULL) && (++i < MAX_CALLBACKS)) ;

    if (i == MAX_CALLBACKS) {
        ERROR("Libwsnet2:wsnet2_register_radio: too many registered radio callbacks\n");
        return;
    }
 
    wsens.radio[i].callback = callback;
    wsens.radio[i].arg      = arg;
    wsens.radio[i].antenna = malloc(strlen(antenna));
    strcpy(wsens.radio[i].antenna, antenna);

    WSNET2_DBG("Libwsnet2:wsnet2_register_radio: radio with antenna '%s' register in position %d\n", wsens.radio[i].antenna, i); 

    return ;
}


void wsnet2_register_measure(char *name, measure_callback_t callback, void *arg) {
    int i = 0;

    while ((wsens.measure[i].callback != NULL) && (++i < MAX_CALLBACKS)) ;

    if (i == MAX_CALLBACKS)  {
        ERROR("Libwsnet2:wsnet2_register_measure: too many registered measure callbacks\n");
        return;
    }
        
    wsens.measure[i].callback = callback;
    wsens.measure[i].arg      = arg;
    strcpy(wsens.measure[i].name, name);
    return ;
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
        libselect_fd_register(wsens.m_fd, SIG_WORLDSENS_IO);
  
    return ret;

 error:
    wsnet2_finalize();
    ERROR("Libwsnet2:wsnet2_connect: Error during connection initialization...\n");
    return -1;
}


int wsnet2_subscribe(void) {
    union _worldsens_pkt pkt;
    char msg[MAX_PKTLENGTH];
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
    WSNET2_DBG("Libwsnet2:wsnet2_subscribe: attempting to connect with id %d...\n", wsens.id);

    /* wait for server response */
    wsens.state = WORLDSENS_CLT_STATE_CONNECTING;
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
        /* receive */
      WSNET2_DBG("Libwsnet2:wsnet2_subscribe: waiting for server response... \n");
        if ((len = recv(wsens.u_fd, msg, MAX_PKTLENGTH, 0)) < 0) {
            perror("(recv)");
            goto error;
        }
        if (wsnet2_parse(msg))
	  return -1;
    }

    WSNET2_DBG("Libwsnet2:wsnet2_subscribe: connection to server successfull \n");

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
    WSNET2_DBG("Libwsnet2:wsnet2_unsubscribe: disconnected id %d\n", wsens.id);
	
    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_unsubscribe: Error when sending unsubscribe request\n");
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int wsnet2_update(void) {
    char msg[MAX_PKTLENGTH];
    int len, ret;
    fd_set readfds;
    struct timeval timeout;
	
    /* synched */
    if (MACHINE_TIME_GET_NANO() >= wsens.n_rp) {
        wsnet2_sync();
    }
	
    /* time to update */
    if (MACHINE_TIME_GET_NANO() < wsens.n_update) {
        return 0;
    } else {
        wsens.n_update = MACHINE_TIME_GET_NANO() + WORLDSENS_UPDATE_PERIOD;
    }

    /* handle received packets */
    if (mcu_signal_get() & SIG_WORLDSENS_IO) {
        mcu_signal_remove(SIG_WORLDSENS_IO);
        
        do {
            /* receive */
            if ((len = recv(wsens.m_fd, msg, MAX_PKTLENGTH, 0)) <= 0) {
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
	
    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_update: Error when updating liwsnet2");
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

static int wsnet2_sync(void) {
    char msg[MAX_PKTLENGTH];
    union _worldsens_pkt pkt;
    int len;
	
    /* format */
    pkt.sync_ack.type        = WORLDSENS_C_SYNC_ACK;
    pkt.sync_ack.node_id     = wsens.id;
    pkt.sync_ack.rp_id       = wsens.rpseq;

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_sync_ack), 0)  < 0) {
        perror("(send)");
	worldsens_packet_dump(&pkt);
        goto error;
    }
    WSNET2_DBG("Libwsnet2:wsnet2_sync: synched on rp %d\n", wsens.rpseq);

	
    /* wait for new rp */
    wsens.state = WORLDSENS_CLT_STATE_PENDING;
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recvfrom)");
            goto error;
        }
		
        /* parse */
        if (wsnet2_parse(msg))
            return -1;      
		
    }
	
    return 0;

 error:
    ERROR("Libwsnet2:wsnet2_sync: Error during synchronization");
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int wsnet2_tx(char data, double freq, int mod, double txdB, uint64_t delay) {
    char msg[MAX_PKTLENGTH];
    union _worldsens_pkt pkt;
    int len;
	
    /* format */
    pkt.byte_tx.type              = WORLDSENS_C_BYTE_TX;
    pkt.byte_tx.node_id           = wsens.id;
    pkt.byte_tx.period            = MACHINE_TIME_GET_NANO() - wsens.l_rp;
    pkt.byte_tx.data              = data;
    pkt.byte_tx.freq              = freq;
    pkt.byte_tx.modulation_id     = mod;
    pkt.byte_tx.power             = txdB;
    //    pkt.byte_tx.dseq              = wsens.dseq++;
    pkt.byte_tx.duration          = delay;
    worldsens_packet_hton(&pkt);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_byte_tx), 0)  < 0) {
        perror("(send)");
	worldsens_packet_dump(&pkt);
        goto error;
    }
    WSNET2_TX("Libwsnet2:wsnet2_tx: packet 0x%02x sent\n",data);
    
    /* wait either for backtrack or for new rp */
    wsens.state = WORLDSENS_CLT_STATE_TXING;
    while (((MACHINE_TIME_GET_NANO() + delay) < wsens.n_rp) && (wsens.state != WORLDSENS_CLT_STATE_IDLE)) {    
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, MAX_PKTLENGTH, 0)) <= 0) {
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

static int wsnet2_parse(char *msg) {
  struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg; 

  switch (header->type) {
 
  case WORLDSENS_S_CONNECT_RSP_OK:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_CONNECT_RSP_OK packet type\n");
      if(wsnet2_published(msg)){
	  WSNET2_DBG("Libwsnet2:wsnet2_parse: error during publishing models\n");	
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
      if (wsnet2_backtrack(msg))
          goto error;
      break;
  case WORLDSENS_S_SYNC_REQ:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_SYNC_REQ packet type\n");
      if (wsnet2_sync_req(msg))
          goto error;
      break;
  case WORLDSENS_S_SYNC_RELEASE:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_SYNC_RELEASE packet type\n");
      if (wsnet2_sync_release(msg))
          goto error;
      break;
  case WORLDSENS_S_BYTE_RX:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_BYTE_TX packet type\n");
      if (wsnet2_rx(msg))
          goto error;
      break;
  case WORLDSENS_S_MEASURE_RSP:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_MEASURE_RSP packet type\n");
      //TODO
      break;
  case WORLDSENS_S_KILLSIM:
      WSNET2_DBG("Libwsnet2:wsnet2_parse: WORLDSENS_S_KILLSIM packet type\n");
      wsnet2_finalize();
      break;
      //  case WSENS_S_RXREQ:
      //if (wsnet2_rxreq(msg))
      //    goto error;
      //break;
  default:
   ERROR("Libwsnet2:wsnet2_parse: unknown packet type!");
   goto error;
  }

  return 0;

 error:
    ERROR("Libwsnet2:wsnet2_parse: error during packet parse");
    wsnet2_finalize();
    return -1;	
}


static int wsnet2_seq(char *msg) {
   struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg;

   if (ntohl(header->seq) > wsens.seq) {
       ERROR("Libwsnet2:wsnet2_seq: lost wsens packet (received: %d while expecting %d)\n", ntohl(header->seq), wsens.seq);
       return -1;
   }  else if (ntohl(header->seq) < wsens.seq) {
       ERROR("Libwsnet2:wsnet2_seq: deprecated wsens packet (received: %d while expecting %d)\n", ntohl(header->seq), wsens.seq);
       return -2;
   }
   
   wsens.seq++;
   return 0;
}


static int wsnet2_published(char *msg) {
   union _worldsens_pkt *pkt = (union _worldsens_pkt *) msg;
   
   int offset = 0;
   int match_antenna = 0;
   uint32_t counter  = 0;
   int i = 0, j = 0;

   worldsens_packet_ntoh(pkt);

   wsens.n_rp  = MACHINE_TIME_GET_NANO() + pkt->cnx_rsp_ok.rp_duration;
   WORLDSENS_SAVE_STATE();

   WSNET2_DBG("Libwsnet2:wsnet2_published: nb models = %d\n", pkt->cnx_rsp_ok.nb_models);

   while (counter < pkt->cnx_rsp_ok.nb_models) {
       i = 0;
       j = 0;
 
       WSNET2_DBG("Libwsnet2:wsnet2_published: checking '%s' -> %d\n", pkt->cnx_rsp_ok.names + offset + sizeof(uint32_t), *((uint32_t *)(pkt->cnx_rsp_ok.names + offset)));
       
       while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	 if (strcmp(wsens.radio[i].antenna, pkt->cnx_rsp_ok.names + offset + sizeof(uint32_t)) == 0) {
	   wsens.radio[i].id = *((uint32_t *)(pkt->cnx_rsp_ok.names + offset));
	   WSNET2_DBG("Libwsnet2:wsnet2_published: antenna '%s' matches\n", wsens.radio[i].antenna);
	   match_antenna++;
           }
           i++;
       }
/*        while ((wsens.measure[j].callback != NULL) && (j < MAX_CALLBACKS)) { */
/* 	 if (strcmp(wsens.measure[i].name,  pkt->cnx_rsp_ok.names + offset + sizeof(uint32_t)) == 0) { */
/* 	   wsens.measure[i].id = *((uint32_t *)(pkt->cnx_rsp_ok.names + offset));              */
/*                WSNET2_DBG("Libwsnet2:wsnet2_published:              matched\n"); */
/*            } */
/*            j++; */
/*        } */
       offset += strlen(pkt->cnx_rsp_ok.names + offset) + sizeof(uint32_t);
       counter++;
   }

   /* check if we have at least as many antennas as radios register */
   if(match_antenna >= i){
     return 0;
   }
   else{
     ERROR("Libwsnet2:wsnet2_published: not enough antennas registered\n");
     return -1;
   }
}


static int wsnet2_backtrack(char *msg) {
   struct _worldsens_s_backtrack *pkt = (struct _worldsens_s_backtrack *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (MACHINE_TIME_GET_NANO() > (wsens.l_rp + ntohll(pkt->rp_duration))) {
       WSNET2_BKTRK("Libwsnet2:wsnet2_backtrack: backtracking\n");
       machine_state_restore();   
   } else {
       WSNET2_BKTRK("Libwsnet2:wsnet2_backtrack: no need to backtrack\n");
   }

   wsens.rpseq = ntohl(pkt->rp_next);
   wsens.n_rp  = wsens.l_rp + ntohll(pkt->rp_duration);
   wsens.state = WORLDSENS_CLT_STATE_IDLE;

   return 0;
}


static int wsnet2_sync_release(char *msg) {
   struct _worldsens_s_sync_release *pkt = (struct _worldsens_s_sync_release *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WORLDSENS_CLT_STATE_PENDING) {
       ERROR("Libwsnet2:wsnet2_sync_release: received a release order while not synched (state: %d)\n", wsens.state);
       return -1;
   }

   if (wsens.rpseq != ntohl(pkt->rp_current)) {
       ERROR("Libwsnet2:wsnet2_sync_release: received release order with bad rp sequence (expected: %d, received: %d)\n", wsens.rpseq, ntohl(pkt->rp_current));
       return -1;
   }
   WSNET2_DBG("Libwsnet2:wsnet2_sync_release: released\n");

   wsens.rpseq = ntohl(pkt->rp_next);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->rp_duration);
   WORLDSENS_SAVE_STATE();
   
   return 0;
}


static int wsnet2_rx(char *msg) {
   struct _worldsens_s_byte_rx *pkt  = (struct _worldsens_s_byte_rx *) msg;
   //   struct _worldsens_data *data = (struct _worldsens_data *) (msg + sizeof(struct _worldsens_s_byte_rx));
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   //   wsnet2_msg_dump(pkt);

   //   while (((char *) data) < (msg + ntohl(pkt->length))) {
   //    if (ntohl(data->id) == wsens.id) {
           int i = 0;

           while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	     if ((ntohl(pkt->nodes_infos[i].node_id) == wsens.id) && 
		 (ntohl(pkt->nodes_infos[i].antenna_id) == wsens.radio[i].id)) {
		 struct wsnet_rx_info info;
		 info.data       = pkt->data;
		 info.freq_mhz   = ntohdbl(pkt->freq);
		 info.modulation = ntohl(pkt->modulation_id);
		 info.power_dbm  = ntohdbl(pkt->nodes_infos[i].power);
                 //info.SiNR       = ntohdbl(data->SINR);
		 WSNET2_DBG("Libwsnet2:wsnet2_rx: rxing on antenna %s\n", wsens.radio[i].antenna);
		 wsens.radio[i].callback(wsens.radio[i].arg, &info);
               }
               i++;
           }
	   //     }
       
// data += 1;
       //  }
   
   return 0;
}


/*int wsnet2_rxreq(char *msg) {
   struct _wsens_s_rxreq *pkt = (struct _wsens_s_rxreq *) msg;
   struct _wsens_data *data   = (struct _wsens_data *) (msg + sizeof(struct _wsens_s_rx));
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WORLDSENS_CLT_STATE_PENDING) {
       ERROR("Worldsens: received a release order while not synched (state: %d)\n", wsens.state);
       return -1;
   }

   if (wsens.rpseq != ntohl(pkt->rpseq)) {
       ERROR("Worldsens: received release order with bad rp sequence (expected: %d, received: %d)\n", wsens.rpseq, ntohl(pkt->rpseq));
       return -1;
   }
   
   DMSG("Worldsens: released\n");
   while (((char *) data) < (msg + ntohl(pkt->length))) {
       if (ntohl(data->id) == wsens.id) {
           int i = 0;

           while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
               if (ntohl(data->antenna) == wsens.radio[i].id) {
                   struct _radio_info info= {
                       ntohdbl(pkt->freq),
                       ntohl(pkt->mod),
                       ntohdbl(data->rxdB),
                       ntohdbl(data->SINR)
                   };
                   DMSG("Worldsens: rxing on antenna %s\n", wsens.radio[i].antenna);
                   wsens.radio[i].callback(wsens.radio[i].arg, data->data, &info);
               }
               i++;
           }
       }
       
       data += 1;
   }
   
   wsens.rpseq = ntohl(pkt->n_rpseq);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);
   WORLDSENS_SAVE_STATE();
   return 0;
   }*/


static int wsnet2_sync_req(char *msg)
{
  return 0;
}

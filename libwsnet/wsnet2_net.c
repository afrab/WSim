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

struct _worldsens_clt wsens;


void wsnet2_init(void) {
    int i = MAX_CALLBACKS;

    wsens.u_fd     = -1;
    wsens.m_fd     = -1;
    wsens.dseq     = 0;
    wsens.n_update = 0;


    while (i--) {
        wsens.radio[i].callback = NULL;
        wsens.phy[i].callback   = NULL;
    }
}


void wsnet2_finalize(void) {

    if (wsens.u_fd > 0)
        close(wsens.u_fd);
    if (wsens.m_fd > 0)
        close(wsens.m_fd);

    wsens.u_fd = -1;
    wsens.m_fd = -1;
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
        ERROR("Worldsens: too many registered radio callbacks\n");
        return;
    }
        
    wsens.radio[i].callback = callback;
    wsens.radio[i].arg      = arg;
    strcpy(wsens.radio[i].antenna, antenna);
    return ;
}


void wsnet2_register_phy(char *channel, phy_callback_t callback, void *arg) {
    int i = 0;

    while ((wsens.phy[i].callback != NULL) && (++i < MAX_CALLBACKS)) ;

    if (i == MAX_CALLBACKS)  {
        ERROR("Worldsens: too many registered physical callbacks\n");
        return;
    }
        
    wsens.phy[i].callback = callback;
    wsens.phy[i].arg      = arg;
    strcpy(wsens.phy[i].channel, channel);
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
        goto error;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
	
    /* allow several bindings */
    if (setsockopt(wsens.m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0 ) {
        perror("(setsockopt):");
        goto error;
    }
	
#if !defined(LINUX)
    /* allow several bindings */
    if (setsockopt(wsens.m_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) != 0 ) { 
        perror("(setsockopt):"); 
        goto error;
    } 
#endif
	
    /* bind */
    if (bind(wsens.m_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(bind):");
        goto error;
    }
	
    /* join */
    if (inet_aton(m_addr, &mreq.imr_multiaddr) == 0) {
        perror("(inet_aton):");
        goto error;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(wsens.m_fd, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 ) {
        perror("(setsockopt):");
        goto error;
    }
	
    /* unicast socket */
    if ((wsens.u_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("(socket):");
        goto error;
    }
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(0);
    addr.sin_addr.s_addr = INADDR_ANY;
	
    /* bind */
    if (bind(wsens.u_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(bind):");
        goto error;
    }
	
    /* connect */	
    addr.sin_port = htons(s_port);
    if (inet_aton(s_addr, &addr.sin_addr) == 0) {
        perror("(inet_aton):");
        goto error;
    }
    if (connect(wsens.u_fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
        perror("(connect):");
        goto error;
    }
	
    /* subscribe to server */
    ret = wsnet2_subscribe();
    if (!ret)
        libselect_fd_register(wsens.m_fd, SIG_WORLDSENS_IO);
  
    return ret;

 error:
    wsnet2_finalize();
    return -1;
}


int wsnet2_subscribe(void) {
    struct _worldsens_c_header header;
    char msg[MAX_PKTLENGTH];
    int len;
    
    /* format */
    header.type = htonl(WORLDSENS_C_CONNECT_REQ);
    header.id   = htonl(wsens.id);
	
    /* send */
    if (send(wsens.u_fd, (char *) (&header), sizeof(struct _worldsens_c_header), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: connecting with id %d...\n", wsens.id);
	
    wsens.state = WORLDSENS_CLT_STATE_CONNECTING;
    while (wsens.state != WORLDSENS_CLT_STATE_IDLE) {
        /* receive */
        if ((len = recv(wsens.u_fd, msg, MAX_PKTLENGTH, 0)) < 0) {
            perror("(recv)");
            goto error;
        }

        if (wsnet2_parse(msg))
            return -1;
    }
    
 error:
    wsnet2_finalize();
    return -1;
}


int wsnet2_unsubscribe(void) {
    struct _worldsens_c_header header;
    
    /* format */
    header.type = htonl(WORLDSENS_C_DISCONNECT);
    header.id   = htonl(wsens.id);
	
    /* send */
    if (send(wsens.u_fd, (char *) (&header), sizeof(struct _worldsens_c_header), 0) < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: disconnected id %d\n", wsens.id);
	
    return 0;

 error:
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
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int wsnet2_sync(void) {
    char msg[MAX_PKTLENGTH];
    struct _worldsens_c_sync_ack pkt;
    int len;
	
    /* format */
    pkt.type        = htonl(WORLDSENS_C_SYNC_ACK);
    pkt.node_id     = htonl(wsens.id);
    pkt.rp_id       = htonl(wsens.rpseq);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_sync_ack), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: synched on rp %d\n", wsens.rpseq);

	
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
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int wsnet2_tx(char data, double freq, int mod, double txdB, uint64_t delay) {
    char msg[MAX_PKTLENGTH];
    struct _worldsens_c_byte_tx pkt;
    int len;
	
    /* format */
    pkt.type              = htonl(WORLDSENS_C_BYTE_TX);
    pkt.node_id           = htonl(wsens.id);
    pkt.period            = htonll(MACHINE_TIME_GET_NANO() - wsens.l_rp);
    pkt.data              = data;
    pkt.freq              = htondbl(freq);
    pkt.modulation_id     = htonl(mod);
    pkt.power             = htondbl(txdB);
    //    pkt.dseq              = htonl(wsens.dseq++);
    pkt.duration          = htonll(delay);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _worldsens_c_byte_tx), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: txing\n");
    
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
    wsnet2_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int wsnet2_parse(char *msg) {
  struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg;
	
  switch (ntohl(header->type)) {

  case WORLDSENS_S_CONNECT_RSP_OK:
      wsnet2_published(msg);
      break;
  case WORLDSENS_S_CONNECT_RSP_NOK:
      ERROR("Connection refused by wsnet server\n");
      wsnet2_finalize();
      return -1;
  case WORLDSENS_S_BACKTRACK:
      if (wsnet2_backtrack(msg))
          goto error;
      break;
  case WORLDSENS_S_SYNC_REQ:
      if (wsnet2_sync_req(msg))
          goto error;
      break;
  case WORLDSENS_S_SYNC_RELEASE:
      if (wsnet2_sync_release(msg))
          goto error;
      break;
  case WORLDSENS_S_BYTE_RX:
      if (wsnet2_rx(msg))
          goto error;
      break;
  case WORLDSENS_S_MEASURE_RSP:
      //TODO
      break;
  case WORLDSENS_S_KILLSIM:
      wsnet2_finalize();
      break;
      //  case WSENS_S_RXREQ:
      //if (wsnet2_rxreq(msg))
      //    goto error;
      //break;
  default:
      return 0;
  }

  return 0;

 error:
    wsnet2_finalize();
    return -1;	
}


int wsnet2_seq(char *msg) {
   struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg;

   if (ntohl(header->seq) > wsens.seq) {
       ERROR("Worldsens: lost wsens packet (received: %d while expecting %d)\n", ntohl(header->seq), wsens.seq);
       return -1;
   }  else if (ntohl(header->seq) < wsens.seq) {
       ERROR("Worldsens: deprecated wsens packet (received: %d while expecting %d)\n", ntohl(header->seq), wsens.seq);
       return -2;
   }
   
   wsens.seq++;
   return 0;
}


void wsnet2_published(char *msg) {
   struct _worldsens_s_connect_rsp *pkt = (struct _worldsens_s_connect_rsp *) msg;
   int offset = 0;

   wsens.seq   = ntohl(pkt->seq);
   wsens.rpseq = ntohl(pkt->rp_current);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->rp_duration);
   WORLDSENS_SAVE_STATE();
   DMSG("Worldsens: connected\n");

   while (offset < sizeof(struct _worldsens_s_connect_rsp)) {
       int i = 0, j = 0;
        
       DMSG("          %s -> %d\n", pkt->names, ntohl(*((uint32_t *)(pkt->names + offset))));

       while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
	 if (strcmp(wsens.radio[i].antenna, pkt->names + offset + sizeof(uint32_t)) == 0) {
	   wsens.radio[i].id = ntohl(*((uint32_t *)(pkt->names + offset)));
               DMSG("              matched\n");
              
           }
           i++;
       }
       while ((wsens.phy[j].callback != NULL) && (j < MAX_CALLBACKS)) {
	 if (strcmp(wsens.phy[i].channel,  pkt->names + offset + sizeof(uint32_t)) == 0) {
	   wsens.phy[i].id = ntohl(*((uint32_t *)(pkt->names + offset)));             
               DMSG("              matched\n");
           }
           j++;
       }
       offset += strlen(pkt->names + offset) + sizeof(uint32_t);
   }
}


int wsnet2_backtrack(char *msg) {
   struct _worldsens_s_backtrack *pkt = (struct _worldsens_s_backtrack *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (MACHINE_TIME_GET_NANO() > (wsens.l_rp + ntohll(pkt->rp_duration))) {
       DMSG("Worldsens: backtracking\n");
       machine_state_restore();   
   } else {
       DMSG("Worldsens: no need to backtrack\n");
   }

   wsens.rpseq = ntohl(pkt->rp_next);
   wsens.n_rp  = wsens.l_rp + ntohll(pkt->rp_duration);
   wsens.state = WORLDSENS_CLT_STATE_IDLE;

   return 0;
}


int wsnet2_sync_release(char *msg) {
   struct _worldsens_s_sync_release *pkt = (struct _worldsens_s_sync_release *) msg;
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WORLDSENS_CLT_STATE_PENDING) {
       ERROR("Worldsens: received a release order while not synched (state: %d)\n", wsens.state);
       return -1;
   }

   if (wsens.rpseq != ntohl(pkt->rp_current)) {
       ERROR("Worldsens: received release order with bad rp sequence (expected: %d, received: %d)\n", wsens.rpseq, ntohl(pkt->rp_current));
       return -1;
   }
   DMSG("Worldsens: released\n");

   wsens.rpseq = ntohl(pkt->rp_next);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->rp_duration);
   WORLDSENS_SAVE_STATE();
   
   return 0;
}


int wsnet2_rx(char *msg) {
   struct _worldsens_s_byte_rx *pkt  = (struct _worldsens_s_byte_rx *) msg;
   //   struct _worldsens_data *data = (struct _worldsens_data *) (msg + sizeof(struct _worldsens_s_byte_rx));
   int ret = wsnet2_seq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   //   while (((char *) data) < (msg + ntohl(pkt->length))) {
   //    if (ntohl(data->id) == wsens.id) {
           int i = 0;

           while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
               if (ntohl(pkt->antenna_id) == wsens.radio[i].id) {
		 struct wsnet_rx_info info;
		 info.data       = pkt->data;
		 info.freq_mhz   = ntohdbl(pkt->freq);
		 info.modulation = ntohl(pkt->modulation_id);
		 info.power_dbm  = ntohdbl(pkt->power);
                 //info.SiNR       = ntohdbl(data->SINR);
		 DMSG("Worldsens: rxing on antenna %s\n", wsens.radio[i].antenna);
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


int wsnet2_sync_req(char *msg){
  return 0;
}

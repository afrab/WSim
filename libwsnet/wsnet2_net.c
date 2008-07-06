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

#include "worldsens_pkt.h"
#include "libworldsens.h"

/* ************************************************** */
/* ************************************************** */

#define WSENS_SAVE_STATE() {                \
    wsens.l_rp  = MACHINE_TIME_GET_NANO();  \
    wsens.state = WSENS_CLT_STATE_IDLE;     \
    machine_state_save();                   \
}   

/* ************************************************** */
/* ************************************************** */

#define WSENS_UPDATE_PERIOD 1000

/* ************************************************** */
/* ************************************************** */

struct _wsens_clt wsens;


void libwsens_clt_init(void) {
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


void libwsens_clt_finalize(void) {

    if (wsens.u_fd > 0)
        close(wsens.u_fd);
    if (wsens.m_fd > 0)
        close(wsens.m_fd);

    wsens.u_fd = -1;
    wsens.m_fd = -1;
}

/* ************************************************** */
/* ************************************************** */

void libwsens_clt_register_radio(char *antenna, radio_callback_t callback, void *arg) {
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


void libwsens_clt_register_phy(char *channel, phy_callback_t callback, void *arg) {
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

int libwsens_clt_connect(char *s_addr, uint16_t s_port, char *m_addr, uint16_t m_port, uint32_t id) {
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
    ret = libwsens_clt_subscribe();
    if (!ret)
        libselect_register_signal(wsens.m_fd, SIG_WORLDSENS_IO);
  
    return ret;

 error:
    libwsens_clt_finalize();
    return -1;
}


int libwsens_clt_subscribe(void) {
    struct _wsens_c_header header;
    char msg[MAX_PKTLENGTH];
    int len;
    
    /* format */
    header.type = htonl(WSENS_C_SUBSCRIBE);
    header.id   = htonl(wsens.id);
	
    /* send */
    if (send(wsens.u_fd, (char *) (&header), sizeof(struct _wsens_c_header), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: connecting with id %d...\n", wsens.id);
	
    wsens.state = WSENS_CLT_STATE_CONNECTING;
    while (wsens.state != WSENS_CLT_STATE_IDLE) {
        /* receive */
        if ((len = recv(wsens.u_fd, msg, MAX_PKTLENGTH, 0)) < 0) {
            perror("(recv)");
            goto error;
        }

        if (libwsens_clt_parse(msg))
            return -1;
    }
    
 error:
    libwsens_clt_finalize();
    return -1;
}


int libwsens_clt_unsubscribe(void) {
    struct _wsens_c_header header;
    
    /* format */
    header.type = htonl(WSENS_C_UNSUBSCRIBE);
    header.id   = htonl(wsens.id);
	
    /* send */
    if (send(wsens.u_fd, (char *) (&header), sizeof(struct _wsens_c_header), 0) < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: disconnected id %d\n", wsens.id);
	
    return 0;

 error:
    libwsens_clt_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int libwsens_clt_update(void) {
    char msg[MAX_PKTLENGTH];
    int len, ret;
    fd_set readfds;
    struct timeval timeout;
	
    /* synched */
    if (MACHINE_TIME_GET_NANO() >= wsens.n_rp) {
        libwsens_clt_sync();
    }
	
    /* time to update */
    if (MACHINE_TIME_GET_NANO() < wsens.n_update) {
        return 0;
    } else {
        wsens.n_update = MACHINE_TIME_GET_NANO() + WSENS_UPDATE_PERIOD;
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
            if (libwsens_clt_parse(msg))
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
    libwsens_clt_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int libwsens_clt_sync(void) {
    char msg[MAX_PKTLENGTH];
    struct _wsens_c_sync pkt;
    int len;
	
    /* format */
    pkt.header.type = htonl(WSENS_C_SYNC);
    pkt.header.id   = htonl(wsens.id);
    pkt.rpseq       = htonl(wsens.rpseq);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _wsens_c_sync), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: synched on rp %d\n", wsens.rpseq);

	
    /* wait for new rp */
    wsens.state = WSENS_CLT_STATE_PENDING;
    while (wsens.state != WSENS_CLT_STATE_IDLE) {
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recvfrom)");
            goto error;
        }
		
        /* parse */
        if (libwsens_clt_parse(msg))
            return -1;      
		
    }
	
    return 0;

 error:
    libwsens_clt_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int libwsens_clt_tx(char data, double freq, int mod, double txdB, uint64_t delay) {
    char msg[MAX_PKTLENGTH];
    struct _wsens_c_tx pkt;
    int len;
	
    /* format */
    pkt.header.type = htonl(WSENS_C_TX);
    pkt.header.id   = htonl(wsens.id);
    pkt.period      = htonll(MACHINE_TIME_GET_NANO() - wsens.l_rp);
    pkt.data        = data;
    pkt.freq        = htondbl(freq);
    pkt.mod         = htonl(mod);
    pkt.txdB        = htondbl(txdB);
    pkt.dseq        = htonl(wsens.dseq++);
    pkt.delay       = htonll(delay);

    /* send */
    if (send(wsens.u_fd, (char *) (&pkt), sizeof(struct _wsens_c_tx), 0)  < 0) {
        perror("(send)");
        goto error;
    }
    DMSG("Worldsens: txing\n");
    
    /* wait either for backtrack or for new rp */
    wsens.state = WSENS_CLT_STATE_TXING;
    while (((MACHINE_TIME_GET_NANO() + delay) < wsens.n_rp) && (wsens.state != WSENS_CLT_STATE_IDLE)) {    
		
        /* receive */
        if ((len = recv(wsens.m_fd, msg, MAX_PKTLENGTH, 0)) <= 0) {
            perror("(recv)");
            goto error;
        }
		
        /* parse */
        if (libwsens_clt_parse(msg))
            return -1;      
    }
	
    return 0;

 error:
    libwsens_clt_finalize();
    return -1;
}

/* ************************************************** */
/* ************************************************** */

int libwsens_clt_parse(char *msg) {
  struct _wsens_s_header *header = (struct _wsens_s_header *) msg;
	
  switch (ntohl(header->type)) {

  case WSENS_S_PUBLISH:
      libwsens_clt_published(msg);
      break;
  case WSENS_S_UNPUBLISH:
      ERROR("Connection refused by wsnet server\n");
      libwsens_clt_finalize();
      return -1;
  case WSENS_S_BACKTRACK:
      if (libwsens_clt_backtrack(msg))
          goto error;
      break;
  case WSENS_S_RELEASE:
      if (libwsens_clt_release(msg))
          goto error;
      break;
  case WSENS_S_RX:
      if (libwsens_clt_rx(msg))
          goto error;
      break;
  case WSENS_S_RXREQ:
      if (libwsens_clt_rxreq(msg))
          goto error;
      break;
  default:
      return 0;
  }

  return 0;

 error:
    libwsens_clt_finalize();
    return -1;	
}


int libwsens_clt_sseq(char *msg) {
   struct _wsens_s_header *header = (struct _wsens_s_header *) msg;

   if (ntohl(header->sseq) > wsens.sseq) {
       ERROR("Worldsens: lost wsens packet (received: %d while expecting %d)\n", ntohl(header->sseq), wsens.sseq);
       return -1;
   }  else if (ntohl(header->sseq) < wsens.sseq) {
       ERROR("Worldsens: deprecated wsens packet (received: %d while expecting %d)\n", ntohl(header->sseq), wsens.sseq);
       return -2;
   }
   
   wsens.sseq++;
   return 0;
}


void libwsens_clt_published(char *msg) {
   struct _wsens_s_pub *pkt = (struct _wsens_s_pub *) msg;
   struct _wsens_map   *map = (struct _wsens_map *) (pkt + 1);

   wsens.sseq  = ntohl(pkt->header.sseq);
   wsens.rpseq = ntohl(pkt->n_rpseq);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);
   WSENS_SAVE_STATE();
   DMSG("Worldsens: connected\n");


   while (((char *) map) < (msg + ntohl(pkt->length))) {
       int i = 0, j = 0;
       
       DMSG("          %s -> %d\n", map.name, ntohl(map->id));

       while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) {
           if (strcmp(wsens.radio[i].antenna, map->name) == 0) {
               wsens.radio[i].id = ntohl(map->id);
               DMSG("              macthed\n");
              
           }
           i++;
       }
       while ((wsens.phy[j].callback != NULL) && (j < MAX_CALLBACKS)) {
           if (strcmp(wsens.phy[i].channel, map->name) == 0) {
               wsens.phy[i].id = ntohl(map->id);               
               DMSG("              matched\n");
           }
           j++;
       }
       
       map += 1;
   }
}


int libwsens_clt_backtrack(char *msg) {
   struct _wsens_s_back *pkt = (struct _wsens_s_back *) msg;
   int ret = libwsens_clt_sseq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (MACHINE_TIME_GET_NANO() > (wsens.l_rp + ntohll(pkt->period))) {
       DMSG("Worldsens: backtracking\n");
       machine_state_restore();   
   } else {
       DMSG("Worldsens: no need to backtrack\n");
   }

   wsens.rpseq = ntohl(pkt->n_rpseq);
   wsens.n_rp  = wsens.l_rp + ntohll(pkt->period);
   wsens.state = WSENS_CLT_STATE_IDLE;

   return 0;
}


int libwsens_clt_release(char *msg) {
   struct _wsens_s_rel *pkt = (struct _wsens_s_rel *) msg;
   int ret = libwsens_clt_sseq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WSENS_CLT_STATE_PENDING) {
       ERROR("Worldsens: received a release order while not synched (state: %d)\n", wsens.state);
       return -1;
   }

   if (wsens.rpseq != ntohl(pkt->rpseq)) {
       ERROR("Worldsens: received release order with bad rp sequence (expected: %d, received: %d)\n", wsens.rpseq, ntohl(pkt->rpseq));
       return -1;
   }
   DMSG("Worldsens: released\n");

   wsens.rpseq = ntohl(pkt->n_rpseq);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);
   WSENS_SAVE_STATE();
   
   return 0;
}


int libwsens_clt_rx(char *msg) {
   struct _wsens_s_rx *pkt  = (struct _wsens_s_rx *) msg;
   struct _wsens_data *data = (struct _wsens_data *) (msg + sizeof(struct _wsens_s_rx));
   int ret = libwsens_clt_sseq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
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
                   DMSG("Worldsens: rxing on antenna %s\n", wsens.radio[i].name);
                   wsens.radio[i].callback(wsens.radio[i].arg, data->data, &info);
               }
               i++;
           }
       }
       
       data += 1;
   }
   
   return 0;
}


int libwsens_clt_rxreq(char *msg) {
   struct _wsens_s_rxreq *pkt = (struct _wsens_s_rxreq *) msg;
   struct _wsens_data *data   = (struct _wsens_data *) (msg + sizeof(struct _wsens_s_rx));
   int ret = libwsens_clt_sseq(msg);

   if (ret == -1)
       return -1;
   else if (ret == -2)
       return 0;
   
   if (wsens.state != WSENS_CLT_STATE_PENDING) {
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
                   DMSG("Worldsens: rxing on antenna %s\n", wsens.radio[i].name);
                   wsens.radio[i].callback(wsens.radio[i].arg, data->data, &info);
               }
               i++;
           }
       }
       
       data += 1;
   }
   
   wsens.rpseq = ntohl(pkt->n_rpseq);
   wsens.n_rp  = MACHINE_TIME_GET_NANO() + ntohll(pkt->period);
   WSENS_SAVE_STATE();
   return 0;
}

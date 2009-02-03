/*
 *  worldsens_srv.c
 *  
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "worldsens.h"

#include <private/simulation_private.h>
#include <private/packets_private.h>
#include <private/nodes_private.h>
#include <private/mobility_private.h>
#include <private/core_private.h>

#include <public/types.h>
#include <public/packets.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
uint16_t g_lport = 9998;
uint16_t g_mport = 9999;
char *	 g_maddr = "224.0.0.1";
char *	 g_saddr = "localhost";

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#if (BIG_ENDIAN == BYTE_ORDER)
static uint64_t ntohll(uint64_t v)
{
  return v;
}
static uint64_t htonll(uint64_t v)
{
  return v;
}
#else
static uint64_t ntohll(uint64_t v)
{
  uint64_t r;
  uint8_t *pv, *pr;
	
  pv = (uint8_t *) &v;
  pr = (uint8_t *) &r;
	
  pr[0] = pv[7];
  pr[1] = pv[6];
  pr[2] = pv[5];
  pr[3] = pv[4];
  pr[4] = pv[3];
  pr[5] = pv[2];
  pr[6] = pv[1];
  pr[7] = pv[0];
  return r;
}

static uint64_t htonll(uint64_t v)
{
  uint64_t r;
  uint8_t *pv, *pr;
	
  pv = (uint8_t *) &v;
  pr = (uint8_t *) &r;
	
  pr[0] = pv[7];
  pr[1] = pv[6];
  pr[2] = pv[5];
  pr[3] = pv[4];
  pr[4] = pv[3];
  pr[5] = pv[2];
  pr[6] = pv[1];
  pr[7] = pv[0];
  return r;
}
#endif


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int pkt_seq = 0;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_initialize(struct _worldsens_s *worldsens)
{
  struct sockaddr_in addr;
	
  /* Unicast socket */
  if ((worldsens->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("worldsens_s_initialize (socket):");
    return -1;
  }
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(g_lport);
  addr.sin_addr.s_addr = INADDR_ANY;
	
	
  /* Bind */
  if (bind(worldsens->fd, (struct sockaddr *) (&addr), 
	   sizeof(addr)) != 0) {
    perror("worldsens_s_initialize (bind):");
    close(worldsens->fd);
    return -1;
  }
	
  /* Initialize multicast */
  memset(&worldsens->maddr, 0, sizeof(worldsens->maddr));
  if (inet_aton(g_maddr, &worldsens->maddr.sin_addr) == 0) {
    perror("worldsens_s_initialize (inet_aton):");
    close(worldsens->fd);
    return -1;
  }
  worldsens->maddr.sin_port = htons(g_mport);
  worldsens->maddr.sin_family = AF_INET;
	
  /* Initialize RP */
  worldsens->rp_seq = 1;
  worldsens->rp = WORLDSENS_SYNCH_PERIOD;
  worldsens->synched = 0;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_s_connect(struct _worldsens_s *worldsens, struct sockaddr_in *addr, char *msg, int UNUSED len) 
{
  struct _worldsens_s_connect_pkt s_pkt;
  struct _worldsens_c_connect_pkt *c_pkt = (struct _worldsens_c_connect_pkt *) msg;
  
  WSNET_S_DBG_DBG("WSNET (%"PRId64"): --> CONNECT (ip: %d)\n", g_time, ntohl(c_pkt->node));
  
  if (node_create(ntohl(c_pkt->node))) 
    {
      /* Forge */
      s_pkt.type = WORLDSENS_S_NOATTRADDR;
      
      /* Send */
      if (sendto(worldsens->fd, (char *)(&s_pkt), sizeof(struct _worldsens_s_connect_pkt), 
		 0, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) 
	  <  (int)sizeof(struct _worldsens_s_connect_pkt)) 
	{
	  perror("worldsens_s_connect (sendto)");
	  close(worldsens->fd);
	  return -1;
	}   
      
      WSNET_S_DBG_EXC("WSNET (%"PRId64"): <-- NOATTRADDR\n", g_time);
      return 0;
    }
	
  /* Forge */
  s_pkt.type    = WORLDSENS_S_ATTRADDR;
  s_pkt.pkt_seq = htonl(pkt_seq); 
  s_pkt.period  = htonll(worldsens->rp - g_time);	
  s_pkt.rp_seq  = htonl(worldsens->rp_seq); 

  /* Send */
  if (sendto(worldsens->fd, (char *)(&s_pkt), sizeof(struct _worldsens_s_connect_pkt), 
	     0, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) 
      < (int)sizeof(struct _worldsens_s_connect_pkt)) 
    {
      perror("worldsens_s_connect (sendto)");
      close(worldsens->fd);
      return -1;
    }

  WSNET_S_DBG_DBG("WSNET (%"PRId64", %d): <-- ATTRADDR (ip: %d)\n", 
		  g_time, ntohl(s_pkt.pkt_seq), ntohl(c_pkt->node));	
  WSNET_S_DBG_DBG("WSNET (%"PRId64", %d): <-- RP (seq: %d, period: %"PRId64")\n", 
		  g_time, pkt_seq, worldsens->rp_seq, worldsens->rp - g_time);	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_disconnect(struct _worldsens_s UNUSED *worldsens, struct sockaddr_in UNUSED *addr, char *msg, int UNUSED len) 
{
  struct _worldsens_c_disconnect_pkt *pkt = (struct _worldsens_c_disconnect_pkt *) msg;
  WSNET_S_DBG_DBG("WSNET (%"PRId64", -1): --> DISCONNECT (ip: %d)\n", g_time, ntohl(pkt->node));
  return node_delete(ntohl(pkt->node));
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_listen_to_next_rp(struct _worldsens_s *worldsens) {
  char msg[WORLDSENS_MAX_PKTLENGTH];
  int len;
  socklen_t addrlen;
  struct sockaddr_in addr;
	
  /* Loop */
  while (1) {
		
    /* Wait */
    addrlen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    if ((len = recvfrom(worldsens->fd, msg, WORLDSENS_MAX_PKTLENGTH, 0, (struct sockaddr *) &addr, &addrlen)) <= 0) 
      {
	perror("worldsens_s_listen_to_next_rp (recvfrom)");
	close(worldsens->fd);
	return -1;
      }
		
    /* Disconnect*/
    if (msg[0] & WORLDSENS_C_DISCONNECT) {
      if (worldsens_s_disconnect(worldsens, &addr, msg, len)) {
	worldsens_s_clean(worldsens);
	return -1;
      }
			
      /* Synched */
      if (worldsens->synched == g_c_nodes) {
	g_time = worldsens->rp;
	WSNET_S_DBG_DBG("WSNET (%"PRId64"): === TIME (seq: %d)\n", g_time, worldsens->rp_seq);
	return 0;
      }
    }
		
    /* Connect */
    if (msg[0] & WORLDSENS_C_CONNECT) {
      if (worldsens_s_connect(worldsens, &addr, msg, len)) {
	return -1;
      }
      continue;
    }
		
    /* Sync */
    if (msg[0] & WORLDSENS_C_SYNCHED) {
      struct _worldsens_c_synched_pkt *c_pkt = (struct _worldsens_c_synched_pkt *) msg;			
			
      if (ntohl(c_pkt->rp_seq) == (unsigned)worldsens->rp_seq) 
	{
	  worldsens->synched++;
	} 
      else 
	{
	  continue;
	}
			
      WSNET_S_DBG_DBG("WSNET (%"PRId64"): --> SYN  (seq: %d)\n", g_time, worldsens->rp_seq);

      /* Synched */
      if (worldsens->synched == g_c_nodes) {
	g_time = worldsens->rp;
	WSNET_S_DBG_DBG("WSNET (%"PRId64"): === TIME (seq: %d)\n", g_time, worldsens->rp_seq);
	return 0;
      }
    }
		
    /* Tx */
    if (msg[0] & WORLDSENS_C_TX) {
      struct _worldsens_c_tx_pkt *pkt = (struct _worldsens_c_tx_pkt *) msg;
      struct _node *node = &g_nodes[ntohl(pkt->node)];
			
      if ((g_time + ntohll(pkt->period)) > worldsens->rp) {

	/* Unsynchronized  */
	WSNET_S_DBG_EXC("WSNET (%"PRId64", -1): --> Deprecetated tx (time: %"PRId64", rp: %"PRId64")\n", g_time, (g_time + ntohll(pkt->period)), worldsens->rp);
	continue;

      } else {
	struct _packet *packet;
	struct _packet *p_loop = g_packets;
	int drop = 0;

	while (p_loop) {
	  if ((p_loop->node == node) 
	      && (p_loop->seq == ntohl(pkt->pkt_seq))) {
	    /* Retransmission */
	    WSNET_S_DBG_EXC("WSNET (%"PRId64", -1): -->  Retransmit tx (ip: %d, seq: %d)\n", g_time, node->addr, ntohl(pkt->pkt_seq));
	    drop = 1;
	    break;
	  }
	  p_loop = p_loop->next;
	}
	if (drop)
	  continue;

				
	/* Create packet */
	if ((packet = packet_create(node, 1)) == NULL) {
	  return -1;
	}
	packet->data[0] = pkt->data;
	packet->node = node;
	mobility_update(node);
	packet->x = node->x;
	packet->y = node->y;
	packet->z = node->z;
	packet->radio = ntohl(pkt->frequency);
	packet->modulation = ntohl(pkt->modulation);
	packet->tx_mW = ntohll(pkt->tx_mW);
	packet->seq = ntohl(pkt->pkt_seq);
	packet->tx_start = g_time + ntohll(pkt->period);
	packet->tx_end = packet->tx_start + ntohll(pkt->duration); 	
				
	WSNET_S_DBG_DBG("WSNET (%"PRId64", %d): --> TX (ip: %d, data: 0x%x, freq: %d, mod: %d, tx_mW: %lf, duration: %"PRId64", end: %"PRId64", period: %"PRId64")\n",
			packet->tx_start, packet->seq, node->addr, packet->data[0] & 0xff, packet->radio, packet->modulation, packet->tx_mW, packet->tx_end - packet->tx_start, packet->tx_end, packet->tx_start - g_time);
				
	/* Create event */
	if (core_add_packet(packet))
	  return -1;
				
	if (packet->tx_end < worldsens->rp) {
	  /* Need backtrack and RP */
	  if (worldsens_s_backtrack_async(worldsens, packet->tx_end - g_time)) {
	    return -1;
	  } 
	}
      }
    }
  }
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_s_backtrack_async(struct _worldsens_s *worldsens, uint64_t period) 
{
  struct _worldsens_s_connect_pkt pkt;
  
  /* Update */
  worldsens->rp_seq++;
  worldsens->rp = g_time + period;
  worldsens->synched = 0;
  if (core_backtrack(worldsens->rp))
    {
      return -1;
    }
  
  /* Forge */
  pkt.type = WORLDSENS_S_BACKTRACK;
  pkt.pkt_seq = htonl(pkt_seq++); 
  pkt.period = htonll(period);	
  pkt.rp_seq = htonl(worldsens->rp_seq); 
  
  /* Send */
  if (sendto(worldsens->fd, (char *)(&pkt), sizeof(struct _worldsens_s_backtrack_pkt), 0, 
	     (struct sockaddr *) &worldsens->maddr, sizeof(struct sockaddr_in)) 
      < (int)sizeof(struct _worldsens_s_backtrack_pkt)) 
    {
      perror("worldsens_s_backtrack_async (sendto)");
      close(worldsens->fd);
      return -1;
    }

  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- BACKTRACK\n", 
		  g_time, pkt_seq - 1);
  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n", 
		  g_time, pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  return 0;
}



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_save_release_request(struct _worldsens_s *worldsens, uint64_t period) 
{
  struct _worldsens_s_saverel_pkt pkt;
  
  /* Update */
  worldsens->rp      = g_time + period;
  worldsens->synched = 0;
	
  /* Forge */
  pkt.type     = WORLDSENS_S_SYNCH_REQ;
  pkt.pkt_seq  = htonl(pkt_seq++); 
  pkt.c_rp_seq = htonl(worldsens->rp_seq); 
  worldsens->rp_seq++;
  pkt.period   = htonll(period);	
  pkt.n_rp_seq = htonl(worldsens->rp_seq); 

  /* Send */
  if (sendto(worldsens->fd, (char *)(&pkt), sizeof(struct _worldsens_s_saverel_pkt), 0, 
	     (struct sockaddr *) &worldsens->maddr, sizeof(struct sockaddr_in))
      < (int)sizeof(struct _worldsens_s_saverel_pkt)) 
    {
      perror("worldsens_s_save_release_request (sendto)");
      close(worldsens->fd);
      return -1;
    }
	
  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- SAVE\n", 
		  g_time, pkt_seq - 1);	
  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n", 
		  g_time, pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_save_release_request_rx(struct _worldsens_s *worldsens, int node,  
					int radio, int modulation,  
					struct _worldsens_data *data, uint64_t period) {
  char reply[sizeof(struct _worldsens_s_srrx_pkt) + sizeof(struct _worldsens_data) * g_c_nodes];  
  struct _worldsens_s_srrx_pkt *pkt = (struct _worldsens_s_srrx_pkt *) reply;
  int length = sizeof(struct _worldsens_s_srrx_pkt) + sizeof(struct _worldsens_data) * g_c_nodes;
	
	
  /* Update */
  worldsens->rp = g_time + period;
  worldsens->synched = 0;
	
  /* Forge */
  memcpy(reply + sizeof(struct _worldsens_s_srrx_pkt), (char *) data, g_c_nodes * sizeof(struct _worldsens_data));
  pkt->type = WORLDSENS_S_SYNCH_REQ | WORLDSENS_S_RX;
  pkt->pkt_seq = htonl(pkt_seq++); 
  pkt->c_rp_seq = htonl(worldsens->rp_seq); 
  worldsens->rp_seq++;
  pkt->period = htonll(period);	
  pkt->n_rp_seq = htonl(worldsens->rp_seq); 
  pkt->size = htonl(length);
  pkt->node = htonl(node);
  pkt->frequency = htonl(radio);
  pkt->modulation = htonl(modulation);
		
  /* Send */
  if (sendto(worldsens->fd, reply, length, 0, (struct sockaddr *) &worldsens->maddr, sizeof(struct sockaddr_in)) < length) {
    perror("worldsens_s_release__request_rx (sendto)");
    close(worldsens->fd);
    return -1;
  }
  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- SAVE\n", g_time, pkt_seq - 1);	
  WSNET_S_DBG_EXC("WSNET (%"PRId64", %d): <-- RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n", g_time, pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  WSNET_S_DBG_EXC("WSNET (%"PRId64", -1): <-- RX (ip: %d, frequency: %d, modulation: %d)\n", g_time, node, radio, modulation);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_rx(struct _worldsens_s *worldsens, int node,
		   int radio,  int modulation, 
		   struct _worldsens_data *data) {
  char reply[sizeof(struct _worldsens_s_rx_pkt) + sizeof(struct _worldsens_data) * g_c_nodes];  
  struct _worldsens_s_rx_pkt *pkt = (struct _worldsens_s_rx_pkt *) reply;
  int length = sizeof(struct _worldsens_s_rx_pkt) + sizeof(struct _worldsens_data) * g_c_nodes;
	
		
  /* Forge */
  memcpy(reply + sizeof(struct _worldsens_s_rx_pkt), (char *) data, g_c_nodes * sizeof(struct _worldsens_data));
  pkt->type = WORLDSENS_S_SYNCH_REQ | WORLDSENS_S_RX;
  pkt->pkt_seq = htonl(pkt_seq++); 
  pkt->size = htonl(length);
  pkt->node = htonl(node);
  pkt->frequency = htonl(radio);
  pkt->modulation = htonl(modulation);
	
  /* Send */	
  if (sendto(worldsens->fd, reply, length, 0, (struct sockaddr *) &worldsens->maddr, sizeof(struct sockaddr_in)) < length) {
    perror("worldsens_s_rx (sendto)");
    close(worldsens->fd);
    return -1;
  }
	
  WSNET_S_DBG_EXC("WSNET (%"PRId64", -1): <-- RX (ip: %d, frequency: %d, modulation: %d)\n", g_time, node, radio, modulation);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int worldsens_s_clean(struct _worldsens_s *worldsens)
{
  close(worldsens->fd);
  core_runtime_end();
  exit(0);
}


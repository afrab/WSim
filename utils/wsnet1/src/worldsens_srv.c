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
char *g_maddr = "224.0.0.1";
char *g_saddr = "localhost";

int pkt_seq                = 0;
int simulation_keeps_going = 1;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if (BIG_ENDIAN == BYTE_ORDER)
static uint64_t
ntohll (uint64_t v)
{
  return v;
}

static uint64_t
htonll (uint64_t v)
{
  return v;
}

static double
ntohdbl (double v)
{
  return v;
}

static double
htondbl (double v)
{
  return v;
}

#else
static uint64_t
ntohll (uint64_t v)
{
  uint64_t r;
  uint8_t *pv, *pr;

  pv = (uint8_t *) & v;
  pr = (uint8_t *) & r;

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

static uint64_t
htonll (uint64_t v)
{
  uint64_t r;
  uint8_t *pv, *pr;

  pv = (uint8_t *) & v;
  pr = (uint8_t *) & r;

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

static double
ntohdbl (double v)
{
  double r;
  uint8_t *pv, *pr;

  pv = (uint8_t *) & v;
  pr = (uint8_t *) & r;

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

static double
htondbl (double v)
{
  double r;
  uint8_t *pv, *pr;

  pv = (uint8_t *) & v;
  pr = (uint8_t *) & r;

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

void worldsens_packet_dump(char UNUSED *msg, char UNUSED *pkt, int UNUSED size)
{
  /*
  int i;
  WSNET_S_DBG_DBG ("WSNET:: %s start ================================\n", msg)
  WSNET_S_DBG_DBG ("WSNET::%s ", msg);
  for(i=0;i<size; i++)
    {
      WSNET_S_DBG_DBG ("%02x:",pkt[i] & 0xff);
    }
  WSNET_S_DBG_DBG ("\n");
  WSNET_S_DBG_DBG ("WSNET:: %s stop  ================================\n", msg);
  */
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_sendto(struct _worldsens_s *worldsens, char *pkt, int size, struct sockaddr_in *addr)
{
  int ret;

  if (worldsens->mfd > -1)
    {
      ret = sendto (worldsens->mfd, pkt, size, 0, (struct sockaddr*) addr, sizeof (struct sockaddr_in));
      if (ret < size)
	{
	  perror ("worldsens:multicast:sendto");
	  close (worldsens->mfd);
	  worldsens->mfd = -1;
	  return -1;
	}
      worldsens_packet_dump("send",pkt,size);
      tracer_event_record(worldsens->trc_mcast_tx, pkt[0]);
    }
  return 0;
}

int
worldsens_s_recvfrom(struct _worldsens_s *worldsens, char *pkt, int sizemax, struct sockaddr_in *addr)
{
  int len;
  socklen_t addrlen;

  addrlen = sizeof (struct sockaddr_in);
  len = recvfrom(worldsens->mfd, pkt, sizemax, 0, (struct sockaddr *) addr, &addrlen);

  if (len <= 0)
    {
      perror ("worldsens:multicast:recvfrom");
      close  (worldsens->mfd);
      worldsens->mfd = -1;
      return -1;
    }

  worldsens_packet_dump("recv",pkt,len);
  tracer_event_record(worldsens->trc_mcast_rx, pkt[0]);
  return len;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_initialize (struct _worldsens_s *worldsens)
{
  struct sockaddr_in addr;

  /* Unicast socket */
  if ((worldsens->mfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("worldsens_s_initialize (socket):");
      return -1;
    }
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (g_lport);
  addr.sin_addr.s_addr = INADDR_ANY;


  /* Bind */
  if (bind (worldsens->mfd, (struct sockaddr *) (&addr), sizeof (addr)) != 0)
    {
      perror ("worldsens_s_initialize:bind:");
      close (worldsens->mfd);
      worldsens->mfd = -1;
      return -1;
    }

  /* Initialize multicast */
  memset (&worldsens->maddr, 0, sizeof (worldsens->maddr));
  if (inet_aton (g_maddr, &worldsens->maddr.sin_addr) == 0)
    {
      perror ("worldsens_s_initialize:inet_aton:");
      close (worldsens->mfd);
      worldsens->mfd = -1;
      return -1;
    }
  worldsens->maddr.sin_port   = htons (g_mport);
  worldsens->maddr.sin_family = AF_INET;

  /* Initialize RP */
  worldsens->rp_seq           = 1;
  worldsens->rp               = WORLDSENS_SYNCH_PERIOD;
  worldsens->synched          = 0;

  /* Initialize tracer */
  worldsens->trc_mcast_rx     = tracer_event_add_id(64,"mcast_rx","wsnet1");
  worldsens->trc_mcast_tx     = tracer_event_add_id(64,"mcast_tx","wsnet1");

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_connect (struct _worldsens_s *worldsens, struct sockaddr_in *addr,
		     char *msg, int UNUSED len)
{
  struct _worldsens_c_connect_pkt *c_pkt = (struct _worldsens_c_connect_pkt *) msg;

  struct _worldsens_s_connect_pkt s_pkt;
  int pktlength = sizeof (struct _worldsens_s_connect_pkt);

  WSNET_S_DBG_DBG ("WSNET:: <-- CONNECT (ip: %d)\n", ntohl (c_pkt->node));

  if (node_create (ntohl (c_pkt->node)))
    {
      /* Forge */
      s_pkt.type = WORLDSENS_S_NOATTRADDR;

      /* Multicast Send */
      if (worldsens_s_sendto(worldsens,(char *) (&s_pkt), pktlength, addr))
	{
	  return -1;
	}

      WSNET_S_DBG_EXC ("WSNET:: --> NOATTRADDR\n");
      return 0;
    }

  /* Forge */
  s_pkt.type     = WORLDSENS_S_ATTRADDR;
  s_pkt.pkt_seq  = htonl (pkt_seq);
  s_pkt.period   = htonll(worldsens->rp - get_global_time());
  s_pkt.rp_seq   = htonl (worldsens->rp_seq);
  s_pkt.cnx_time = htonll(get_global_time());
  pktlength      = sizeof(s_pkt);

  /* Send */
  if (worldsens_s_sendto(worldsens,(char *) (&s_pkt), pktlength, addr))
    {
      return -1;
    }

  WSNET_S_DBG_DBG ("WSNET:: --> ATTRADDR (seq: %d, ip: %d) global time %"PRId64"\n",
		   ntohl (s_pkt.pkt_seq), ntohl (c_pkt->node), ntohll(s_pkt.cnx_time));
  WSNET_S_DBG_DBG ("WSNET:: --> RP (seq: %d, rp_seq: %d, period: %"PRId64 ")\n", 
		   pkt_seq, worldsens->rp_seq, worldsens->rp - get_global_time());
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_disconnect (struct _worldsens_s UNUSED * worldsens,
			struct sockaddr_in UNUSED * addr, char *msg,
			int UNUSED len)
{
  struct _worldsens_c_disconnect_pkt *pkt =
    (struct _worldsens_c_disconnect_pkt *) msg;
  WSNET_S_DBG_DBG ("WSNET:: <-- DISCONNECT (ip: %d)\n", ntohl (pkt->node));
  return node_delete (ntohl (pkt->node));
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_listen_to_next_rp (struct _worldsens_s *worldsens)
{
  char msg[WORLDSENS_MAX_PKTLENGTH];
  int len;
  struct sockaddr_in addr;

  /* Loop */
  while (1)
    {

      /* Wait */
      memset (&addr, 0, sizeof (addr));
            
      if ((len = worldsens_s_recvfrom(worldsens, msg, WORLDSENS_MAX_PKTLENGTH, &addr)) <= 0)
	{
	  return -1;
	}
            
      /*****************/
      /* Disconnect    */
      /*****************/
      if (msg[0] & WORLDSENS_C_DISCONNECT)
	{
	  if (worldsens_s_disconnect (worldsens, &addr, msg, len))
	    {
	      worldsens_s_clean (worldsens);
	      return -1;
	    }

	  /* Synched */
	  if (worldsens->synched == g_c_nodes)
	    {
	      set_global_time( worldsens->rp );
	      return 0;
	    }
	}

      /*****************/
      /* Connect       */
      /*****************/
      if (msg[0] & WORLDSENS_C_CONNECT)
	{
	  if (worldsens_s_connect (worldsens, &addr, msg, len))
	    {
	      return -1;
	    }
	  continue;
	}

      /*****************/
      /* Sync          */
      /*****************/
      if (msg[0] & WORLDSENS_C_SYNCHED)
	{
	  struct _worldsens_c_synched_pkt *c_pkt =
	    (struct _worldsens_c_synched_pkt *) msg;

	  if (ntohl (c_pkt->rp_seq) == (unsigned) worldsens->rp_seq)
	    {
	      worldsens->synched++;
	    }
	  else
	    {
	      continue;
	    }

	  WSNET_S_DBG_DBG ("WSNET:: <-- SYN  (seq: %d)\n",  worldsens->rp_seq);

	  /* Synched */
	  if (worldsens->synched == g_c_nodes)
	    {
	      set_global_time( worldsens->rp );
	      return 0;
	    }
	}

      /*****************/
      /* Tx            */
      /*****************/
      if (msg[0] & WORLDSENS_C_TX)
	{
	  struct _worldsens_c_tx_pkt *pkt =
	    (struct _worldsens_c_tx_pkt *) msg;
	  struct _node *node = &g_nodes[ntohl (pkt->node)];

	  if ((get_global_time() + ntohll (pkt->period)) > worldsens->rp)
	    {

	      /* Unsynchronized  */
	      WSNET_S_DBG_EXC ("WSNET:: <-- Deprecetated tx (time: %"PRId64", current rp: %"PRId64")\n", 
			       (get_global_time() + ntohll (pkt->period)), worldsens->rp);
	      continue;

	    }
	  else
	    {
	      struct _packet *packet;
	      struct _packet *p_loop = g_packets;
	      int drop = 0;

	      while (p_loop)
		{
		  if ((p_loop->node == node)
		      && (p_loop->seq == ntohl (pkt->pkt_seq)))
		    {
		      /* Retransmission */
		      WSNET_S_DBG_EXC ("WSNET:: <--  Retransmit tx (ip: %d, seq: %d)\n",
				       node->addr, ntohl (pkt->pkt_seq));
		      drop = 1;
		      break;
		    }
		  p_loop = p_loop->next;
		}
	      if (drop)
		continue;


	      /* Create packet */
	      if ((packet = packet_create (node, 1)) == NULL)
		{
		  return -1;
		}
	      packet->data[0] = pkt->data;
	      packet->node = node;
	      mobility_update (node);
	      packet->x = node->x;
	      packet->y = node->y;
	      packet->z = node->z;
	      packet->freq = ntohl (pkt->frequency);
	      packet->modulation = ntohl (pkt->modulation);
	      packet->tx_mW = ntohdbl (pkt->tx_mW);
	      packet->seq = ntohl (pkt->pkt_seq);
	      packet->tx_start = get_global_time() + ntohll (pkt->period);
	      packet->tx_end = packet->tx_start + ntohll (pkt->duration);

	      {
		int iii;
		WSNET_S_DBG_DBG ("WSNET:: <-- TX (%"PRId64",%d) (ip:%d,size:%d,data:",
				 packet->tx_start, packet->seq, node->addr,packet->size);
		for(iii=0; iii<packet->size; iii++)
		  {
		    WSNET_S_DBG_DBG("%02x:", packet->data[ iii ] & 0xff);
		  }
		WSNET_S_DBG_DBG (",freq:%gMHz,mod:%d,tx:%lgmW)\n",
			       (unsigned)packet->freq / 1000000.0, packet->modulation, packet->tx_mW);
	      }

	      /* Create event */
	      if (core_add_packet (packet))
		return -1;

	      if (packet->tx_end < worldsens->rp)
		{
		  /* Need backtrack and RP */
		  if (worldsens_s_backtrack_async
		      (worldsens, packet->tx_end - get_global_time()))
		    {
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

int
worldsens_s_backtrack_async (struct _worldsens_s *worldsens, uint64_t period)
{
  struct _worldsens_s_backtrack_pkt pkt;
  int pktlength = sizeof(pkt);

  /* Update */
  worldsens->rp_seq++;
  worldsens->rp = get_global_time() + period;
  worldsens->synched = 0;
  if (core_backtrack (worldsens->rp))
    {
      return -1;
    }

  /* Forge */
  pkt.type    = WORLDSENS_S_BACKTRACK;
  pkt.pkt_seq = htonl  (pkt_seq++);
  pkt.period  = htonll (period);
  pkt.rp_seq  = htonl  (worldsens->rp_seq);

  /* Send */
  if (worldsens_s_sendto(worldsens,(char *) (&pkt), pktlength, &worldsens->maddr))
    {
      return -1;
    }
  
  /*
  if (sendto
      (worldsens->mfd, (char *) (&pkt),
       sizeof (struct _worldsens_s_backtrack_pkt), 0,
       (struct sockaddr *) &worldsens->maddr,
       sizeof (struct sockaddr_in)) <
      (int) sizeof (struct _worldsens_s_backtrack_pkt))
    {
      perror ("worldsens_s_backtrack_async (sendto)");
      close (worldsens->mfd);
      return -1;
    }
  */

  WSNET_S_DBG_EXC ("WSNET:: --> (seq: %d) BACKTRACK \n", pkt_seq - 1);
  WSNET_S_DBG_EXC ("WSNET:: --> (seq: %d) RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n", 
		   pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  return 0;
}



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_save_release_request (struct _worldsens_s *worldsens,
				  uint64_t period)
{
  struct _worldsens_s_saverel_pkt pkt;
  int pktlength = sizeof(pkt);

  /* Update */
  worldsens->rp = get_global_time() + period;
  worldsens->synched = 0;

  /* Forge */
  pkt.type = WORLDSENS_S_SYNCH_REQ;
  pkt.pkt_seq = htonl (pkt_seq++);
  pkt.c_rp_seq = htonl (worldsens->rp_seq);
  worldsens->rp_seq++;
  pkt.period = htonll (period);
  pkt.n_rp_seq = htonl (worldsens->rp_seq);

  /* Send */
  if (worldsens_s_sendto(worldsens,(char *) (&pkt), pktlength, &worldsens->maddr))
    {
      return -1;
    }

  WSNET_S_DBG_EXC ("WSNET:: --> (seq:%d) SAVE + RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n", 
		   pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int
worldsens_s_save_release_request_rx (struct _worldsens_s *worldsens, int node,
				     int freq, int modulation,
				     struct _worldsens_data *data,
				     uint64_t period)
{
  char reply[sizeof (struct _worldsens_s_srrx_pkt) +
	     sizeof (struct _worldsens_data) * g_c_nodes];
  struct _worldsens_s_srrx_pkt *pkt = (struct _worldsens_s_srrx_pkt *) reply;
  int length =
    sizeof (struct _worldsens_s_srrx_pkt) +
    sizeof (struct _worldsens_data) * g_c_nodes;


  /* Update */
  worldsens->rp = get_global_time() + period;
  worldsens->synched = 0;

  /* Forge */
  memcpy (reply + sizeof (struct _worldsens_s_srrx_pkt), (char *) data,
	  g_c_nodes * sizeof (struct _worldsens_data));
  pkt->type       = WORLDSENS_S_SYNCH_REQ | WORLDSENS_S_RX;
  pkt->pkt_seq    = htonl  (pkt_seq++);
  pkt->c_rp_seq   = htonl  (worldsens->rp_seq);
  worldsens->rp_seq++;
  pkt->period     = htonll (period);
  pkt->n_rp_seq   = htonl  (worldsens->rp_seq);
  pkt->size       = htonl  (length);
  pkt->node       = htonl  (node);
  pkt->frequency  = htonl  (freq);
  pkt->modulation = htonl  (modulation);

  /* Send */
  if (worldsens_s_sendto(worldsens, reply, length ,&worldsens->maddr))
    {
      return -1;
    }

  WSNET_S_DBG_EXC ("WSNET:: --> (seq: %d) SAVE + RP (seq: %d, period: %"PRId64", rp: %"PRId64")\n",
		   pkt_seq - 1, worldsens->rp_seq, period, worldsens->rp);
  WSNET_S_DBG_EXC ("WSNET:: --> RX (ip:%d, freq:%gMHz, modul: %d)\n", 
		   node, (unsigned)freq / 1000000.0, modulation);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**
 * Sends a data on the multicast channel
 *
 */

int
worldsens_s_rx (struct _worldsens_s *worldsens, int node, int freq,
		int modulation, struct _worldsens_data *data)
{
  int length =
    sizeof (struct _worldsens_s_rx_pkt) +
    sizeof (struct _worldsens_data) * g_c_nodes;
  char reply[length];

  struct _worldsens_s_rx_pkt *pkt = (struct _worldsens_s_rx_pkt *) reply;

  /* Forge */
  memcpy (reply + sizeof (struct _worldsens_s_rx_pkt), (char *) data,
	  g_c_nodes * sizeof (struct _worldsens_data));

  pkt->type       = WORLDSENS_S_SYNCH_REQ | WORLDSENS_S_RX;
  pkt->pkt_seq    = htonl (pkt_seq++);
  pkt->size       = htonl (length);
  pkt->node       = htonl (node);
  pkt->frequency  = htonl (freq);
  pkt->modulation = htonl (modulation);

  /* Multicast Send */
  if (worldsens_s_sendto(worldsens, reply, length, &worldsens->maddr))
    {
      return -1;
    }

  WSNET_S_DBG_EXC ("WSNET:: --> RX (ip: %d, size:%d, freq:%gMHz, modul:%d)\n", 
		   node, length, (unsigned)freq / 1000000.0, modulation);
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
worldsens_s_clean (struct _worldsens_s *worldsens)
{
  close (worldsens->mfd);
  worldsens->mfd         = -1;
  simulation_keeps_going =  0;
  core_runtime_end ();
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

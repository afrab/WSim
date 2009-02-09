/*
 *  core.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include "private/models_private.h"
#include "private/simulation_private.h"
#include "private/propagation_private.h"
#include "private/antenna_private.h"
#include "private/radio_private.h"
#include "private/mac_private.h"
#include "private/queue_private.h"
#include "private/application_private.h"
#include "private/nodes_private.h"
#include "private/packets_private.h"
#include "private/interference_private.h"
#include "private/mobility_private.h"
#include "private/modulation_private.h"
#include "private/core_private.h"
#include "private/battery_private.h"

#include "public/log.h"
#include "worldsens.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct _event *g_events = NULL;
int g_events_card = 0;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void
core_runtime_end (void)
{
  struct _packet *packet;
  struct _event *event;

#ifndef WORLDSENS
  int loop = g_m_nodes;

  while (loop--)
    {
      struct _node *node = &(g_nodes[loop]);

      application_complete (node);
      queue_complete (node);
      mac_complete (node);
      radio_complete (node);
      antenna_complete (node);
      battery_complete (node);
    }
#endif //WORLDSENS

  modulation_complete ();
  interference_complete ();
  propagation_complete ();

  while (g_events)
    {
      event = g_events;
      g_events = g_events->next;
      free (event);
    }

  while (g_packets)
    {
      packet = g_packets;
      g_packets = g_packets->next;
      packet_destroy (packet);
    }

  models_clean ();
  free (g_nodes);
  return;
}


#ifndef WORLDSENS
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void
core_notify_carrier_sense (void)
{
  struct _packet *p_loop = g_packets;

  while (p_loop)
    {
      int i = g_m_nodes;

      /* Only deal with packets that start now */
      if (p_loop->tx_start != g_time)
	{
	  p_loop = p_loop->next;
	  continue;
	}

      while (i--)
	{
	  double rx_mW, noise;

	  /* Update receiving node mobility */
	  mobility_update (&g_nodes[i]);

	  /* Compute rx_mW and SINR */
	  rx_mW = propagation_compute_rx_mW (&g_nodes[i], p_loop);
	  noise =
	    propagation_compute_noise (&g_nodes[i], p_loop->radio, p_loop->id,
				       g_time);

	  /* Notify carrier sense */
	  radio_carrier_sense (&g_nodes[i], p_loop->id, rx_mW, rx_mW / noise,
			       p_loop->radio);
	}

      p_loop = p_loop->next;
    }

  return;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
core_start (void)
{
  static int evt_nb = 0;

  while (g_events)
    {
      struct _event *event = g_events;

      /* Perform cs notification before time advances */
      if (g_time != event->time)
	{
	  core_notify_carrier_sense ();
	}

      /* And time goes on... */
      g_time = event->time;
      if (g_time > g_simend)
	{
	  g_time = g_simend;
	  break;
	}
      evt_nb++;


      if (event->type == SCHED_EVENT_TYPE)
	{
	  void *arg = event->arg;

	  /* Remove event before calling callback */
	  g_events = event->next;
	  g_events_card--;
	  event->callback (arg);
	  free (event);

	}
      else
	{
	  struct _packet *packet = event->packet;
	  int i = g_m_nodes;

	  /* Remove event */
	  g_events = event->next;
	  g_events_card--;
	  free (event);

	  while (i--)
	    {
	      struct _packet *rx_pkt = packet_duplicate (packet);

	      /* Update receiving node mobility */
	      mobility_update (&g_nodes[i]);

	      /* Compute packet BER/SiNR and notify receiving node */
	      propagation_compute_BER (&g_nodes[i], rx_pkt);
	      antenna_rx (&g_nodes[i], rx_pkt);
	    }

	  /* Update "in the air tonight" packet list */
	  if (core_update_packet_list ())
	    {
	      return -1;
	    }
	}
    }

  core_runtime_end ();
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#else //WORLDSENS

#if (BIG_ENDIAN == BYTE_ORDER)
static uint64_t
htonll (uint64_t v)
{
  return v;
}
#else
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
#endif


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
core_start (struct _worldsens_s *worldsens)
{
  static int evt_nb = 0;

  simulation_keeps_going = 1;

  while (simulation_keeps_going == 1)
    {

      /* If no event or if event in the future, wait at rp point */
      if ((g_events == NULL) || (g_events->time > g_time))
	{
	  worldsens_s_listen_to_next_rp (worldsens);
	}

      if (g_events == NULL)
	{
	  /* If no event, program rp point */
	  if (worldsens_s_save_release_request
	      (worldsens, WORLDSENS_SYNCH_PERIOD))
	    return -1;
	}
      else if (g_time < g_events->time)
	{
	  /* If event, program event */
	  if (worldsens_s_save_release_request
	      (worldsens, g_events->time - g_time))
	    return -1;
	}
      else
	{
	  struct _event *event = g_events;

	  /* And time goes on... */
	  if (g_time != event->time)
	    {
	      fprintf (stderr,
		       "\nEXCEPTION: CORE DESYNCHRONIZATION time=%" PRId64
		       ", evt_time=%" PRId64 "\n", g_time, event->time);
	      return -1;
	    }
	  evt_nb++;

	  if (event->type == TX_EVENT_TYPE)
	    {
	      struct _packet *packet = event->packet;
	      struct _worldsens_data worldsens_data[g_c_nodes];
	      int i = g_m_nodes;
	      int c_node = 0;

	      /* Remove event */
	      g_events = event->next;
	      g_events_card--;
	      free (event);

	      while (i--)
		{
		  if (g_nodes[i].active)
		    {
		      struct _packet *rx_pkt = packet_duplicate (packet);

		      /* Update receiving node mobility */
		      mobility_update (&g_nodes[i]);

		      /* Compute packet BER/SiNR */
		      propagation_compute_BER (&g_nodes[i], rx_pkt);

		      /* Record reception and destroy packet */
		      worldsens_data[c_node].node = htonl (i);
		      worldsens_data[c_node].data = packet->data[0];
		      worldsens_data[c_node].SiNR = htonll (rx_pkt->SiNR[0]);
		      worldsens_data[c_node].rx_mW = htonll (rx_pkt->rx_mW);
		      packet_destroy (rx_pkt);	//TODO: optimize.... no need to creat and destroy packet...

		      /* Update considered node */
		      c_node++;
		    }
		}

	      if (g_events == NULL)
		{
		  /* If no more event, backup and program rp point */
		  if (worldsens_s_save_release_request_rx (worldsens, packet->node->addr, packet->radio, packet->modulation, worldsens_data, 31250))	// ToCheck: optimization to avoir backtracks
		    return -1;
		}
	      else
		{
		  /* If future event, backup and program event */
		  if (g_events->time > g_time)
		    {
		      if (worldsens_s_save_release_request_rx
			  (worldsens, packet->node->addr, packet->radio,
			   packet->modulation, worldsens_data,
			   g_events->time - g_time))
			return -1;
		    }
		  else
		    {
		      /* If simultaneous events, do nothing */
		      if (worldsens_s_rx (worldsens, packet->node->addr,
					  packet->radio, packet->modulation,
					  worldsens_data))
			return -1;
		    }
		}

	      /* Update "in the air tonight" packet list */
	      if (core_update_packet_list ())
		{
		  return -1;
		}
	    }
	}
    }				/* while */

  return 0;
}
#endif //WORLDSENS


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int
core_add_schedule (void (*callback) (void *), void *arg, uint64_t time)
{
  struct _event *event;
  struct _event *loop = NULL;
  struct _event *tmp = NULL;

  if ((event = (struct _event *) malloc (sizeof (struct _event))) == NULL)
    {
      fprintf (stderr, "\nEXCEPTION: MALLOC ERROC\n");
      return -1;
    }

  event->type = SCHED_EVENT_TYPE;
  event->time = time;
  event->callback = callback;
  event->arg = arg;

  if (g_events == NULL)
    {
      g_events = event;
      event->next = NULL;
    }
  else
    {
      loop = g_events;

      /* We want callback events to be called after packet events (for protocol timeouts) */
      while ((loop != NULL) && (loop->time <= event->time))
	{
	  tmp = loop;
	  loop = loop->next;
	}

      if (loop == NULL)
	{
	  tmp->next = event;
	  event->next = NULL;
	}
      else if (tmp == NULL)
	{
	  g_events = event;
	  event->next = loop;
	}
      else
	{
	  tmp->next = event;
	  event->next = loop;
	}
    }

  g_events_card++;
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
core_add_packet (struct _packet *packet)
{
  struct _event *event;
  struct _event *loop = NULL;
  struct _event *tmp = NULL;
  struct _packet *p_loop = NULL;
  struct _packet *p_tmp = NULL;

  if ((event = (struct _event *) malloc (sizeof (struct _event))) == NULL)
    {
      fprintf (stderr, "\nEXCEPTION: MALLOC ERROC\n");
      return -1;
    }

  event->type = TX_EVENT_TYPE;
  event->node = packet->node->addr;
  event->time = packet->tx_end;
  event->packet = packet;

  if (g_events == NULL)
    {
      g_events = event;
      event->next = NULL;
    }
  else
    {
      loop = g_events;

      /* We want callback events to be called after packet events (for protocol timeouts) */
      while ((loop != NULL) && (loop->time < event->time))
	{
	  tmp = loop;
	  loop = loop->next;
	}

      if (loop == NULL)
	{
	  tmp->next = event;
	  event->next = NULL;
	}
      else if (tmp == NULL)
	{
	  g_events = event;
	  event->next = loop;
	}
      else
	{
	  tmp->next = event;
	  event->next = loop;
	}
    }

  /* Update "in the air tonight" packet list */
  if (g_packets == NULL)
    {
      g_packets = packet;
      packet->next = NULL;
    }
  else
    {
      p_loop = g_packets;

      while ((p_loop != NULL) && (p_loop->tx_end < packet->tx_end))
	{
	  p_tmp = p_loop;
	  p_loop = p_loop->next;
	}

      if (p_loop == NULL)
	{
	  p_tmp->next = packet;
	  packet->next = NULL;
	}
      else if (p_tmp == NULL)
	{
	  g_packets = packet;
	  packet->next = p_loop;
	}
      else
	{
	  p_tmp->next = packet;
	  packet->next = p_loop;
	}
    }

  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
core_update_packet_list (void)
{
  struct _packet *p_loop = g_packets;
  struct _packet *p_tmp = NULL;
  struct _event *event = g_events;
  uint64_t tx_start;

  /* Get date of first "in the air tonight" packet */
  tx_start = g_time;
  while (event)
    {
      if ((event->type == TX_EVENT_TYPE)
	  && (event->packet->tx_start < tx_start))
	{
	  tx_start = event->packet->tx_start;
	}
      event = event->next;
    }

  /* Remove anterior packets */
  while (p_loop != NULL)
    {
      if (p_loop->tx_end <= tx_start)
	{
	  if (p_tmp != NULL)
	    {
	      p_tmp->next = p_loop->next;
	      packet_destroy (p_loop);
	      p_loop = p_tmp->next;
	    }
	  else
	    {
	      g_packets = p_loop->next;
	      packet_destroy (p_loop);
	      p_loop = g_packets;
	    }
	}
      else
	{
	  p_tmp = p_loop;
	  p_loop = p_loop->next;
	}
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#ifdef WORLDSENS
int
core_backtrack (uint64_t time)
{
  struct _packet *p_loop = g_packets;
  struct _packet *p_tmp = NULL;
  struct _event *e_loop = g_events;
  struct _event *e_tmp = NULL;

  while (e_loop != NULL)
    {
      if (e_loop->packet->tx_start > time)
	{
	  if (e_tmp != NULL)
	    {
	      e_tmp->next = e_loop->next;
	      free (e_loop);
	      e_loop = e_tmp->next;
	    }
	  else
	    {
	      g_events = e_loop->next;
	      free (e_loop);
	      e_loop = g_events;
	    }
	}
      else
	{
	  e_tmp = e_loop;
	  e_loop = e_loop->next;
	}
    }

  while (p_loop != NULL)
    {
      if (p_loop->tx_start > time)
	{
	  if (p_tmp != NULL)
	    {
	      p_tmp->next = p_loop->next;
	      packet_destroy (p_loop);
	      p_loop = p_tmp->next;
	    }
	  else
	    {
	      g_packets = p_loop->next;
	      packet_destroy (p_loop);
	      p_loop = g_packets;
	    }
	}
      else
	{
	  p_tmp = p_loop;
	  p_loop = p_loop->next;
	}
    }

  return 0;
}
#endif //WORLDSENS

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

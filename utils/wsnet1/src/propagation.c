/*
 *  propagation.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/packets_private.h>
#include <private/propagation_private.h>
#include <private/nodes_private.h>
#include <private/simulation_private.h>
#include <private/interference_private.h>
#include <private/modulation_private.h>
#include <private/mobility_private.h>
#include <private/models_private.h>

#include <public/log.h>
#include <public/antenna.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _model_propagation * g_propagation = NULL;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static inline int min(int a, int b) {
  return a < b ? a : b;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static inline int max(int a, int b) {
  return a > b ? a : b;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int propagation_instantiate(char *key, FILE *config_fd) {
  struct _model_propagation *loop = m_propagation;

  while (loop && strcmp(loop->key, key)) {
    loop = loop->next;
  }
	
  if (loop == NULL) {
    fprintf(stderr, "Configuration error: propagation model not found\n");
    return -1;
  }
	
  g_propagation = loop;
  return g_propagation->propagation_instantiate(config_fd);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double propagation_compute_rx_mW(struct _node *node, struct _packet *packet) {	
  double rx_mW;
	
  rx_mW = packet->node->antenna->antenna_compute_tx(packet->node, packet->tx_mW, node->x - packet->node->x,
						    node->y - packet->node->y, node->z - packet->node->z);
  rx_mW = g_propagation->propagation_propagation(rx_mW, packet->node->x, packet->node->y, packet->node->z, node->x, node->y, node->z);
  rx_mW = node->antenna->antenna_compute_rx(node, rx_mW, node->x - packet->node->x, 
					    node->y - packet->node->y, node->z - packet->node->z);

  /* Filter the low mW packets */
  if (rx_mW < antenna_get_min_rx_mW(node))
    rx_mW = 0;
	
  return rx_mW;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double propagation_compute_noise(struct _node *node, int radio, int p_id, uint64_t time)  {
  double noise = 0.0;
  struct _packet *packet;
	
  /* Compute rx_mW for coliding packets */
  packet = g_packets;
  while (packet != NULL)
    {
      double rx_mW;
		
      /* Remove myself */
      if (packet->id == (unsigned int)p_id) {
	packet = packet->next;
	continue;
      }
		
      /* Check if the current packet is to be considered */	
      if ((packet->tx_start > time) || (packet->tx_end < time)) {
	packet = packet->next;
	continue;
      }
		
      /* Compute rx power associated to the computed packet */
      rx_mW = propagation_compute_rx_mW(node, packet);

      /* Add rx power to the noise */
      noise += g_interference->interference_correlation(rx_mW, packet->radio, radio);
		
      packet = packet->next;
    }
	
  noise += antenna_get_noise_mW(node);
  return noise; 
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/* ToCheck -> relation BER/PER*/
/* TODO: optimization on rx_mW = 0 */
double propagation_compute_BER(struct _node *node, struct _packet *packet) {
  double PER = 1;
  double f_duration;
  int loop;
	
  /* Get rx_mW */
  packet->rx_mW = propagation_compute_rx_mW(node, packet);

  /* Computing frame characteristics */
  f_duration = (packet->tx_end - packet->tx_start) / ((double) packet->size);
	
  for (loop = 0; loop < packet->size; loop++) {
    double noise;
		
    /* Compute noise */
    noise = propagation_compute_noise(node, packet->radio, packet->id, packet->tx_start + (((double) loop) * f_duration));

    /* Compute SiNR */
    if (noise == 0.0) {
      packet->SiNR[loop] = 0.0;
    } else {
      packet->SiNR[loop] = packet->rx_mW / noise;
    }
		
    /* Compute BER and update PER */
    packet->BER[loop] = modulation_compute_BER(packet->SiNR[loop]);
    PER *= pow((1 - packet->BER[loop]), 8);

    /* Introduce errors in packet */
#ifndef WORLDSENS
    if (g_error) {
#else //WORLDSENS
      if (1) {
#endif //WORLDSENS
	int b_current; 
			
	for (b_current = 0; b_current < 8; b_current++) {
	  if (drand48() < packet->BER[loop]) {
	    if ((packet->data[loop] >> b_current) & 0x01) {
	      packet->data[loop] &=  ~(0x01 << b_current);						
	    } else {
	      packet->data[loop] |=  (0x01 << b_current);
	    }
	  }
	}
      }
    }
	
    /* Complete PER computation */
    packet->PER = 1- PER;	
    return PER;
  }


  /**************************************************************************/
  /**************************************************************************/
  /**************************************************************************/
  int propagation_complete(void) {
    return g_propagation->propagation_complete();
  }

/*
 *  antenna.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/nodes_private.h>
#include <private/packets_private.h>
#include <private/antenna_private.h>
#include <private/radio_private.h>
#include <private/core_private.h>

#include <public/log.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_instantiate(struct _node *node, char *key, FILE *config_fd) {
  struct _model_antenna *loop = m_antenna;

  while (loop && strcmp(loop->key, key)) {
    loop = loop->next;
  }
	
  if (loop == NULL) {
    fprintf(stderr, "Configuration error: antenna model not found for node %d\n", node->addr);
    return -1;
  }
	
  node->antenna = loop;
  memset(&node->antenna_public, 0, sizeof(struct _antenna_public));
  memset(node->antenna_private, 0, ANTENNA_PRIVATE_SIZE);
  return node->antenna->antenna_instantiate(node, config_fd);
}


#ifndef WORLDSENS
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_tx(struct _node * node, struct _packet * packet) {
  /* tx -> core */
  flogf2(LEVEL_LAYER, node->addr, packet->id, EVENT_TX, packet->size, ANTENNA_LAYER, KEY_CORE, "");
  core_add_packet(packet);
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void antenna_rx(struct _node *node, struct _packet *packet) {
  /* antenna -> radio */
  flogf2(LEVEL_LAYER, node->addr, packet->id, EVENT_RX, packet->size, ANTENNA_LAYER, KEY_CORE, "");
  radio_rx(node, packet);
	
  return;
}
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double antenna_get_orientation(struct _node *node) {
  return node->antenna_public.orientation; 
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void antenna_set_orientation(struct _node *node, double degree) {
  node->antenna_public.orientation = degree;
  return;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_get_ioctl(struct _node *node, int ioctl, void *arg) {
  return node->antenna->antenna_get_ioctl(node, ioctl, arg);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_set_ioctl(struct _node *node, int ioctl, void *arg) {
  return node->antenna->antenna_set_ioctl(node, ioctl, arg);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double antenna_get_noise_mW(struct _node *node) {
  return node->antenna_public.noise_mW;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double antenna_get_min_rx_mW(struct _node *node) {
  return node->antenna_public.sensibility;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void antenna_complete(struct _node *node) {
  node->antenna->antenna_complete(node);	
  return;
}




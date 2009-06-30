/*
 *  packets.c
 *  
 *
 *  Created by Guillaume Chelius on 08/08/05.
 *  Copyright 2005 __WorldSens_. All rights reserved.
 *
 */
#include <private/packets_private.h>
#include <public/types.h>

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/* "In the air tonight" packets list */
struct _packet *g_packets = NULL;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _packet *packet_create(struct _node *node, int size) {
  static int id = 0;
  struct _packet *packet;
	
  if ((packet = (struct _packet *) malloc(sizeof(struct _packet) + size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    return NULL;
  }
  memset(packet, 0, sizeof(struct _packet) + size);

  if ((packet->SiNR = (double *) malloc(sizeof(double)*size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    free(packet);
    return NULL;
  }
  if ((packet->BER = (double *) malloc(sizeof(double)*size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    free(packet->SiNR);
    free(packet);
    return NULL;
  }
	
  memset(packet->BER, 0, sizeof(double)*size);
  memset(packet->SiNR, 0, sizeof(double)*size);
  packet->data = ((char *) packet) + sizeof(struct _packet);
  packet->id = id++;
  packet->size = size;
  packet->node = node;
	
  //  mobility_update(node);
  packet->x = node->x;
  packet->y = node->y;
  packet->z = node->z;
	
  return packet;	
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void packet_destroy(struct _packet *packet) {	
  free(packet->BER);
  free(packet->SiNR);
  free(packet);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _packet *packet_duplicate(struct _packet *packet) {
  struct _packet *dup;
	
  if ((dup = (struct _packet *) malloc(sizeof(struct _packet) + packet->size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    return NULL;
  }
  memcpy(dup, packet, sizeof(struct _packet) + packet->size);  
  dup->data = ((char *) dup) + sizeof(struct _packet);

  if ((dup->SiNR = (double *) malloc(sizeof(double)*dup->size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    free(dup);
    return NULL;
  }
  if ((dup->BER = (double *) malloc(sizeof(double)*dup->size)) == NULL) {
    fprintf(stderr, "malloc error\n");
    free(dup->SiNR);
    free(dup);
    return NULL;
  }
	
  memset(dup->BER, 0, sizeof(double)*dup->size);
  memset(dup->SiNR, 0, sizeof(double)*dup->size);
  return dup;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

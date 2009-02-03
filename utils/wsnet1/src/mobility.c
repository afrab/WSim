/*
 *  mobility.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/mobility_private.h>

#include <public/log.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int mobility_instantiate(struct _node *node, char *key, FILE *config_fd) {
  struct _model_mobility *loop = m_mobility;

  while (loop && strcmp(loop->key, key)) {
    loop = loop->next;
  }
	
  if (loop == NULL) {
    fprintf(stderr, "Configuration error: mobility model not found for node %d\n", node->addr);
    return -1;
  }
	
  node->mobility = loop;
  memset(node->mobility_private, 0, MOBILITY_PRIVATE_SIZE);
  return node->mobility->mobility_instantiate(node, config_fd);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int mobility_update(struct _node *node) {
  int retval;

  retval = node->mobility->mobility_update(node);
  return retval;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define UNUSED __attribute__((unused))  
void mobility_complete(struct _node UNUSED *node) {
  return;
}

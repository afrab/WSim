/*
 *  interference.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/interference_private.h>

#include <public/log.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _model_interference *g_interference = NULL;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int interference_instantiate(char *key, FILE *config_fd) {
  struct _model_interference *loop =	m_interference;

  while (loop && strcmp(loop->key, key)) {
    loop = loop->next;
  }
	
  if (loop == NULL) {
    fprintf(stderr, "Configuration error: interference model not found\n");
    return -1;
  }
	
  g_interference = loop;

  return g_interference->interference_instantiate(config_fd);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double interference_correlation(double mW, int radio0, int radio1) {
  return g_interference->interference_correlation(mW, radio0, radio1);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int interference_complete(void) {
  return g_interference->interference_complete();
}

/*
 *  modulation.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/modulation_private.h>

#include <public/log.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _model_modulation *g_modulation = NULL;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int modulation_instantiate(char *key, FILE *config_fd) {
  struct _model_modulation *loop = m_modulation;
	
  while (loop && strcmp(loop->key, key)) {
    loop = loop->next;
  }
	
  if (loop == NULL) {
    fprintf(stderr, "Configuration error: modulation model not found\n");
    return -1;
  }
	
  g_modulation = loop;
	
  return g_modulation->modulation_instantiate(config_fd);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
double modulation_compute_BER(double SiNR) {
  if (SiNR == 0.0)
    return 0.0;
	
  return g_modulation->modulation_compute_BER(SiNR);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int modulation_complete(void)  {
  return g_modulation->modulation_complete();
}

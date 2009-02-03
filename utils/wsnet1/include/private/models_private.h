/*
 *  models_private.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _MODELS_PRIVATE_H
#define _MODELS_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/models.h>
#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _model_propagation {
  char key [MODEL_NAME_LEN]; 
  int (* propagation_instantiate) (FILE * config_fd);
  double (* propagation_propagation) (double mW, double x0, double y0, double z0, double x1, double y1, double z1);
  int (* propagation_complete) (void);

  struct _model_propagation * next; 
};

struct _model_interference {
  char key [MODEL_NAME_LEN]; 
  int (* interference_instantiate) (FILE * config_fd);
  double (* interference_correlation) (double mW, int radio0, int radio1);
  int (* interference_complete)       (void);

  struct _model_interference * next; 
};

struct _model_modulation {
  char key [MODEL_NAME_LEN]; 
  int (* modulation_instantiate) (FILE * config_fd);
  double (* modulation_compute_BER) (double SiNR);
  int (* modulation_complete) (void);

  struct _model_modulation * next; 
};

struct _model_antenna {
  char key [MODEL_NAME_LEN]; 
  int (* antenna_instantiate) (struct _node * node, FILE * config_fd);
  double (* antenna_compute_tx) (struct _node * node, double mW, double x, double y, double z);
  double (* antenna_compute_rx) (struct _node * node, double mW, double x, double y, double z);
  int (* antenna_get_ioctl) (struct _node * node, int ioctl, void * val);
  int (* antenna_set_ioctl) (struct _node * node, int ioctl, void * val);
  int (* antenna_complete) (struct _node * node);

  struct _model_antenna * next; 
};

struct _model_radio {
  char key [MODEL_NAME_LEN]; 
  int (* radio_instantiate) (struct _node * node, FILE * config_fd);
  void (* radio_on) (struct _node * node);
  void (* radio_off) (struct _node * node);
  int (* radio_tx_begin) (struct _node * node, struct _packet * packet);
  int (* radio_rx_end) (struct _node * node, struct _packet * packet);
  void (* radio_carrier_sense) (struct _node * node, int p_id, double rx_mW,  double SiNR, int radio);
  int (* radio_get_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* radio_set_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* radio_complete) (struct _node * node);

  struct _model_radio * next; 
};

struct _model_mac {
  char key [MODEL_NAME_LEN]; 
  int (* mac_instantiate) (struct _node * node, FILE * config_fd);
  int (* mac_tx) (struct _node * node);
  int (* mac_rx) (struct _node * node, struct _packet * packet);
  int (* mac_get_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* mac_set_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* mac_complete) (struct _node * node);

  struct _model_mac * next; 
};

struct _model_queue {
  char key [MODEL_NAME_LEN];
  int (* queue_instantiate) (struct _node * node, FILE * config_fd);
  int (* queue_tx) (struct _node * node, char * data, int size, int dst, int priority);
  struct _txbuf * (* queue_get) (struct _node * node);
  int (* queue_get_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* queue_set_ioctl) (struct _node * node, int ioctl, void * arg);
  int (* queue_complete) (struct _node * node);
	
  struct _model_queue * next;
};

struct _model_application {
  char key [MODEL_NAME_LEN]; 
  int (* application_instantiate) (struct _node * node, FILE * config_fd);
  int (* application_rx) (struct _node * node, char * data, int size);
  int (* application_complete) (struct _node * node);
	
  struct _model_application * next; 
};

struct _model_mobility {
  char key [MODEL_NAME_LEN]; 
  int (* mobility_instantiate) (struct _node * node, FILE * config_fd);
  int (* mobility_update) (struct _node * node);
  int (* mobility_complete) (struct _node * node);
	
  struct _model_mobility * next; 
};

struct _model_battery {
  char key [MODEL_NAME_LEN]; 
  int (* battery_instantiate) (struct _node *node, FILE *config_fd);	
  int (* battery_complete) (struct _node *node);                        

  struct _model_battery * next;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_clean (void);


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern struct _model_propagation * m_propagation;
extern struct _model_interference * m_interference;
extern struct _model_modulation * m_modulation;
extern struct _model_antenna * m_antenna;
extern struct _model_radio * m_radio;
extern struct _model_mac * m_mac;
extern struct _model_application * m_application;
extern struct _model_mobility * m_mobility;
extern struct _model_queue * m_queue;
extern struct _model_battery * m_battery;


#endif //_MODELS_PRIVATE_H


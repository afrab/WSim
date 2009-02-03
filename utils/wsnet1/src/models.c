/*
 *  models.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <private/models_private.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _model_propagation *m_propagation = NULL;
struct _model_interference *m_interference = NULL;
struct _model_modulation *m_modulation = NULL;
struct _model_antenna *m_antenna = NULL;
struct _model_radio *m_radio = NULL;
struct _model_mac *m_mac = NULL;
struct _model_application *m_application = NULL;
struct _model_mobility *m_mobility = NULL;
struct _model_queue *m_queue = NULL;
struct _model_battery *m_battery = NULL;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_propagation(const char *key, 
			   int (* propagation_instantiate) (FILE *config_fd), 
			   double (* propagation_propagation) (double mW, double x0, double y0, double z0, double x1, double y1, double z1),
			   int (* propagation_complete)	(void)) {
  struct _model_propagation *propagation;
	
  if ((propagation = (struct _model_propagation *) malloc (sizeof(struct _model_propagation))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }

  strcpy(propagation->key, key);
  propagation->propagation_instantiate = propagation_instantiate;
  propagation->propagation_propagation = propagation_propagation;
  propagation->propagation_complete = propagation_complete;
  propagation->next = m_propagation;
  m_propagation =	propagation;

  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_interference(const char *key, 
			    int (* interference_instantiate) (FILE *config_fd), 
			    double (* interference_correlation) (double mW, int radio0, int radio1),
			    int (* interference_complete) (void)) {
  struct _model_interference *interference;
  
  if ((interference = (struct _model_interference *) malloc (sizeof(struct _model_interference))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(interference->key, key);
  interference->interference_instantiate	= interference_instantiate;
  interference->interference_correlation	= interference_correlation;
  interference->interference_complete = interference_complete;
  interference->next = m_interference;
  m_interference = interference;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_modulation(const char *key, 
			  int (* modulation_instantiate) (FILE *config_fd), 
			  double (* modulation_compute_BER) (double SiNR),
			  int (* modulation_complete) (void)) {
  struct _model_modulation *modulation;
	
  if ((modulation = (struct _model_modulation *) malloc (sizeof(struct _model_modulation))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(modulation->key, key);
  modulation->modulation_instantiate = modulation_instantiate;
  modulation->modulation_compute_BER = modulation_compute_BER;
  modulation->modulation_complete = modulation_complete;
  modulation->next = m_modulation;
  m_modulation = modulation;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_antenna(const char *key, 
		       int (* antenna_instantiate) (struct _node *node, FILE *config_fd), 
		       double (* antenna_compute_tx) (struct _node *node, double mW, double x, double y, double z), 
		       double (* antenna_compute_rx) (struct _node *node, double mW, double x, double y, double z),
		       int (* antenna_get_ioctl) (struct _node *node, int ioctl, void *arg),
		       int (* antenna_set_ioctl) (struct _node *node, int ioctl, void *arg),
		       int (* antenna_complete) (struct _node *node)) {
  struct _model_antenna *antenna;
	
  if ((antenna = (struct _model_antenna *) malloc (sizeof(struct _model_antenna))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(antenna->key, key);
  antenna->antenna_instantiate = antenna_instantiate;
  antenna->antenna_compute_tx = antenna_compute_tx;
  antenna->antenna_compute_rx = antenna_compute_rx;
  antenna->antenna_get_ioctl = antenna_get_ioctl;
  antenna->antenna_set_ioctl = antenna_set_ioctl;
  antenna->antenna_complete = antenna_complete;
  antenna->next =	m_antenna;
  m_antenna = antenna;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
int models_add_radio(const char *key, 
		     int (* radio_instantiate) (struct _node *node, FILE *config_fd),
		     void (* radio_on) (struct _node *node),
		     void (* radio_off) (struct _node *node),
		     int (* radio_tx_begin) (struct _node *node, struct _packet *packet),
		     int (* radio_rx_end) (struct _node *node, struct _packet *packet),
		     void (* radio_carrier_sense) (struct _node *node, int p_id, double rx_mW, double SiNR, int radio),
		     int (* radio_get_ioctl) (struct _node *node, int ioctl, void *arg),
		     int (* radio_set_ioctl) (struct _node *node, int ioctl, void *arg),
		     int (* radio_complete) (struct _node *node)) {
  struct _model_radio *radio;
	
  if ((radio = (struct _model_radio *) malloc (sizeof(struct _model_radio))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(radio->key, key);
  radio->radio_instantiate = radio_instantiate;
  radio->radio_on = radio_on;
  radio->radio_off = radio_off;
  radio->radio_tx_begin = radio_tx_begin;
  radio->radio_rx_end = radio_rx_end;
  radio->radio_carrier_sense = radio_carrier_sense;
  radio->radio_get_ioctl = radio_get_ioctl;
  radio->radio_set_ioctl = radio_set_ioctl;
  radio->radio_complete = radio_complete;
  radio->next = m_radio;
  m_radio = radio;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_mac(const char *key, 
		   int (* mac_instantiate) (struct _node *node, FILE *config_fd), 
		   int (* mac_tx) (struct _node *node),
		   int (* mac_rx) (struct _node *node, struct _packet *packet),
		   int (* mac_get_ioctl) (struct _node *node, int ioctl, void *arg),
		   int (* mac_set_ioctl) (struct _node *node, int ioctl, void *arg),
		   int (* mac_complete)	(struct _node *node)) {
  struct _model_mac *mac;
	
  if ((mac = (struct _model_mac *) malloc (sizeof(struct _model_mac))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(mac->key, key);
  mac->mac_instantiate = mac_instantiate;
  mac->mac_tx = mac_tx;
  mac->mac_rx = mac_rx;
  mac->mac_get_ioctl = mac_get_ioctl;
  mac->mac_set_ioctl = mac_set_ioctl;
  mac->mac_complete = mac_complete;
  mac->next = m_mac;
  m_mac =	mac;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_queue(const char *key, 
		     int (* queue_instantiate) (struct _node *node, FILE *config_fd), 
		     int (* queue_tx) (struct _node *node, char *data, int size, int dst, int priority),
		     struct _txbuf *(* queue_get) (struct _node *node),
		     int (* queue_get_ioctl) (struct _node *node, int ioctl, void *arg),
		     int (* queue_set_ioctl) (struct _node *node, int ioctl, void *arg),
		     int(* queue_complete) (struct _node *node)) {
  struct _model_queue *queue;
	
  if ((queue = (struct _model_queue *) malloc (sizeof(struct _model_queue))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(queue->key, key);
  queue->queue_instantiate = queue_instantiate;
  queue->queue_tx = queue_tx;
  queue->queue_get = queue_get;
  queue->queue_get_ioctl = queue_get_ioctl;
  queue->queue_set_ioctl = queue_set_ioctl;
  queue->queue_complete = queue_complete;
  queue->next = m_queue;
  m_queue = queue;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_application(const char *key, 
			   int (* application_instantiate) (struct _node *node, FILE *config_fd), 
			   int (* application_rx) (struct _node *node, char *data, int size),
			   int (* application_complete)	(struct _node *node)) {
  struct _model_application *application;
	
  if ((application = (struct _model_application *) malloc (sizeof(struct _model_application))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(application->key, key);
  application->application_instantiate = application_instantiate;
  application->application_rx = application_rx;
  application->application_complete = application_complete;
  application->next = m_application;
  m_application =	application;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_mobility (const char *key, 
			 int (* mobility_instantiate) (struct _node *node, FILE *config_fd), 
			 int (* mobility_update) (struct _node *node),
			 int (* mobility_complete) (struct _node *node)) {
  struct _model_mobility *mobility;
	
  if ((mobility = (struct _model_mobility *) malloc (sizeof(struct _model_mobility))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(mobility->key, key);
  mobility->mobility_instantiate = mobility_instantiate;
  mobility->mobility_update = mobility_update;
  mobility->mobility_complete = mobility_complete;
  mobility->next = m_mobility;
  m_mobility = mobility;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_battery (const char * key, 
			int (* battery_instantiate) (struct _node *node, FILE *config_fd), 
			int (* battery_complete) (struct _node *node))
{
  struct _model_battery *battery;
	
  if ((battery = (struct _model_battery *) malloc (sizeof(struct _model_battery))) == NULL) {
    fprintf(stderr, "malloc error\n");
    exit (-1);
  }
	
  strcpy(battery->key, key);
  battery->battery_instantiate = battery_instantiate;
  battery->battery_complete = battery_complete;
  battery->next = m_battery;
  m_battery = battery;
	
  return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_clean (void) {
  struct _model_propagation *propagation = NULL;
  struct _model_interference *interference = NULL;
  struct _model_modulation *modulation = NULL;
  struct _model_antenna *antenna = NULL;
  struct _model_radio *radio = NULL;
  struct _model_mac *mac = NULL;
  struct _model_application *application = NULL;
  struct _model_mobility *mobility = NULL;
  struct _model_battery *battery = NULL;
  struct _model_queue *queue = NULL;
	
  while (m_propagation) {
    propagation = m_propagation;
    m_propagation = m_propagation->next;
    free(propagation);
  }

  while (m_interference) {
    interference = m_interference;
    m_interference = m_interference->next;
    free(interference);
  }

  while (m_modulation) {
    modulation = m_modulation;
    m_modulation = m_modulation->next;
    free(modulation);
  }

  while (m_antenna) {
    antenna = m_antenna;
    m_antenna = m_antenna->next;
    free(antenna);
  }

  while (m_radio) {
    radio = m_radio;
    m_radio = m_radio->next;
    free(radio);
  }

  while (m_mac) {
    mac = m_mac;
    m_mac = m_mac->next;
    free(mac);
  }
	
  while (m_application) {
    application = m_application;
    m_application = m_application->next;
    free(application);
  }

  while (m_mobility) {
    mobility = m_mobility;
    m_mobility = m_mobility->next;
    free(mobility);
  }

  while (m_queue) {
    queue = m_queue;
    m_queue = m_queue->next;
    free(queue);
  }

  while (m_battery) {
    battery = m_battery;
    m_battery = m_battery->next;
    free(battery);
  }

  return 0;
}

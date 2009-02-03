/*
 *  models.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _MODELS_H
#define _MODELS_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define MODEL_NAME_LEN 36


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int models_add_propagation(const char * key, 
			   int (* propagation_instantiate) (FILE * config_fd), 
			   double (* propagation_propagation) (double mW, double x0, double y0, double z0, double x1, double y1, double z1),
			   int (* propagation_complete)	(void));

int models_add_interference(const char * key, 
			    int (* interference_instantiate) (FILE * config_fd), 
			    double (* interference_correlation) (double mW, int radio0, int radio1),
			    int (* interference_complete) (void));

int models_add_modulation(const char * key, 
			  int (* modulation_instantiate) (FILE * config_fd), 
			  double (* modulation_compute_BER) (double SiNR),
			  int (* modulation_complete) (void));

int models_add_antenna(const char * key, 
		       int (* antenna_instantiate) (struct _node * node, FILE * config_fd), 
		       double (* antenna_compute_tx) (struct _node * node, double mW, double x, double y, double z), 
		       double (* antenna_compute_rx) (struct _node * node, double mW, double x, double y, double z),
		       int (* antenna_get_ioctl) (struct _node * node, int ioctl, void * arg),
		       int (* antenna_set_ioctl) (struct _node * node, int ioctl, void * arg),
		       int (* antenna_complete) (struct _node * node));

int models_add_radio(const char * key, 
		     int (* radio_instantiate) (struct _node * node, FILE * config_fd),
		     void (* radio_on) (struct _node * node),
		     void (* radio_off) (struct _node * node),
		     int (* radio_tx_begin) (struct _node * node, struct _packet * packet),
		     int (* radio_rx_end) (struct _node * node, struct _packet * packet),
		     void (* radio_carrier_sense) (struct _node * node, int p_id, double rx_mW, double SiNR, int radio),
		     int (* radio_get_ioctl) (struct _node * node, int ioctl, void * arg),
		     int (* radio_set_ioctl) (struct _node * node, int ioctl, void * arg),
		     int (* radio_complete) (struct _node * node));

int models_add_mac(const char * key, 
		   int (*  mac_instantiate) (struct _node * node, FILE *  config_fd), 
		   int (* mac_tx) (struct _node * node),
		   int (* mac_rx) (struct _node * node, struct _packet * packet),
		   int (* mac_get_ioctl) (struct _node * node, int ioctl, void * arg),
		   int (* mac_set_ioctl) (struct _node * node, int ioctl, void * arg),
		   int (* mac_complete)	(struct _node * node));

int models_add_queue(const char * key, 
		     int (* queue_instantiate) (struct _node * node, FILE * config_fd), 
		     int (* queue_tx) (struct _node * node, char * data, int size, int dst, int priority),
		     struct _txbuf * (* queue_get) (struct _node * node),
		     int (* queue_get_ioctl) (struct _node * node, int ioctl, void * arg),
		     int (* queue_set_ioctl) (struct _node * node, int ioctl, void * arg),
		     int(* queue_complete) (struct _node * node));

int models_add_application(const char * key, 
			   int (* application_instantiate) (struct _node * node, FILE * config_fd), 
			   int (* application_rx) (struct _node * node, char * data, int size),
			   int (* application_complete)	(struct _node * node));

int models_add_mobility (const char * key, 
			 int (* mobility_instantiate) (struct _node * node, FILE * config_fd), 
			 int (* mobility_update) (struct _node * node),
			 int (* mobility_complete) (struct _node * node));

int models_add_battery (const char * key, 
			int (* battery_instantiate) (struct _node *node, FILE *config_fd),
			int (* battery_complete) (struct _node *node));

#endif //_MODELS_H

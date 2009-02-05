/*
 *  parse.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <time.h>

#include <private/parse.h>
#include <private/nodes_private.h>
#include <private/command_line.h>
#include <private/propagation_private.h>
#include <private/interference_private.h>
#include <private/modulation_private.h>
#include <private/simulation_private.h>
#include <private/mobility_private.h>
#include <private/antenna_private.h>
#include <private/radio_private.h>
#include <private/mac_private.h>
#include <private/application_private.h>
#include <private/queue_private.h>
#include <private/core_private.h>
#include <private/battery_private.h>

#include <public/simulation.h>
#include <public/types.h>

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int parse_config(void) 
{
  FILE *config_fd;
  char config_file[256];
  char arg_str[256];
  char c;
  int loop;
  long int f_pos = 0;
  int repeat_conf_type = 0;
  int repeat_conf_counter = 0;
#ifndef WORLDSENS
  char mylabel[30];
#endif //WORLDSENS
	
  /* Initialize seed value */
  if (g_seed == -1) 
    {
      struct timeval tp;
      gettimeofday(&tp, NULL);
      srand48(tp.tv_sec + tp.tv_usec);
    } 
  else 
    {
      srand48(g_seed);
    }
	
	
  /******************** Simulation.inst ************************/
  /* Open simulation.inst */
  sprintf(config_file, "%s/simulation.inst", g_config_repository);

  if ((config_fd = fopen(config_file, "r")) == NULL) 
    {
      perror("Unable to open \"simulation.inst\":");
      return -1;
    }
	
#ifndef WORLDSENS
  /* Get informations*/
  if (fscanf(config_fd, "%d %lf %lf %lf %d %" PRId64 "", 
	     &g_m_nodes, &g_x, &g_y, &g_z, &g_error, &g_simend) != 6) 
    {
      fprintf(stderr, "Configuration error: Unable to read \"simulation.inst\"\n");
      fclose(config_fd);
      return -1;
    }
#else //WORLDSENS	
  /* Get informations*/
  if (fscanf(config_fd, "%d %lf %lf %lf", 
	     &g_m_nodes, &g_x, &g_y, &g_z) != 4) 
    {
      fprintf(stderr, "Configuration error: Unable to read \"simulation.inst\"\n");
      fclose(config_fd);
      return -1;
    }
#endif //WORLDSENS
	
  if (g_m_nodes <= 0) 
    {
      fprintf(stderr, "Configuration error: maximum number of nodes <= 0\n");	
      fclose(config_fd);
      return -1;
    }
	
  if (g_x < 0) 
    {
      fprintf(stderr, "Configuration error: testbed x length <= 0\n");	
      fclose(config_fd);
      return -1;
    }
	
  if (g_y < 0) 
    {
      fprintf(stderr, "Configuration error: testbed y length <= 0\n");	
      fclose(config_fd);
      return -1;
    }

  if (g_z < 0) 
    {
      fprintf(stderr, "Configuration error: testbed z length <= 0\n");	
      fclose(config_fd);
      return -1;
    }

#ifndef WORLDSENS
  if (g_simend == 0) 
    {
      fprintf(stderr, "Configuration error: simulation duration <= 0\n");	
      fclose(config_fd);
      return -1;
    }
#endif //WORLDSENS
	

  /* Allocate node space */
  if ((g_nodes = (struct _node *) malloc(g_m_nodes * sizeof(struct _node))) == NULL) {
    fprintf(stderr, "malloc error\n");
    fclose(config_fd);
    return -1;
  }
  memset(g_nodes, 0, sizeof(sizeof(struct _node) * g_m_nodes));
	
  /* For each node */
  loop = g_m_nodes;
  while (loop--) 
    {
      struct _node *node = &(g_nodes[loop]);
      
      node->addr = loop;
    }		
	
  fclose(config_fd);

	
  /************* propagation.inst ***********************/
  /* Open propagation.inst */
  config_fd = NULL;
  sprintf(config_file, "%s/propagation.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL) {
    perror("Unable to open \"propagation.inst\":");
    return -1;
  }
	
  /* Get informations*/
  if (fscanf(config_fd, "%s", arg_str) != 1) {
    fprintf(stderr, "Unable to read \"propagation.inst\"\n");
    fclose(config_fd);
    return -1;
  }
	
  if (propagation_instantiate(arg_str, config_fd)) {
    fclose(config_fd);
    return -1;
  }
	
  fclose(config_fd);
	
	

  /************* interference.inst ***********************/
  /* Open interference.inst */
  config_fd = NULL;
  sprintf(config_file, "%s/interference.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL) {
    perror("Unable to open \"interference.inst\":");
    return -1;
  }
	
  /* Get informations*/
  if (fscanf(config_fd, "%s", arg_str) != 1) {
    fprintf(stderr, "Unable to read \"interference.inst\"\n");
    fclose(config_fd);
    return -1;
  }
	
  if (interference_instantiate(arg_str, config_fd)) {
    fclose(config_fd);
    return -1;
  }
	
  fclose(config_fd);
	
	
  /************* modulation.inst ***********************/
  /* Open modulation.inst */
  config_fd = NULL;
  sprintf(config_file, "%s/modulation.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL) {
    perror("Unable to open \"modulation.inst\":");
    return -1;
  }
	
  /* Get informations*/
  if (fscanf(config_fd, "%s", arg_str) != 1) {
    fprintf(stderr, "Unable to read \"modulation.inst\"\n");
    fclose(config_fd);
    return -1;
  }
	
  if (modulation_instantiate(arg_str, config_fd)) {
    fclose(config_fd);
    return -1;
  }
	
  fclose(config_fd);	

	
  /********************************************** mobility.inst *******************************************/
  /* Open mobility.inst */
  config_fd = NULL;
  sprintf(config_file, "%s/mobility.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"mobility.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
		
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"mobility.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }

    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }
		
    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    }else{
      if (mobility_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 

	break;
      default:
	break;
      }
    }
  }
  fclose(config_fd);
	

  /********************************************** antemna.inst *******************************************/
  /* Open antemna.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/antemna.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
  {
      perror("Unable to open \"antemna.inst\":");
      return -1;
  }
  
  /* For each node, */
  loop = g_m_nodes;
  
  while (loop--) {
	  struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
	  
	  /* Get informations*/
	  if (fscanf(config_fd, "%s", arg_str) != 1)
      {
		  fprintf(stderr, "Unable to read \"antemna.inst\" for node %d\n", g_m_nodes - 1 - loop);
		  fclose(config_fd);
		  return -1;
      }
	  
	  if (arg_str[0] == '#') {
		  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
		  f_pos = ftell(config_fd);
		  loop ++;
		  continue;
	  }
	  
	  if(strcmp("ALL",arg_str)==0){
		  f_pos = ftell(config_fd);
		  repeat_conf_type = 1;
		  loop++;
	  }else if(strcmp("REPEAT",arg_str)==0){
		  if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
			  if(repeat_conf_counter>=1){
				  repeat_conf_type = 2;
				  f_pos = ftell(config_fd);
				  loop++;
			  }
			  if (repeat_conf_counter == 0) {
				  /* if repeat is 0, skip the line */
				  fscanf(config_fd, "%c", &c);
				  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
				  f_pos = ftell(config_fd);
				  loop ++;
			  }
		  }
	  }else{
		  if (antenna_instantiate(node, arg_str, config_fd)) {
			  fclose(config_fd);
			  return -1;
		  }
		  switch(repeat_conf_type){
			  case 1:
				  fseek(config_fd,f_pos,SEEK_SET);
				  break;
			  case 2:
				  repeat_conf_counter--;
				  if(repeat_conf_counter>0)  
					  fseek(config_fd,f_pos,SEEK_SET);
				  else repeat_conf_type=0; 	
				  break;
			  default:
				  break;
		  }
	  }
  }
  fclose(config_fd);
  
  
#ifndef WORLDSENS
  /********************************************** battery.inst *******************************************/
  /* Open battery.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/battery.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"battery.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
		
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"battery.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }
		

    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }

    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    }else{
      if (battery_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 
	break;
      default:
	break;
      }
    }
	
  }
  fclose(config_fd);


  /********************************************** radio.inst *******************************************/
  /* Open radio.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/radio.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"radio.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
		
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"radio.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }


    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }

    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    }else{
      if (radio_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 
	break;
      default:
	break;
      }
    }
	
  }
  fclose(config_fd);
	
  /********************************************** mac.inst *******************************************/
  /* Open mac.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/mac.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"mac.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
		
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"mac.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }
		

    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }

    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    }else{
      if (mac_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 
	break;
      default:
	break;
      }
    }
	
  }
  fclose(config_fd);

  /********************************************** queue.inst ****************************************/
  /* Open queue.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/queue.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"queue.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
		
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"queue.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }
		

    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }

    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    }else{
      if (queue_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 
	break;
      default:
	break;
      }
    }
	
  }
  fclose(config_fd);

	
  /********************************************** application.inst *******************************************/
  /* Open application.inst */
  config_fd = NULL;
  repeat_conf_type = 0;
  sprintf(config_file, "%s/application.inst", g_config_repository);
  if ((config_fd = fopen(config_file, "r")) == NULL)
    {
      perror("Unable to open \"application.inst\":");
      return -1;
    }
	
  /* For each node, */
  loop = g_m_nodes;
	
  while (loop--) {
    struct _node *node = &(g_nodes[g_m_nodes - 1 - loop]);
				
    /* Get informations*/
    if (fscanf(config_fd, "%s", arg_str) != 1)
      {
	fprintf(stderr, "Unable to read \"application.inst\" for node %d\n", g_m_nodes - 1 - loop);
	fclose(config_fd);
	return -1;
      }


    if (arg_str[0] == '#') {
      while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
      f_pos = ftell(config_fd);
      loop ++;
      continue;
    }

    if(strcmp("ALL",arg_str)==0){
      f_pos = ftell(config_fd);
      repeat_conf_type = 1;
      loop++;
    }else if(strcmp("REPEAT",arg_str)==0){
      if(fscanf(config_fd, "%d", &repeat_conf_counter)!=-1){
	if(repeat_conf_counter>=1){
	  repeat_conf_type = 2;
	  f_pos = ftell(config_fd);
	  loop++;
	}
	if (repeat_conf_counter == 0) {
	  /* if repeat is 0, skip the line */
	  fscanf(config_fd, "%c", &c);
	  while ( ( fscanf(config_fd,"%c", &c) > 0) && (c!='\n') ) {}
	  f_pos = ftell(config_fd);
	  loop ++;
	}
      }
    } else {
      if (application_instantiate(node, arg_str, config_fd)) {
	fclose(config_fd);
	return -1;
      }
      switch(repeat_conf_type){
      case 1:
	fseek(config_fd,f_pos,SEEK_SET);
	break;
      case 2:
	repeat_conf_counter--;
	if(repeat_conf_counter>0)  
	  fseek(config_fd,f_pos,SEEK_SET);
	else repeat_conf_type=0; 
	break;
      default:
	break;
      }
    }
	
  }
  fclose(config_fd);

#endif //WORLDSENS

#ifndef WORLDSENS
  fprintf(stdout, "Simulation setup: nodes=%d, field=(%lfm, %lfm, %lfm), error=%d, duration=%" PRId64 "\n", 
	  g_m_nodes, g_x, g_y, g_z, g_error, g_simend);	
#else //WORLDSENS
  fprintf(stdout, "Simulation setup: %d nodes, (%lfm, %lfm, %lfm) field\n", 
	  g_m_nodes, g_x, g_y, g_z);
#endif //WORLDSENS
  return 0;
}

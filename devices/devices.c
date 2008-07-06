
/**
 *  \file   devices.c
 *  \brief  Platform devices and handling functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_delete(void)
{
  int res = 0;
  MAP_OVER_DEVICES(delete);
  free(machine.devices_state);
  free(machine.devices_state_backup);  
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_memory_allocate(void)
{
  int i;
  /* state memory allocation */
  machine.devices_state_size = 0;
  for(i=0; i<machine.device_max; i++)
    {
      machine.devices_state_size += machine.device_size[i];
#if defined(DEBUG_MEMFOOTPRINT)
      HW_DMSG_DEV("Devices: allocating %d bytes for device %d state\n",machine.device_size[i],i); 
#endif
    }

  if ((machine.devices_state        = (unsigned char*)malloc(machine.devices_state_size)) == NULL)
    {
      ERROR("** Could not allocate memory for devices states storage\n");
      return 1;
    }

  if ((machine.devices_state_backup = (unsigned char*)malloc(machine.devices_state_size)) == NULL)
    {
      free(machine.devices_state);
      ERROR("** Could not allocate memory for devices states storage backup\n");
      return 1;
    }

  memset(machine.devices_state,        0, machine.devices_state_size);
  memset(machine.devices_state_backup, 0, machine.devices_state_size);

  /* state memory distribution */
  machine.device[0].data = machine.devices_state;
  for(i=1; i<machine.device_max; i++)
    {
      machine.device[i].data = ((char*) machine.device[i-1].data) +  machine.device_size[i-1];
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_reset(void)
{
  int res = 0;

  /* reset all devices */
  MAP_OVER_DEVICES(reset);

  /* post reset to init interconnect           */
  /* defined in platform dependant description */
  res += devices_reset_post();

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void devices_print(void)
{
  int i;
  OUTPUT("device list:\n");
  for(i=0; i < machine.device_max; i++)
    {
      if (machine.device[i].name != NULL)
	{
	  OUTPUT("  dev %02d : %s\n",i,machine.device[i].name);
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void devices_dump_stats(int64_t user_nanotime)
{
  int i;
  for(i=0; i< machine.device_max; i++)
    {
      if (machine.device[i].name != NULL)
	{
	  OUTPUT("  - %-28s:\n",machine.device[i].name);
	  if (machine.device[i].statdump != NULL)
	    {
	      machine.device[i].statdump(i,user_nanotime);
	    }
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

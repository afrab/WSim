
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
#include "machine/machine.h"
#include "devices/devices.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_delete(void)
{
  int res = 0;
  MAP_OVER_DEVICES(delete);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_memory_allocate(void)
{
  int i;
  int size;
  uint8_t *devzone;
  /* state memory allocation */
  size = 0;
  for(i=0; i<machine.device_max; i++)
    {
      size += machine.device_size[i];
    }

  devzone = machine_state_allocate(size);

  if (devzone == NULL)
    return 1;

  /* state memory distribution */
  machine.device[0].data = devzone;
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
  OUTPUT_BOXM("device list:\n");
  for(i=0; i < machine.device_max; i++)
    {
      if (machine.device[i].name != NULL)
	{
	  OUTPUT_BOXM("  dev %02d : %s\n",i,machine.device[i].name);
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
	  OUTPUT_STATS("  - %-28s:\n",machine.device[i].name);
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

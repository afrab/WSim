/**
 *  \file   libgdb.c
 *  \brief  GDB mode main loop
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include "arch/common/hardware.h"
#include "machine/machine.h"

#include "gdbremote.h"
#include "gdbremote_socket.h"
#include "gdbremote_utils.h"
#include "libgdb.h"

/* ************************************************** */
/* ************************************************** */

int libgdb_target_mode_main(unsigned short port)
{
  int retcode = GDB_CMD_DETACH;
  struct gdbremote_t gdb;


  if (gdbremote_init(&gdb,port))
    {
      return GDB_INIT_ERROR;
    }

  machine_state_save();

  do 
    {
      if (gdbremote_accept(&gdb))
	{
	  ERROR("** GDB accept error\n");
	  break;
	}

      while ((retcode = gdbremote_getcmd(&gdb)) == GDB_CMD_OK) ;

    }
  while (retcode != GDB_CMD_ERROR && retcode != GDB_CMD_KILL); // DETACH left

  switch (retcode)
    {
    case GDB_CMD_ERROR:
      ERROR("Main:GDB getcmd returned with an error\n");
      break;
    case GDB_CMD_KILL:
      ERROR("Main:GDB simulator was killed by remote gdb\n");
      break;
    }

  if (gdbremote_close(&gdb))
    {
      return GDB_CLOSE_ERROR;
    }

  return GDB_OK;
}

/* ************************************************** */
/* ************************************************** */

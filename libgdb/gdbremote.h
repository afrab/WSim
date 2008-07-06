/**
 *  \file   gdbremote.h
 *  \brief  GDB Remote target functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef __GDBREMOTE_H_
#define __GDBREMOTE_H_

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct gdbremote_t {
  char           initialized;
  char           extended_mode;
  int            socket_listen;
  int            socket;
  unsigned short port;

  uint32_t       last_signal;
};

#define GDB_CMD_OK      0
#define GDB_CMD_ERROR   1
#define GDB_CMD_DETACH  2
#define GDB_CMD_KILL    3

/**
 * Read command from remote GDB connexion
 * @return 0 ok, otherwise error
 */
int gdbremote_getcmd(struct gdbremote_t *gdb);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* Serial remote protocol commands / replies  */
#define NO_DEBUG_SRP_PROTO 1
/* GDB detailed debug of commands and actions */
#define NO_DEBUG_GDB_CMD   1
/* verbose levels                             */
#define GDB_VERB_LEVEL  4


#if defined(DEBUG_SRP_PROTO)
#define DMSG_GDB(x...)       VERBOSE(GDB_VERB_LEVEL,x)
#else
#define DMSG_GDB(x...)       do { } while (0)
#endif

#if defined(DEBUG_GDB_CMD)
#define DMSG_GDB_CMD(x...)   VERBOSE(GDB_VERB_LEVEL + 1,x)
#else
#define DMSG_GDB_CMD(x...)   do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

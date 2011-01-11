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
  char                      initialized;
  char                      extended_mode;
  uint32_t                  last_signal;
  struct libselect_socket_t skt;
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

#endif

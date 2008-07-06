/**
 *  \file   gdbremote_socket.h
 *  \brief  GDB Remote functions, socket API
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef __GDBREMOTE_SOCKET_H_
#define __GDBREMOTE_SOCKET_H_

int  gdbremote_init         (struct gdbremote_t *gdb, unsigned short port);
int  gdbremote_accept       (struct gdbremote_t *gdb);
int  gdbremote_close        (struct gdbremote_t *gdb);
int  gdbremote_close_client (struct gdbremote_t *gdb);

char gdbremote_getchar      (struct gdbremote_t *gdb);

#define GDBREMOTE_PUTCHAR_OK    0
#define GDBREMOTE_PUTCHAR_ERROR 1

int  gdbremote_putchar      (struct gdbremote_t *gdb, unsigned char c);

#endif

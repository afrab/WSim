/**
 *  \file   libselect_socket.h
 *  \brief  libselect socket API, socket API
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef __LIBSELECT_SOCKET_H_
#define __LIBSELECT_SOCKET_H_

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct libselect_socket_t {
  int            socket_listen;
  int            socket;
  unsigned short port;
};

#define LIBSELECT_SOCKET_PUTCHAR_OK    0
#define LIBSELECT_SOCKET_PUTCHAR_ERROR 1

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int  libselect_skt_init         (struct libselect_socket_t *s, unsigned short port);
int  libselect_skt_accept       (struct libselect_socket_t *s);
int  libselect_skt_close        (struct libselect_socket_t *s);
int  libselect_skt_close_client (struct libselect_socket_t *s);

char libselect_skt_getchar      (struct libselect_socket_t *s);
int  libselect_skt_putchar      (struct libselect_socket_t *s, unsigned char c);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

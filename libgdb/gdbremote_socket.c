/**
 *  \file   gdbremote_socket.c
 *  \brief  GDB Remote functions, socket API
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#if defined(__MINGW32__)
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h> /* TCP_NODELAY */
  #include <arpa/inet.h>
  #include <netdb.h>
#endif

#include "config.h"
#include "gdbremote.h"
#include "gdbremote_socket.h"
#include "liblogger/logger.h"

#define LISTENQ 2 // socket listen fifo size (backlog)

/* ************************************************** */
/* ************************************************** */

static int 
gdbremote_create_socket(int type, unsigned int addr, unsigned short port)
{
  int yes;
  int desc; 
  int length=sizeof(struct sockaddr_in); 
  struct sockaddr_in address;

  if ((desc = socket(AF_INET,type,0)) == -1)
    {
      perror("could not create socket");
      return -1;
    }
  yes = 1;
  if (setsockopt(desc,SOL_SOCKET,SO_REUSEADDR,(void*)&yes,sizeof(yes)) < 0)
    {
      perror("could not setsockopt");
      close(desc);
      return -2;
    }
  memset(&address,0,length);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(addr);
  address.sin_port = htons(port);
  if (bind(desc,(struct sockaddr*)&address,length) == -1)
    {
      perror("could not bind socket");
      close(desc);
      return -3;
    }
  if (type == SOCK_STREAM)
    {
      if (listen(desc,LISTENQ) == -1)
	{
	  perror("could not listen to socket");
	  close(desc);
	  return -4;
	}
    }
  return desc; 
}

/* ************************************************** */
/* ************************************************** */

static void
gdbremote_log_connection(struct sockaddr_in *cli)
{
#ifdef INET_NTOP
  char buf_svr[120];
  char buf_cli[120];
#endif

  if (1)
    {
      OUTPUT("wsim:gdb: connexion opened ");
#ifdef INET_NTOP
      OUTPUT("from %s:%d\n",
	       inet_ntop(cli->sin_family,(void*)&cli->sin_addr,
			 buf_cli,sizeof(buf_cli)),
	       ntohs(cli->sin_port));
#else
      OUTPUT("from %s:%d\n",
	       inet_ntoa(cli->sin_addr),
	       ntohs(cli->sin_port));
#endif 
    }
}

/* ************************************************** */
/* ************************************************** */

int 
gdbremote_init(struct gdbremote_t *gdb, unsigned short port)
{
  gdb->socket_listen = -1;
  gdb->socket        = -1;
  gdb->port          = port;

  if ((gdb->socket_listen = gdbremote_create_socket(SOCK_STREAM, INADDR_ANY, gdb->port)) < 0)
    return 1;
  
  return 0;
}

/* ************************************************** */
/* ************************************************** */

int 
gdbremote_accept(struct gdbremote_t *gdb)
{
  int yes;
  socklen_t length;
  struct sockaddr_in s;

  length = sizeof(s);
  memset(&s,0,length);
  OUTPUT("wsim:gdb: waiting for a gdb connection on port %d\n",gdb->port);
  if ((gdb->socket = accept(gdb->socket_listen,(struct sockaddr*)&s,&length)) == -1)
    {
      DMSG_GDB("gdbremote: %s", strerror(errno));
      return 1;
    }

  /* TCP keep alive */
  yes = 1;
  setsockopt (gdb->socket,SOL_SOCKET,SO_KEEPALIVE,(void*)&yes,sizeof(yes));

  /* TCP no delay */
#if !defined(PROTO_TCP)
#define PROTO_TCP 6
#endif

  yes = 1;
  setsockopt (gdb->socket,PROTO_TCP,TCP_NODELAY,(void*)&yes,sizeof(yes));

#if defined(SIGPIPE)
  signal (SIGPIPE, SIG_IGN);
#endif

  gdbremote_log_connection(&s);
  return 0;
}

/* ************************************************** */
/* ************************************************** */

int
gdbremote_close_client(struct gdbremote_t *gdb)
{
  if (gdb->socket != -1)
    {
      close(gdb->socket);
      gdb->socket = -1;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */

int 
gdbremote_close(struct gdbremote_t *gdb)
{
  gdbremote_close_client(gdb);

  if (gdb->socket_listen != -1)
    {
      close(gdb->socket_listen);
      gdb->socket_listen = -1;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */

char
gdbremote_getchar(struct gdbremote_t *gdb)
{
  unsigned char c;

  if (gdb->socket == -1)
    {
      ERROR("wsim:gdb:getchar: read on closed socket\n");
      return 0;
    }

  if (read(gdb->socket,&c,1) == -1)
    {
      ERROR("wsim:gdb:getchar: read failed - %s\n",strerror(errno));
      return 0;
    }

  return c;
}

/* ************************************************** */
/* ************************************************** */

int
gdbremote_putchar(struct gdbremote_t *gdb, unsigned char c)
{
  if (gdb->socket == -1)
    {
      ERROR("wsim:gdb:putchar: write on closed socket\n");
      return GDBREMOTE_PUTCHAR_ERROR;
    }

  if (write(gdb->socket,&c,1) < 1)
    {
      ERROR("wsim:gdb:putchar: write failed - %s\n",strerror(errno));
      close(gdb->socket);
      gdb->socket = -1;
      return GDBREMOTE_PUTCHAR_ERROR;
    }

  return GDBREMOTE_PUTCHAR_OK;
}

/* ************************************************** */
/* ************************************************** */

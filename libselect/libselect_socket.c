/**
 *  \file   libselect_socket.c
 *  \brief  libselect socket API, socket API
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
#include "libselect.h"
#include "libselect_socket.h"
#include "liblogger/logger.h"

#define LISTENQ 2 // socket listen fifo size (backlog)

#define DMSG_LIBSELECT_SOCKET(x...) VERBOSE(5,x)
/* ************************************************** */
/* ************************************************** */

static int 
libselect_create_socket(int type, unsigned int addr, unsigned short port)
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
libselect_log_connection(struct sockaddr_in *cli)
{
#ifdef INET_NTOP
  char buf_svr[120];
  char buf_cli[120];
#endif

  if (1)
    {
      OUTPUT("wsim:libselect_socket: connexion opened ");
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
libselect_skt_init(struct libselect_socket_t *skt, unsigned short port)
{
  skt->socket_listen = -1;
  skt->socket        = -1;
  skt->port          = port;

  if ((skt->socket_listen = libselect_create_socket(SOCK_STREAM, INADDR_ANY, skt->port)) < 0)
    return 1;
  
  return 0;
}

/* ************************************************** */
/* ************************************************** */

int 
libselect_skt_accept(struct libselect_socket_t *skt)
{
  int yes;
  socklen_t length;
  struct sockaddr_in s;

  length = sizeof(s);
  memset(&s,0,length);
  OUTPUT("wsim:libselect_socket: waiting for a libselect_socket connection on port %d\n", skt->port);
  if ((skt->socket = accept(skt->socket_listen,(struct sockaddr*)&s,&length)) == -1)
    {
      DMSG_LIBSELECT_SOCKET("libselect:accept %s", strerror(errno));
      return 1;
    }

  /* TCP keep alive */
  yes = 1;
  setsockopt (skt->socket,SOL_SOCKET,SO_KEEPALIVE,(void*)&yes,sizeof(yes));

  /* TCP no delay */
#if !defined(PROTO_TCP)
#define PROTO_TCP 6
#endif

  yes = 1;
  setsockopt (skt->socket,PROTO_TCP,TCP_NODELAY,(void*)&yes,sizeof(yes));

#if defined(SIGPIPE)
  signal (SIGPIPE, SIG_IGN);
#endif

  libselect_log_connection(&s);
  return 0;
}

/* ************************************************** */
/* ************************************************** */

int
libselect_skt_close_client(struct libselect_socket_t *skt)
{
  if (skt->socket != -1)
    {
      close(skt->socket);
      skt->socket = -1;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */

int 
libselect_skt_close(struct libselect_socket_t *skt)
{
  libselect_skt_close_client(skt);

  if (skt->socket_listen != -1)
    {
      close(skt->socket_listen);
      skt->socket_listen = -1;
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */

char
libselect_skt_getchar(struct libselect_socket_t *skt)
{
  unsigned char c;

  if (skt->socket == -1)
    {
      ERROR("wsim:libselect_socket:getchar: read on closed socket\n");
      return 0;
    }

  if (read(skt->socket,&c,1) == -1)
    {
      ERROR("wsim:libselect_socket:getchar: read failed - %s\n",strerror(errno));
      return 0;
    }

  return c;
}

/* ************************************************** */
/* ************************************************** */

int
libselect_skt_putchar(struct libselect_socket_t *skt, unsigned char c)
{
  if (skt->socket == -1)
    {
      ERROR("wsim:libselect_socket:putchar: write on closed socket\n");
      return LIBSELECT_SOCKET_PUTCHAR_ERROR;
    }

  if (write(skt->socket,&c,1) < 1)
    {
      ERROR("wsim:libselect_socket:putchar: write failed - %s\n",strerror(errno));
      close(skt->socket);
      skt->socket = -1;
      return LIBSELECT_SOCKET_PUTCHAR_ERROR;
    }

  return LIBSELECT_SOCKET_PUTCHAR_OK;
}

/* ************************************************** */
/* ************************************************** */

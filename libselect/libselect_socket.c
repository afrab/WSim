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
#include <stdlib.h>
#include <ctype.h>

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

#define DMSG_SKT(x...) VERBOSE(4,x)

/* ************************************************** */
/* ************************************************** */

static int 
libselect_create_socket(int type, char *hostname, unsigned short port)
{
  int yes;
  int desc; 
  int length=sizeof(struct sockaddr_in); 
  struct sockaddr_in address;
  struct hostent *hp;

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

  if ((hp = gethostbyname((hostname))) == NULL)
    {
      ERROR("wsim:libselect_socket:init unknown machine %s\n",hostname); 
      return -1;
    }
  
  memset(&address,0,length);
  address.sin_family = AF_INET;
  memcpy(&(address.sin_addr.s_addr),hp->h_addr,hp->h_length); 
  address.sin_port = htons(port);

  /* DMSG_SKT("wsim:libselect_socket:create:bind 0x%08x\n",address.sin_addr.s_addr); */

  if (bind(desc,(struct sockaddr*)&address,length) == -1)
    {
      perror("could not bind socket");
      close(desc);
      return -3;
    }

  if (port != 0)
    {
      if (type == SOCK_STREAM)
	{
	  if (listen(desc,LISTENQ) == -1)
	    {
	      perror("could not listen to socket");
	      close(desc);
	      return -4;
	    }
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
      VERBOSE(2,"wsim:libselect_socket: connexion opened ");
#ifdef INET_NTOP
      VERBOSE(2,"from %s:%d\n",
	       inet_ntop(cli->sin_family,(void*)&cli->sin_addr,
			 buf_cli,sizeof(buf_cli)),
	       ntohs(cli->sin_port));
#else
      VERBOSE(2,"from %s:%d\n",
	       inet_ntoa(cli->sin_addr),
	       ntohs(cli->sin_port));
#endif 
    }
}

/* ************************************************** */
/* ************************************************** */
#define MAX_URL      200
#define MAX_HOSTNAME 50
#define MAX_PORT     10

int 
libselect_skt_init(struct libselect_socket_t *skt, char *name_org)
{
  char *tok;
  char delim[] = ":";
  char name[MAX_URL];

  strncpy(name,name_org,MAX_URL);
  skt->socket_listen = -1;
  skt->socket        = -1;
  skt->port          =  0;

  DMSG_SKT("wsim:libselect_socket:init: %s\n",name);

  /* tcp:s:machine:port         */
  /* tcp:c:machine:port         */
  tok = strtok(name,delim);
  if (strcmp(tok,"tcp") == 0)
    {
      tok = strtok(NULL,delim);
      switch (tok[0])
	{
	case 's':
	  {
	    char hostname[MAX_HOSTNAME];
	    char port[MAX_PORT];

	    strncpy(hostname, strtok(NULL,delim), MAX_HOSTNAME);
	    strncpy(port    , strtok(NULL,delim), MAX_PORT);
	    skt->port = atoi(port);

	    DMSG_SKT("wsim:libselect_socket:init: tcp server creation for %s:%d\n",hostname,atoi(port));
	    if ((skt->socket_listen = libselect_create_socket(SOCK_STREAM, hostname, skt->port)) < 0)
	      {
		ERROR("wsim:libselect_socket:init create TCP srv not possible\n");
		return -1;
	      }
	  }
	  break;
	case 'c':
	  {
	    struct sockaddr_in addr; 
	    int lg=sizeof(addr);
	    struct hostent *hp;

	    char hostname[MAX_HOSTNAME];
	    char port[MAX_PORT];
	    strncpy(hostname, strtok(NULL,delim), MAX_HOSTNAME);
	    strncpy(port    , strtok(NULL,delim), MAX_PORT);
	    skt->port = atoi(port);

	    if ((skt->socket = libselect_create_socket(SOCK_STREAM, "0.0.0.0", 0)) < 0)
	      {
		ERROR("wsim:libselect_socket:init create TCP clt not possible\n");
		return -1;
	      }

	    if ((hp = gethostbyname((hostname))) == NULL)
	      {
		ERROR("wsim:libselect_socket:init unknown machine %s\n",hostname); 
		return -1;
	      }

	    addr.sin_family=AF_INET; 
	    addr.sin_port=htons(skt->port);
	    memcpy(&(addr.sin_addr.s_addr),hp->h_addr,hp->h_length); 
	    if (connect(skt->socket,(struct sockaddr*) &addr, lg) == -1)
	      {
		ERROR("wsim:libselect_socket:connect error on %s:%d\n",hostname,skt->port);
		return -1;
	      } 
	  }
	  break;
	default:
	  DMSG_SKT("wsim:libselect_socket:init syntax error on tcp socket %s\n",name);
	  return 1;
	}
    }

  /* udp:local:port:remote:port */
  else if (strcmp(tok,"udp") == 0)
    {
      char local_host[MAX_HOSTNAME];
      char local_port[MAX_PORT];
      char remot_host[MAX_HOSTNAME];
      char remot_port[MAX_PORT];
      struct sockaddr_in addr; 
      int lg=sizeof(addr);
      struct hostent *hp;

      strncpy(local_host, strtok(NULL,delim), MAX_HOSTNAME);
      strncpy(local_port, strtok(NULL,delim), MAX_PORT);
      strncpy(remot_host, strtok(NULL,delim), MAX_HOSTNAME);
      strncpy(remot_port, strtok(NULL,delim), MAX_PORT);
      skt->port = atoi(local_port);

      DMSG_SKT("wsim:libselect_socket:init: udp link creation for %s:%d to %s:%d\n",
	       local_host,atoi(local_port),remot_host,atoi(remot_port));
      if ((skt->socket = libselect_create_socket(SOCK_DGRAM, local_host, skt->port)) < 0)
	{
	  ERROR("wsim:libselect_socket:init create UDP socket not possible\n");
	  return -1;
	}
      if ((hp = gethostbyname((remot_host))) == NULL)
	{
	  ERROR("wsim:libselect_socket:init unknown machine %s\n",remot_host);
	  return -1;
	}
      addr.sin_family=AF_INET; 
      addr.sin_port=htons(atoi(remot_port));
      memcpy(&(addr.sin_addr.s_addr),hp->h_addr,hp->h_length); 
      if (connect(skt->socket,(struct sockaddr*) &addr, lg) == -1)
	{
	  ERROR("wsim:libselect_socket:connect error on %s:%d\n",remot_host,atoi(remot_port));
	  return -1;
	} 
    }
  else
    {
      return -1;
    }

  DMSG_SKT("wsim:libselect_socket:init: done\n");
  
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

  DMSG_SKT("wsim:libselect_socket:accept: waiting for a libselect_socket connection on port %d\n", skt->port);
  if ((skt->socket = accept(skt->socket_listen,(struct sockaddr*)&s,&length)) == -1)
    {
      DMSG_SKT("wsim:libselect_socket:accept: %s", strerror(errno));
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

  DMSG_SKT("wsim:libselect_socket:accept:done fd=%d -> fd=%d\n", skt->socket_listen, skt->socket);
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

int
libselect_skt_getchar(struct libselect_socket_t *skt, unsigned char *c)
{
  *c = 0x55;

  if (skt->socket == -1)
    {
      ERROR("wsim:libselect_socket:getchar: read on closed socket\n");
      return LIBSELECT_SOCKET_GETCHAR_ERROR;
    }

  if (read(skt->socket, c, 1) == -1)
    {
      ERROR("wsim:libselect_socket:getchar: read failed - %s\n",strerror(errno));
      close(skt->socket);
      skt->socket = -1;
      return LIBSELECT_SOCKET_GETCHAR_ERROR;
    }

  DMSG_SKT("wsim:libselect_socket:getchar: 0x%02x %c\n", 
	      *c, isprint(*c) ? *c : '.');

  return LIBSELECT_SOCKET_GETCHAR_OK;
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

  if (write(skt->socket, &c, 1) < 1)
    {
      ERROR("wsim:libselect_socket:putchar: write failed - %s\n",strerror(errno));
      close(skt->socket);
      skt->socket = -1;
      return LIBSELECT_SOCKET_PUTCHAR_ERROR;
    }

  DMSG_SKT("wsim:libselect_socket:getchar: 0x%02x %c\n", 
	      c, isprint(c) ? c : '.');

  return LIBSELECT_SOCKET_PUTCHAR_OK;
}

/* ************************************************** */
/* ************************************************** */

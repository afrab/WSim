/**
 *  \file   pktlist.c
 *  \brief  fixed sized packet list
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#include <string.h>

#include "config.h"
#include "liblogger/logger.h"
#include "pktlist.h"

/***************************************************/
/***************************************************/
/***************************************************/

#undef DEBUG

#if defined(DEBUG)
#define DMSG(x...) HW_DMSG(x)
#else
#define DMSG(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

int pktlist_init(struct _pktlist_t *p)
{
  memset(p,0,sizeof(*p));
  p->rptr = 0;
  p->wptr = 0;
  p->size = 0;
  DMSG("wsnet:pktlist:init:ok\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int pktlist_size(struct _pktlist_t *p)
{
  return p->size;
}

/***************************************************/
/***************************************************/
/***************************************************/

int pktlist_empty(struct _pktlist_t *p)
{
  return p->size == 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

/** returns -1 on error */
int pktlist_enqueue(struct _pktlist_t *p, char *pkt, int len)
{
  if (p->size == PKTLIST_MAX)
    {
      ERROR("wsnet:pktlist:enqueue: list full\n");
      return -1;
    }

  if (len >= PKTSIZE_MAX)
    {
      ERROR("wsnet:pktlist:enqueue: packet too large %d bytes (limit %d)\n",len,PKTSIZE_MAX);
      return -1;
    }

  memcpy(p->msg[ p->wptr ], pkt, len);
  p->len[ p->wptr ] = len; 
  p->wptr           = (p->wptr + 1) % PKTLIST_MAX;
  p->size           = (p->size + 1);
  DMSG("wsnet:pktlist:enqueue: msg size %d, queue size %d\n",len,p->size);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

/** returns pkt size, 0 on empty list, -1 on error */
int pktlist_dequeue(struct _pktlist_t *p, char *pkt)
{
  int len;
  if (p->size == 0)
    {
      ERROR("wsnet:pktlist:dequeue: empty list\n");
      return -1;
    }

  len     = p->len[ p->rptr ];
  memcpy(pkt, p->msg[ p->rptr ], len);
  p->rptr = (p->rptr + 1) % PKTLIST_MAX;
  p->size = (p->size - 1);
  DMSG("wsnet:pktlist:dequeue: msg size %d, queue size %d\n",len,p->size);
  return len;
}

/***************************************************/
/***************************************************/
/***************************************************/

/** returns pkt size, 0 on empty list, -1 on error */
int pktlist_getn(struct _pktlist_t *p, char *pkt, int n)
{
  int len,idx;
  if (n < 0) 
    {
      ERROR("wsnet:pktlist:getn: wrong argument %d\n",n);
      return -1;
    }
  if (p->size == 0) 
    {
      ERROR("wsnet:pktlist:getn: empty list\n");
      return -1;
    }
  if (p->size < (n-1)) 
    {
      ERROR("wsnet:pktlist:getn: request %d, size %d\n",n,p->size);
      return -1;
    }

  idx     = (p->rptr + n) % PKTLIST_MAX;
  len     = p->len[ idx ];
  memcpy(pkt, p->msg[ idx ], len);
  DMSG("wsnet:pktlist:getn: %d\n",n);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

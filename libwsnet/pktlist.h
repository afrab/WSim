/**
 *  \file   pktlist.h
 *  \brief  fixed sized packet list
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#ifndef _PKTLIST_H_
#define _PKTLIST_H_

/***************************************************/
/***************************************************/
/***************************************************/
#define PKTLIST_MAX 0x7F /* 7F == 127 keep simple for modulos */
#define PKTSIZE_MAX 200

struct _pktlist_t 
{
  char msg[PKTLIST_MAX][PKTSIZE_MAX];
  int  len[PKTLIST_MAX];
  int  rptr;
  int  wptr;
  int  size;
};

/* init does not allocate memory, returns 0 on ok */
int pktlist_init       (struct _pktlist_t *p);
int pktlist_size       (struct _pktlist_t *p);
int pktlist_empty      (struct _pktlist_t *p);

/** returns -1 on error */
int pktlist_enqueue    (struct _pktlist_t *p, char *pkt, int len);

/** returns pkt size, 0 on empty list, -1 on error */
int pktlist_dequeue    (struct _pktlist_t *p, char *pkt);
int pktlist_getn       (struct _pktlist_t *p, char *pkt, int n);

/***************************************************/
/***************************************************/
/***************************************************/

#endif

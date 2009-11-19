/**
 *  \file   wsnet2_net.h
 *  \brief  Worldsens client v2, network layer
 *  \author Loic Lemaitre
 *  \date   2009
 **/

#ifndef WSNET2_NET_H
#define WSNET2_NET_H

#include "libwsnet.h"


#define MAX_CALLBACKS 10


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* wsim client states */
#define WORLDSENS_CLT_STATE_CONNECTING    1
#define WORLDSENS_CLT_STATE_IDLE          2
#define WORLDSENS_CLT_STATE_PENDING       3
#define WORLDSENS_CLT_STATE_TXING         4
#define WORLDSENS_CLT_STATE_RXING         5
#define WORLDSENS_CLT_STATE_KILLED        6

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct _worldsens_radio_t {
  wsnet_callback_rx_t            callback;
  void                           *arg;
  char                           *antenna;
  uint32_t                       antenna_id;
};

struct _worldsens_measure_t {
  wsnet_callback_measure_t       callback;
  void                           *arg;
  char                           *name;
  uint32_t                       id;
};
 
struct _worldsens_clt {
  struct _worldsens_radio_t      radio  [MAX_CALLBACKS];
  struct _worldsens_measure_t    measure[MAX_CALLBACKS];
  uint16_t                       nb_radios;              /* number of radios registered*/
  uint16_t                       nb_measures;            /* number of measures registered*/
  uint32_t                       id;                     /* my address */
  int                            u_fd;                   /* unicast file descriptor */
  int                            m_fd;                   /* multicast file descriptor */
  uint64_t                       seq;                    /* receive packet sequence */
  uint64_t                       n_update;               /* next update time */
  uint64_t                       n_rp;                   /* next rendez-vous */
  uint64_t                       l_rp;                   /* last rendez-vous */
  uint64_t                       rpseq;                  /* next rendez vous sequence */ 
  int                            state;                  /* state of wsnet2 client */
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void            wsnet2_init             (void);
void            wsnet2_finalize         (void);
uint32_t        wsnet2_get_node_id      (void);
int             wsnet2_update           (void);
int             wsnet2_register_radio   (char *, wsnet_callback_rx_t, void *);
int             wsnet2_register_measure (char *channel, wsnet_callback_measure_t callback, void *);
int             wsnet2_connect          (char *, uint16_t, char *, uint16_t, uint32_t);
int             wsnet2_tx               (char, double, int, double, uint64_t, int);
int             wsnet2_tx_measure_req   (int);



#endif

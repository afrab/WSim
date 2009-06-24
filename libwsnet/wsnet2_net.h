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

/* TODO: to define phy and radio callback structure specifically */
typedef wsnet_callback_rx_t radio_callback_t;
typedef wsnet_callback_rx_t phy_callback_t;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* wsim client states */
#define WORLDSENS_CLT_STATE_CONNECTING    1
#define WORLDSENS_CLT_STATE_IDLE          2
#define WORLDSENS_CLT_STATE_PENDING       3
#define WORLDSENS_CLT_STATE_TXING         4
#define WORLDSENS_CLT_STATE_RXING         5

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct _worldsens_radio_t {
  radio_callback_t               callback;
  void                           *arg;
  char                           *antenna;
  uint32_t                       id;
};

struct _worldsens_phy_t {
  phy_callback_t                 callback;
  void                           *arg;
  char                           *channel;
  uint32_t                       id;
};
 
struct _worldsens_clt {
  struct _worldsens_radio_t      radio[MAX_CALLBACKS];
  struct _worldsens_phy_t        phy  [MAX_CALLBACKS];
  uint32_t                       id;                     /* my address */
  int                            u_fd;                   /* unicast file descriptor */
  int                            m_fd;                   /* multicast file descriptor */
  int                            dseq;
  int                            seq;  
  uint32_t                       n_update;               /* next update time */
  uint32_t                       n_rp;                   /* next rendez-vous */
  uint32_t                       l_rp;                   /* last rendez-vous */
  int                            rpseq;                  /* rendez vous sequence */ 
  int                            state;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void     wsnet2_init           (void);
void     wsnet2_finalize       (void);
uint32_t wsnet2_get_node_id    (void);
void     wsnet2_register_radio (char *, radio_callback_t, void *);
void     wsnet2_register_phy   (char *channel, phy_callback_t callback, void *);
int      wsnet2_connect        (char *, uint16_t, char *, uint16_t, uint32_t);
int      wsnet2_sync           (void);
int      wsnet2_tx             (char, double, int, double, uint64_t);
int      wsnet2_parse          (char *);
int      wsnet2_seq            (char *);
void     wsnet2_published      (char *);
int      wsnet2_backtrack      (char *);
int      wsnet2_sync_release   (char *);
int      wsnet2_sync_req       (char *);
int      wsnet2_rx             (char *);
int      wsnet2_rxreq          (char *);
int      wsnet2_subscribe      (void);


#endif

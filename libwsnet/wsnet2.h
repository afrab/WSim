/**
 *  \file   wsnet2.h
 *  \brief  Worldsens communication protocol v2
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2007
 **/

#ifndef __wsnet2_h
#define __wsnet2_h

#include <inttypes.h>



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


struct _worldsens_c_backtrack {
  int	                  tx_pkt_seq;      /* pkt_seq, Tx  to  WSNet          */
  int	                  rx_pkt_seq;      /* pkt_seq, Rx from WSNet          */

  int                     rp_seq;          /* sequence                        */
  uint64_t                next_rp;         /* time                            */
  uint64_t                last_rp;         /* time                            */

  char                    pending;
  char                    tx_backtracked;
  int64_t                 min_duration;    /* minimal duration after callback */

  struct _pktlist_t       pktlist;
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* public */
int             worldsens_c_options_add           (void);
void            worldsens_c_state_save            (void);
void            worldsens_c_state_restore         (void);
int             worldsens_c_get_node_id           (void);
int             worldsens_c_rx_register           (void*, wsnet_callback_rx_t, char*);

int             worldsens_c_initialize            (void);
int             worldsens_c_connect               (void);
int             worldsens_c_close                 (void);
int             worldsens_c_tx                    (struct wsnet_tx_info *);
int             worldsens_c_update                (void);

/* private */
static int      worldsens_option_validate         (void);   /* validate options */


#endif

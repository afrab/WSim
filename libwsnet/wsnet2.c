/**
 *  \file   wsnet2.c
 *  \brief  Worldsens communication protocol v2
 *  \author Guillaume Chelius, Antoine Fraboulet, Loic Lemaitre
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/options.h"
#include "liblogger/logger.h"

#include "pktlist.h"
#include "wsnet2_net.h"
#include "wsnet2.h"
#include "wsnet2_dbg.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static struct moption_t node_id_opt = {
  .longname    = "node-id",
  .type        = required_argument,
  .helpstring  = "worldsens node id",
  .value       = NULL
};

static struct moption_t server_ip_opt = {
  .longname    = "server-ip",
  .type        = required_argument,
  .helpstring  = "server ip address",
  .value       = NULL
};

static struct moption_t server_port_opt = {
  .longname    = "server-port",
  .type        = required_argument,
  .helpstring  = "server udp port",
  .value       = NULL
};

static struct moption_t multicast_ip_opt = {
  .longname    = "multicast-ip",
  .type        = required_argument,
  .helpstring  = "multicast ip address",
  .value       = NULL
};

static struct moption_t multicast_port_opt = {
  .longname    = "multicast-port",
  .type        = required_argument,
  .helpstring  = "multicast udp port",
  .value       = NULL
};


static struct _worldsens_c_backtrack    worldsens_backtracked;
static struct _worldsens_c_backtrack    worldsens_backup;

#define WSENS_SEQ_PKT_TX        worldsens_backtracked.tx_pkt_seq
#define WSENS_SEQ_PKT_RX        worldsens_backtracked.rx_pkt_seq
#define WSENS_SEQ_RDV           worldsens_backtracked.rp_seq     /* next RDV */

#define WSENS_RDV_LAST_TIME     worldsens_backtracked.last_rp
#define WSENS_RDV_NEXT_TIME     worldsens_backtracked.next_rp
#define WSENS_RDV_PENDING       worldsens_backtracked.pending

#define WSENS_TX_BACKTRACKED    worldsens_backtracked.tx_backtracked
#define WSENS_TIME_TO_WAIT      worldsens_backtracked.min_duration

#define WSENS_PKT_LIST          worldsens_backtracked.pktlist


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_options_add(void)
{
  options_add(& server_ip_opt      );
  options_add(& server_port_opt    );
  options_add(& multicast_ip_opt   );
  options_add(& multicast_port_opt );
  options_add(& node_id_opt        );
  return 0;
}

static int worldsens_option_validate(void)
{
  if (node_id_opt.value != NULL) {
    HW_DMSG_DEV(" - node id option value %s\n",node_id_opt.value);
  } else {
    node_id_opt.value = "1";
  }

  if (server_ip_opt.value != NULL) {
    HW_DMSG_DEV(" - server ip option value %s\n",server_ip_opt.value);
  } else {
    server_ip_opt.value = "127.0.0.1";
  }

  if (server_port_opt.value != NULL) {
    HW_DMSG_DEV(" - server port option value %d\n",atoi(server_port_opt.value));
  } else {
    server_port_opt.value = "9998";
  }

  if (multicast_ip_opt.value != NULL) {
    HW_DMSG_DEV(" - multicast ip option value %s\n",multicast_ip_opt.value);
  } else {
    multicast_ip_opt.value = "224.0.0.1";
  }

  if (multicast_port_opt.value != NULL) {
    HW_DMSG_DEV(" - multicast port option value %d\n",atoi(multicast_port_opt.value));
  } else {
    multicast_port_opt.value = "9999";
  }

  return 0;
}

void worldsens_c_state_save     (void)
{
  memcpy(&worldsens_backup, &worldsens_backtracked, sizeof(struct _worldsens_c_backtrack));
}

void worldsens_c_state_restore  (void)
{
  memcpy(&worldsens_backtracked, &worldsens_backup, sizeof(struct _worldsens_c_backtrack));
}

int worldsens_c_get_node_id(void)
{
  return wsnet2_get_node_id();
}

void worldsens_c_rx_register(void* arg, wsnet_callback_rx_t cbrx)
{
  char antenna[] = "" ;
  wsnet2_register_radio(antenna, cbrx, arg);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_initialize(void)
{
  int        ret_connect;
  char      *srv_addr;
  uint16_t   srv_port;
  char      *mul_addr;
  uint16_t   mul_port;
  uint32_t   node_id;

  /* parse options */
  worldsens_option_validate();

  /* structures initialization */
  wsnet2_init();

 /* initialize multicast and unicast sockets */
  srv_addr = server_ip_opt.value;
  srv_port = atoi(server_port_opt.value);
  mul_addr = multicast_ip_opt.value;
  mul_port = atoi(multicast_port_opt.value);
  node_id  = atoi(node_id_opt.value);

  ret_connect = wsnet2_connect(srv_addr, srv_port, mul_addr, mul_port, node_id);

  return ret_connect;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_close(void) 
{
  wsnet2_finalize();
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_tx(struct wsnet_tx_info *info) 
{
  char data           = info->data;
  double frequency    = info->freq_mhz * 1000000; //TODO : to check 1000000 factor
  int modulation      = info->modulation;

  double tx_dbm       = info->power_dbm;
  uint64_t duration   = info->duration;

  if( wsnet2_tx(data, frequency, modulation,tx_dbm, duration) == -1 )
    {
      ERROR("wsnet2:tx: error during packet send\n");
      return -1;
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_update(void) 
{
  if( wsnet2_update() == -1 )
    {
      ERROR("wsnet2:update: error during update\n");      
      return -1;
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

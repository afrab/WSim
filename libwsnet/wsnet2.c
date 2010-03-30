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

void worldsens2_c_state_save     (void)
{
}

void worldsens2_c_state_restore  (void)
{
}

int worldsens2_c_get_node_id(void)
{
  return wsnet2_get_node_id();
}

int worldsens2_c_rx_register(void* arg, wsnet_callback_rx_t cbrx, char *antenna)
{
  return wsnet2_register_radio(antenna, cbrx, arg);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens2_c_initialize(void)
{
  /* structures initialization */
  wsnet2_init();
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens2_c_connect(char *srv_addr, uint16_t srv_port, char *mul_addr, uint16_t mul_port, uint32_t node_id)
{
  return wsnet2_connect(srv_addr, srv_port, mul_addr, mul_port, node_id);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens2_c_close(void) 
{
  wsnet2_finalize();
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens2_c_tx(struct wsnet_tx_info *info) 
{
  char data           = info->data;
  double frequency    = info->freq_mhz * 1000000; //TODO : to check 1000000 factor
  int modulation      = info->modulation;

  double tx_dbm       = info->power_dbm;
  uint64_t duration   = info->duration;
  int radio_id        = info->radio_id;

  if( wsnet2_tx(data, frequency, modulation, tx_dbm, duration, radio_id) == -1 )
    {
      ERROR("wsnet2:tx: error during packet send\n");
      return -1;
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens2_c_update(void) 
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

/**
 *  \file   wsnet_wrapper.c
 *  \brief  Worldsens communication protocol wrapper
 *  \author Loic Lemaitre
 *  \date   2009
 **/

#include "machine/machine.h"
#include "libwsnet.h"
#include "src/options.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int  worldsens_c_initialize (int wsens_mode)
{
  switch (wsens_mode)
    {
    case WS_MODE_WSNET0 :
      worldsens_c_state_save    = worldsens0_c_state_save;
      worldsens_c_state_restore = worldsens0_c_state_restore;
      worldsens_c_get_node_id   = worldsens0_c_get_node_id;
      worldsens_c_rx_register   = worldsens0_c_rx_register;
      worldsens_c_connect       = worldsens0_c_connect;
      worldsens_c_close         = worldsens0_c_close;
      worldsens_c_tx            = worldsens0_c_tx;
      worldsens_c_update        = worldsens0_c_update;
      return worldsens0_c_initialize();

    case WS_MODE_WSNET1 :
      worldsens_c_state_save    = worldsens1_c_state_save;
      worldsens_c_state_restore = worldsens1_c_state_restore;
      worldsens_c_get_node_id   = worldsens1_c_get_node_id;
      worldsens_c_rx_register   = worldsens1_c_rx_register;
      worldsens_c_connect       = worldsens1_c_connect;
      worldsens_c_close         = worldsens1_c_close;
      worldsens_c_tx            = worldsens1_c_tx;
      worldsens_c_update        = worldsens1_c_update;
      return worldsens1_c_initialize();

    case WS_MODE_WSNET2 :
      worldsens_c_state_save    = worldsens2_c_state_save;
      worldsens_c_state_restore = worldsens2_c_state_restore;
      worldsens_c_get_node_id   = worldsens2_c_get_node_id;
      worldsens_c_rx_register   = worldsens2_c_rx_register;
      worldsens_c_connect       = worldsens2_c_connect;
      worldsens_c_close         = worldsens2_c_close;
      worldsens_c_tx            = worldsens2_c_tx;
      worldsens_c_update        = worldsens2_c_update;
      return worldsens2_c_initialize();
    }

  return 0;
}

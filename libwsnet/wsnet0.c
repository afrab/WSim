/**
 *  \file   wsnet0.c
 *  \brief  Worldsens communication protocol wrapper
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include "libwsnet.h"
#include "wsnet0.h"

#undef UNUSED
#define UNUSED __attribute__((unused))


int worldsens_c_options_add(void)
{
  return 0;
}

int worldsens_c_initialize(void)
{
  return 0;
}

int worldsens_c_connect(void)
{
  return 0;
}

int worldsens_c_close(void)
{
  return 0;
}

int worldsens_c_get_node_id(void)
{
  return 0;
}

void worldsens_c_rx_register(void UNUSED *arg, wsnet_callback_rx_t UNUSED cbrx)
{
}

int worldsens_c_tx (struct wsnet_tx_info UNUSED *tx)
{
  return 0;
}

int worldsens_c_update(void)
{
  return 0;
}

void worldsens_c_state_save(void)
{
}

void worldsens_c_state_restore(void)
{
}

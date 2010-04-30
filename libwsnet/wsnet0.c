/**
 *  \file   wsnet0.c
 *  \brief  Worldsens communication protocol wrapper
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include "machine/machine.h"
#include "src/options.h"

#include "libwsnet.h"
#include "wsnet0.h"


#undef UNUSED
#define UNUSED __attribute__((unused))


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static int worldsens_nb_interfaces = 0; /* number of registered interfaces */

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens0_c_initialize(void)
{
  return 0;
}

int worldsens0_c_connect(char UNUSED *srv_addr, uint16_t UNUSED srv_port, 
			 char UNUSED *mul_addr, uint16_t UNUSED mul_port, uint32_t UNUSED node_id)
{
  return 0;
}

int worldsens0_c_close(void)
{
  return 0;
}

int worldsens0_c_get_node_id(void)
{
  return -1;
}

int worldsens0_c_rx_register(void UNUSED *arg, wsnet_callback_rx_t UNUSED cbrx, char UNUSED *antenna)
{
  return worldsens_nb_interfaces++;
}

int worldsens0_c_tx (struct wsnet_tx_info UNUSED *tx)
{
  return 0;
}

int worldsens0_c_update(void)
{
  return 0;
}

void worldsens0_c_state_save(void)
{
}

void worldsens0_c_state_restore(void)
{
}

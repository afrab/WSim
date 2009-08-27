/**
 *  \file   libwsnet.h
 *  \brief  Worldsens communication protocol wrapper
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef _LIBWSNET_H
#define _LIBWSNET_H

#include <stdint.h>
#include <stdio.h>

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define WSNET_MODULATION_UNKNOWN    0x00
#define WSNET_MODULATION_2FSK       0x01
#define WSNET_MODULATION_GFSK       0x02
#define WSNET_MODULATION_ASK_OOK    0x03
#define WSNET_MODULATION_MSK        0x04
#define WSNET_MODULATION_OQPSK      0x05
#define WSNET_MODULATION_OQPSK_REV  0x06
#define WSNET_MODULATION_802_15_4   0x07

/**
 * Units:
 *   - Frequency      : MHz
 *   - Channel Width  : Hz
 *   - Time           : ns
 *   - Power          : dBm
 **/

struct wsnet_rx_info {
  uint8_t   data;

  uint8_t   pad1;
  uint8_t   pad2;
  uint8_t   pad3;

  double    freq_mhz;
  double    freq_width;
  uint32_t  modulation;
  double    power_dbm;
  double    SiNR;
};

struct wsnet_tx_info {
  uint8_t   data;

  uint8_t   pad1;
  uint8_t   pad2;
  uint8_t   pad3;

  double    freq_mhz;
  double    freq_width;
  uint32_t  modulation;
  double    power_dbm;
  uint64_t  duration;

  int       radio_id;
};

typedef uint64_t (*wsnet_callback_rx_t) (void*, struct wsnet_rx_info *);

int  worldsens_c_tx             (struct wsnet_tx_info *);
int  worldsens_c_rx_register    (void*, wsnet_callback_rx_t);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int  worldsens_c_options_add    (void);
int  worldsens_c_initialize     (void);
int  worldsens_c_connect        (void);
int  worldsens_c_close          (void);

int  worldsens_c_update         (void);
int  worldsens_c_get_node_id    (void);

void worldsens_c_state_save     (void);
void worldsens_c_state_restore  (void);

#define LIBWSNET_UPDATE()  worldsens_c_update()

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

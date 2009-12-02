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

#define WSNET_MAX_MODULATIONS       0x08

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

typedef uint64_t (*wsnet_callback_rx_t)      (void*, struct wsnet_rx_info *);
typedef uint64_t (*wsnet_callback_measure_t) (void*, double);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* libwsnet public encapsulation functions */
int    worldsens_c_initialize     (int);

void (*worldsens_c_state_save)    (void);
void (*worldsens_c_state_restore) (void);
int  (*worldsens_c_get_node_id)   (void);
int  (*worldsens_c_rx_register)   (void*, wsnet_callback_rx_t, char*);
int  (*worldsens_c_connect)       (char*, uint16_t, char*, uint16_t, uint32_t);
int  (*worldsens_c_close)         (void);
int  (*worldsens_c_tx)            (struct wsnet_tx_info *);
int  (*worldsens_c_update)        (void);

#define LIBWSNET_UPDATE()  worldsens_c_update()

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* libwsnet0 public functions */
int  worldsens0_c_initialize    (void);

void worldsens0_c_state_save    (void);
void worldsens0_c_state_restore (void);
int  worldsens0_c_get_node_id   (void);
int  worldsens0_c_rx_register   (void*, wsnet_callback_rx_t, char*);
int  worldsens0_c_connect       (char*, uint16_t, char*, uint16_t, uint32_t);
int  worldsens0_c_close         (void);
int  worldsens0_c_tx            (struct wsnet_tx_info *);
int  worldsens0_c_update        (void);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* libwsnet1 public functions */
int  worldsens1_c_initialize    (void);

void worldsens1_c_state_save    (void);
void worldsens1_c_state_restore (void);
int  worldsens1_c_get_node_id   (void);
int  worldsens1_c_rx_register   (void*, wsnet_callback_rx_t, char*);
int  worldsens1_c_connect       (char*, uint16_t, char*, uint16_t, uint32_t);
int  worldsens1_c_close         (void);
int  worldsens1_c_tx            (struct wsnet_tx_info *);
int  worldsens1_c_update        (void);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* libwsnet2 public functions */
int  worldsens2_c_initialize    (void);

void worldsens2_c_state_save    (void);
void worldsens2_c_state_restore (void);
int  worldsens2_c_get_node_id   (void);
int  worldsens2_c_rx_register   (void*, wsnet_callback_rx_t, char*);
int  worldsens2_c_connect       (char*, uint16_t, char*, uint16_t, uint32_t);
int  worldsens2_c_close         (void);
int  worldsens2_c_tx            (struct wsnet_tx_info *);
int  worldsens2_c_update        (void);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif

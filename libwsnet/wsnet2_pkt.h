/**
 *  \file   wsnet2_pkt.h
 *  \brief  Worldsens client v2, packet format
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2007
 **/

#ifndef WORLDSENS_PKT_H
#define WORLDSENS_PKT_H

#include <stdint.h>

#define __PACKED__  __attribute__((packed))

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum woldsens_pkt_type {
  WORLDSENS_UNKNOWN = 0,

  /* WSim -> WSNet2 */
  WORLDSENS_CONNECT_REQ,  /* connection request  */
  WORLDSENS_SYNC_ACK,     /*                     */
  WORLDSENS_BYTE_TX,      /* Tx                  */
  WORLDSENS_MEASURE_REQ,  /* measure request     */
  WORLDSENS_DISCONNECT,   /* disconnect          */

  /* WSNet2 -> WSim */
  WORLDSENS_CONNECT_RSP,  /* connection granted  */  
  WORLDSENS_SYNC_REQ,     /* RP request          */
  WORLDSENS_SYNC_RELEASE, /* RP save and release */
  WORLDSENS_BYTE_RX,      /* Rx                  */
  WORLDSENS_MEASURE_RSP,  /* measure response    */
  WORLDSENS_KILLSIM,

  /* */
  WORLDSENS_LASTID
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

typedef uint8_t                 ws_pkt_type;

typedef uint32_t                ws_id_node;
typedef uint32_t                ws_id_resource;
typedef uint64_t                ws_id_rp;
typedef uint64_t                ws_id_seq;

typedef double                  ws_frequency;
typedef double                  ws_power;
typedef double                  ws_measure;
typedef double                  ws_ber;

typedef uint64_t                ws_time;
typedef uint8_t                 ws_data;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct __PACKED__ ws_connect_req {
  ws_pkt_type            pkt_id;
  ws_id_node             node_id;
};

struct __PACKED__ ws_sync_ack {
  ws_pkt_type            pkt_id;
  ws_id_node             node_id;
  ws_id_rp               rp_id;
};

struct __PACKED__ ws_byte_tx {
  ws_pkt_type            pkt_id;
  ws_id_node             node_id;
  ws_id_resource         antenna_id;
  ws_id_resource         modulation_id;
  ws_frequency           freq;
  ws_power               power;
  ws_time                duration;
  ws_data                data;
};

struct __PACKED__ ws_measure_req {
  ws_pkt_type            pkt_id;
  ws_id_resource         measure_id;
};

struct __PACKED__ ws_disconnect {
  ws_pkt_type            pkt_id;
  ws_id_node             node_id;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define WORLDSENS_MAX_MODELS_ID_SIZE 1200

struct __PACKED__ ws_connect_rsp {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;

  uint8_t                n_antenna_id;
  uint8_t                n_modulation_id;
  uint8_t                n_measure_id;

  char                   names[WORLDSENS_MAX_MODELS_ID_SIZE];
};

struct __PACKED__ ws_sync_req {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;
};

struct __PACKED__ ws_sync_release {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;
};

struct __PACKED__ ws_byte_rx {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
  ws_id_resource         antenna_id;
  ws_id_resource         modulation_id;
  ws_frequency           freq;
  ws_power               power;
  ws_data                data;
  ws_ber                 ber;
};

struct __PACKED__ ws_measure_rsp {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
  ws_id_resource         measure_id;
  ws_measure             measure_val;
};

struct __PACKED__ ws_killsim {
  ws_pkt_type            pkt_id;
  ws_id_seq              seq;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

union ws_pkt {
  struct ws_connect_req        cnx_req;
  struct ws_connect_rsp        cnx_rsp;

  struct ws_byte_tx            byte_tx;
  struct ws_byte_rx            byte_rx;

  struct ws_sync_req           sync_req;
  struct ws_sync_ack           sync_ack;
  struct ws_sync_release       sync_release;

  struct ws_measure_req        measure_req;
  struct ws_measure_rsp        measure_rsp;

  struct ws_disconnect         disconnect;
  struct ws_killsim            killsim;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* returns type of packet             */
/* return type == 0 UNKNOWN on errors */

int worldsens_packet_hton(union ws_pkt *pkt);
int worldsens_packet_ntoh(union ws_pkt *pkt);

/* dump on stdout */
int worldsens_packet_dump(union ws_pkt *pkt);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif


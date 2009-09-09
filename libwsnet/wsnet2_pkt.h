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

#define WORLDSENS_MAX_PKTLENGTH (sizeof(union _worldsens_pkt))

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum woldsens_pkt_type {
  WORLDSENS_UNKNOWN = 0,

  /* WSim -> WSNet2 */
  WORLDSENS_C_CONNECT_REQ,     /* connection request  */
  WORLDSENS_C_SYNC_ACK,        /*                     */
  WORLDSENS_C_BYTE_TX,         /* Tx                  */
  WORLDSENS_C_MEASURE_REQ,     /* measure request     */
  WORLDSENS_C_DISCONNECT,      /* disconnect          */

  /* WSNet2 -> WSim */
  WORLDSENS_S_CONNECT_RSP_OK,  /* connection granted  */
  WORLDSENS_S_CONNECT_RSP_NOK, /* connection refused  */
  WORLDSENS_S_SYNC_RELEASE,    /* RP save and release */
  WORLDSENS_S_BACKTRACK,       /* backtrack request   */
  WORLDSENS_S_BYTE_RX,         /* Rx                  */
  WORLDSENS_S_BYTE_SR_RX,      /* RP req and rx       */
  WORLDSENS_S_MEASURE_RSP,     /* measure response    */
  WORLDSENS_S_KILLSIM,         /* kill all nodes      */
  WORLDSENS_S_KILL,            /* kill one node       */

  /* */
  WORLDSENS_LASTID
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

typedef uint8_t                 ws_pkt_type;

typedef uint32_t                ws_id_node;
typedef int64_t                 ws_id_resource;
typedef uint64_t                ws_id_rp;
typedef uint64_t                ws_id_seq;

typedef double                  ws_frequency;
typedef double                  ws_power;
typedef double                  ws_measure;
typedef double                  ws_sinr;

typedef uint64_t                ws_time;
typedef uint8_t                 ws_data;



/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* packet sent by a client to the server */

struct __PACKED__ _worldsens_c_header {
  ws_pkt_type            type;
  ws_id_node             id;
};

struct __PACKED__ _worldsens_c_connect_req {
  ws_pkt_type            type;
  ws_id_node             node_id;
};

struct __PACKED__ _worldsens_c_sync_ack {
  ws_pkt_type            type;
  ws_id_node             node_id;
  ws_id_rp               rp_id;
};

struct __PACKED__ _worldsens_c_byte_tx {
  ws_pkt_type            type;
  ws_id_node             node_id;
  ws_id_resource         antenna_id;
  ws_id_resource         wsnet_mod_id;
  ws_id_resource         wsim_mod_id;
  ws_frequency           freq;
  ws_power               power_dbm;
  ws_time                duration;
  ws_time                period;
  ws_data                data;
};

struct __PACKED__ _worldsens_c_measure_req {
  ws_pkt_type            type;
  ws_id_node             node_id;
  ws_id_resource         measure_id;
};

struct __PACKED__ _worldsens_c_disconnect {
  ws_pkt_type            type;
  ws_id_node             node_id;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define WORLDSENS_MAX_MODELS_SIZE    1200


/* packet sent by the server to a client */

struct __PACKED__ _worldsens_s_header {
  ws_pkt_type            type;
  ws_id_seq              seq;
};

struct __PACKED__ _worldsens_s_connect_rsp_ok {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;

  uint8_t                n_antenna_id;
  uint8_t                n_modulation_id;
  uint8_t                n_measure_id;

  char                   names_and_ids[WORLDSENS_MAX_MODELS_SIZE];
};

struct __PACKED__ _worldsens_s_connect_rsp_nok {
  ws_pkt_type            type;
  ws_id_seq              seq;
};

struct __PACKED__ _worldsens_s_sync_release {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;
};

struct __PACKED__ _worldsens_s_backtrack {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;
};

struct __PACKED__ _worldsens_s_byte_rx {
  ws_pkt_type            type;                        
  ws_id_seq              seq;
  ws_id_node             node_id;                          
  ws_id_resource         antenna_id;                       
  ws_id_resource         wsim_mod_id;                      
  ws_frequency           freq;                                                         
  ws_power               power_dbm;
  ws_sinr                sinr;
  ws_data                data;
};

struct __PACKED__ _worldsens_s_byte_sr_rx {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_rp               rp_next;
  ws_time                rp_duration;
  ws_id_node             node_id;
  ws_id_resource         antenna_id;
  ws_id_resource         wsim_mod_id;
  ws_frequency           freq;
  ws_power               power_dbm;
  ws_sinr                sinr;
  ws_data                data;
};

struct __PACKED__ _worldsens_s_measure_rsp {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_resource         measure_id;
  ws_measure             measure_val;
};

struct __PACKED__ _worldsens_s_killsim {
  ws_pkt_type            type;
  ws_id_seq              seq;
};

struct __PACKED__ _worldsens_s_kill {
  ws_pkt_type            type;
  ws_id_seq              seq;
  ws_id_node             node_id;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

union _worldsens_pkt {
  struct _worldsens_c_header             c_header;
  struct _worldsens_s_header             s_header;

  struct _worldsens_c_connect_req        cnx_req;
  struct _worldsens_s_connect_rsp_ok     cnx_rsp_ok;
  struct _worldsens_s_connect_rsp_nok    cnx_rsp_nok;

  struct _worldsens_c_byte_tx            byte_tx;
  struct _worldsens_s_byte_rx            byte_rx;
  struct _worldsens_s_byte_sr_rx         byte_sr_rx;

  struct _worldsens_c_sync_ack           sync_ack;
  struct _worldsens_s_sync_release       sync_release;

  struct _worldsens_s_backtrack          bktrk;

  struct _worldsens_c_measure_req        measure_req;
  struct _worldsens_s_measure_rsp        measure_rsp;

  struct _worldsens_c_disconnect         disconnect;
  struct _worldsens_s_killsim            killsim;
  struct _worldsens_s_kill               kill;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* returns type of packet             */
/* return type == 0 UNKNOWN on errors */

int worldsens_packet_hton(union _worldsens_pkt *pkt);
int worldsens_packet_ntoh(union _worldsens_pkt *pkt);

/* dump on stdout */
int worldsens_packet_dump(union _worldsens_pkt *pkt);


uint64_t ntohll  (uint64_t);
uint64_t htonll  (uint64_t);
double   ntohdbl (double);
double   htondbl (double);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif


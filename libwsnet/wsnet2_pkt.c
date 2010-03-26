/**
 *  \file   wsnet2_pkt.c
 *  \brief  WorldSens client v2, packet format
 *  \author Guillaume Chelius, Antoine Fraboulet, Loic Lemaitre
 *  \date   2007
 **/


#define UNUSED __attribute__((unused))
#define WSIM

#if defined(WSIM) /* WSIM */
#include "wsnet2_pkt.h"
#include "wsnet2_net.h"
#include "liblogger/logger.h"
#define DIRECTION1 "snd -->"
#define DIRECTION2 "rcv <--"
#define SOFT       "Libwsnet2"
#else /* WSNET */
#include <include/worldsens_pkt.h>
#include <stdio.h>
#define ERROR(x...) fprintf(stderr,x)
#define VERBOSE(UNUSED,x...) fprintf(stderr,x)
#define DIRECTION1 "rcv <--"
#define DIRECTION2 "snd -->"
#define SOFT       "WSNET2:"
#endif

#define VLVL 5  /* Verbose level for worldsens_packet_dump function */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

//#define WORLDSENS_USES_LITTLEENDIAN 1
//#define WORDS_BIGENDIAN

#if defined(WORLDSENS_USES_LITTLEENDIAN) 
#  if defined(WORDS_BIGENDIAN)
#     undef   SWAP_IS_NOP
#  else
#     define  SWAP_IS_NOP
#  endif
#else
#  if defined(WORDS_BIGENDIAN)
#     define  SWAP_IS_NOP
#  else
#     undef   SWAP_IS_NOP
#  endif
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(SWAP_IS_NOP)


static inline void swap2(uint8_t UNUSED *v) { }
static inline void swap4(uint8_t UNUSED *v) { }
static inline void swap8(uint8_t UNUSED *v) { }

#define SWAPN(v) do { } while (0)


#else /* SWAP_IS_NOP */


static inline void swap2(uint16_t *v)
{
  uint16_t r;
  uint8_t *pv = (uint8_t *) v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[1];
  pr[1] = pv[0];
  *v = r;
}

static inline void swap4(uint32_t *v)
{
  uint32_t r;
  uint8_t *pv = (uint8_t *) v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[3];
  pr[1] = pv[2];
  pr[2] = pv[1];
  pr[3] = pv[0];
  *v = r;
}

static inline void swap8(uint64_t *v)
{
  uint64_t r;
  uint8_t *pv = (uint8_t *) v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[7];
  pr[1] = pv[6];
  pr[2] = pv[5];
  pr[3] = pv[4];
  pr[4] = pv[3];
  pr[5] = pv[2];
  pr[6] = pv[1];
  pr[7] = pv[0];
  *v = r;
}

#define SWAPN(v)				     \
  do {						     \
    if (sizeof(v)      == 4)			     \
      swap4((uint32_t *) &v);			     \
    else if (sizeof(v) == 8)			     \
      swap8((uint64_t *) &v);			     \
    else if (sizeof(v) == 2)			     \
      swap2((uint16_t *) &v);			     \
  } while (0)			     


#endif /* SWAP_IS_NOP */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline int worldsens_packet_swap(union _worldsens_pkt *pkt)
{
  int ptype = *((ws_pkt_type*)pkt);
  /* int pkt_type = pkt->cnx_req.pkt_id; */ /* could be any struct ws_* */

  switch (ptype)
    {
    case WORLDSENS_C_CONNECT_REQ:
      SWAPN (pkt->cnx_req.type);
      SWAPN (pkt->cnx_req.node_id);
      break;

    case WORLDSENS_C_SYNC_ACK:
      SWAPN (pkt->sync_ack.type);
      SWAPN (pkt->sync_ack.node_id);
      SWAPN (pkt->sync_ack.rp_id);
      break;

    case WORLDSENS_C_BYTE_TX:
      SWAPN (pkt->byte_tx.type);
      SWAPN (pkt->byte_tx.node_id);
      SWAPN (pkt->byte_tx.antenna_id);
      SWAPN (pkt->byte_tx.wsnet_mod_id);
      SWAPN (pkt->byte_tx.wsim_mod_id);
      SWAPN (pkt->byte_tx.freq);
      SWAPN (pkt->byte_tx.power_dbm);
      SWAPN (pkt->byte_tx.duration);
      SWAPN (pkt->byte_tx.data);
      SWAPN (pkt->byte_tx.period);
      break;

    case WORLDSENS_C_MEASURE_REQ:
      SWAPN (pkt->measure_req.type);
      SWAPN (pkt->measure_req.node_id);
      SWAPN (pkt->measure_req.measure_id);
      SWAPN (pkt->measure_req.period);
      break;

    case WORLDSENS_C_DISCONNECT:
      SWAPN (pkt->disconnect.type);
      SWAPN (pkt->disconnect.node_id);
      break;

    case WORLDSENS_S_CONNECT_RSP_OK:
      SWAPN (pkt->cnx_rsp_ok.type);
      SWAPN (pkt->cnx_rsp_ok.seq);
      SWAPN (pkt->cnx_rsp_ok.rp_next);
      SWAPN (pkt->cnx_rsp_ok.rp_duration);
      SWAPN (pkt->cnx_rsp_ok.n_antenna_id);
      SWAPN (pkt->cnx_rsp_ok.n_modulation_id);
      SWAPN (pkt->cnx_rsp_ok.n_measure_id);
      /* .names field left alone */
      break;

    case WORLDSENS_S_CONNECT_RSP_NOK:
      SWAPN (pkt->cnx_rsp_nok.type);
      SWAPN (pkt->cnx_rsp_nok.seq);
      break;

    case WORLDSENS_S_SYNC_RELEASE:
      SWAPN (pkt->sync_release.type);
      SWAPN (pkt->sync_release.seq);
      SWAPN (pkt->sync_release.rp_next);
      SWAPN (pkt->sync_release.rp_duration);
      break;

    case WORLDSENS_S_BACKTRACK:
      SWAPN (pkt->sync_release.type);
      SWAPN (pkt->sync_release.seq);
      SWAPN (pkt->sync_release.rp_next);
      SWAPN (pkt->sync_release.rp_duration);
      break;

    case WORLDSENS_S_BYTE_RX:
      SWAPN (pkt->byte_rx.type);
      SWAPN (pkt->byte_rx.seq);
      SWAPN (pkt->byte_rx.node_id);
      SWAPN (pkt->byte_rx.antenna_id);
      SWAPN (pkt->byte_rx.wsim_mod_id);
      SWAPN (pkt->byte_rx.freq);
      SWAPN (pkt->byte_rx.power_dbm);
      SWAPN (pkt->byte_rx.sinr);
      SWAPN (pkt->byte_rx.data);
      break;

    case WORLDSENS_S_BYTE_SR_RX:
      SWAPN (pkt->byte_sr_rx.type);
      SWAPN (pkt->byte_sr_rx.seq);
      SWAPN (pkt->byte_sr_rx.rp_next);
      SWAPN (pkt->byte_sr_rx.rp_duration);
      SWAPN (pkt->byte_sr_rx.node_id);
      SWAPN (pkt->byte_sr_rx.antenna_id);
      SWAPN (pkt->byte_sr_rx.wsim_mod_id);
      SWAPN (pkt->byte_sr_rx.freq);
      SWAPN (pkt->byte_sr_rx.power_dbm);
      SWAPN (pkt->byte_sr_rx.sinr);
      SWAPN (pkt->byte_sr_rx.data);
      break;

    case WORLDSENS_S_MEASURE_RSP:
      SWAPN (pkt->measure_rsp.type);
      SWAPN (pkt->measure_rsp.seq);
      SWAPN (pkt->measure_rsp.node_id);
      SWAPN (pkt->measure_rsp.measure_id);
      SWAPN (pkt->measure_rsp.measure_val);
      break;

    case WORLDSENS_S_MEASURE_SR_RSP:
      SWAPN (pkt->measure_sr_rsp.type);
      SWAPN (pkt->measure_sr_rsp.seq);
      SWAPN (pkt->measure_sr_rsp.node_id);
      SWAPN (pkt->measure_sr_rsp.measure_id);
      SWAPN (pkt->measure_sr_rsp.measure_val);
      SWAPN (pkt->measure_sr_rsp.rp_next);
      SWAPN (pkt->measure_sr_rsp.rp_duration);
      break;

    case WORLDSENS_S_KILLSIM:
      SWAPN (pkt->killsim.type);
      SWAPN (pkt->killsim.seq);
      break;

    case WORLDSENS_S_KILL:
      SWAPN (pkt->kill.type);
      SWAPN (pkt->kill.seq);
      SWAPN (pkt->kill.node_id);
      break;

    default:
      ERROR("worldsens: hton/ntoh unknown packet type 0x%x\n",ptype);
      ptype = WORLDSENS_UNKNOWN;
      break;
    }
  return ptype;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int worldsens_packet_hton(union _worldsens_pkt *pkt)
{
  return worldsens_packet_swap(pkt);
}

int worldsens_packet_ntoh(union _worldsens_pkt *pkt)
{
  return worldsens_packet_swap(pkt);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int worldsens_packet_dump(union _worldsens_pkt *msg)
{
  struct _worldsens_s_header *header = (struct _worldsens_s_header *) msg;

  VERBOSE(VLVL,"%s:=======:pkt: start ====================\n", SOFT);
  switch (header->type)
    {
      /*WSIM->WSNET2*/
    case WORLDSENS_C_CONNECT_REQ:
      {
	struct _worldsens_c_connect_req *pkt = (struct _worldsens_c_connect_req *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION1, "WORLDSENS_C_CONNECT_REQ");
	VERBOSE(VLVL,"%s:%s:pkt:    node %d\n",                      SOFT, DIRECTION1, pkt->node_id);
	break;
      }
    case WORLDSENS_C_SYNC_ACK:
      {
	struct _worldsens_c_sync_ack *pkt = (struct _worldsens_c_sync_ack *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION1, "WORLDSENS_C_SYNC_ACK");
	VERBOSE(VLVL,"%s:%s:pkt:    node %d\n",                      SOFT, DIRECTION1, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_id %"PRIu64"\n",              SOFT, DIRECTION1, pkt->rp_id);
	break;
      }
    case WORLDSENS_C_BYTE_TX:
      {
	struct _worldsens_c_byte_tx *pkt = (struct _worldsens_c_byte_tx *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION1, "WORLDSENS_C_BYTE_TX");
	VERBOSE(VLVL,"%s:%s:pkt:    node %d\n",                      SOFT, DIRECTION1, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    antenna %"PRId64"\n",            SOFT, DIRECTION1, pkt->antenna_id);
	VERBOSE(VLVL,"%s:%s:pkt:    wsnet modulation id %"PRId64"\n",SOFT, DIRECTION1, pkt->wsnet_mod_id);
	VERBOSE(VLVL,"%s:%s:pkt:    wsim modulation id %"PRId64"\n", SOFT, DIRECTION1, pkt->wsim_mod_id);
	VERBOSE(VLVL,"%s:%s:pkt:    frequence %g hz\n",              SOFT, DIRECTION1, pkt->freq);
	VERBOSE(VLVL,"%s:%s:pkt:    power %g dbm\n",                 SOFT, DIRECTION1, pkt->power_dbm);
	VERBOSE(VLVL,"%s:%s:pkt:    duration %"PRIu64"\n",           SOFT, DIRECTION1, pkt->duration);
	VERBOSE(VLVL,"%s:%s:pkt:    data 0x%02x\n",                  SOFT, DIRECTION1, pkt->data);
	VERBOSE(VLVL,"%s:%s:pkt:    period %"PRIu64"\n",             SOFT, DIRECTION1, pkt->period);
	break;
      }
    case WORLDSENS_C_MEASURE_REQ:
      {
	struct _worldsens_c_measure_req *pkt = (struct _worldsens_c_measure_req *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION1, "WORLDSENS_C_MEASURE_REQ");
	VERBOSE(VLVL,"%s:%s:pkt:    node %d\n",                      SOFT, DIRECTION1, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    measure id %"PRId64"\n",         SOFT, DIRECTION1, pkt->measure_id);
	VERBOSE(VLVL,"%s:%s:pkt:    period %"PRIu64"\n",             SOFT, DIRECTION1, pkt->period);
	break;
      }
    case WORLDSENS_C_DISCONNECT:
      {
	struct _worldsens_c_disconnect *pkt = (struct _worldsens_c_disconnect *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION1, "WORLDSENS_C_DISCONNECT");
	VERBOSE(VLVL,"%s:%s:pkt:    node %d\n",                      SOFT, DIRECTION1, pkt->node_id);
	break;
      }

      /* WSNET2 -> WSIM */
    case WORLDSENS_S_CONNECT_RSP_OK:
      {
	struct _worldsens_s_connect_rsp_ok *pkt = (struct _worldsens_s_connect_rsp_ok *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_CONNECT_RSP_OK");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_next  %"PRIu64"\n",           SOFT, DIRECTION2, pkt->rp_next);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_duration  %"PRIu64"\n",       SOFT, DIRECTION2, pkt->rp_duration);
	VERBOSE(VLVL,"%s:%s:pkt:    nb of antenna  %d\n",            SOFT, DIRECTION2, pkt->n_antenna_id);
	VERBOSE(VLVL,"%s:%s:pkt:    nb of modulation  %d\n",         SOFT, DIRECTION2, pkt->n_modulation_id);
	VERBOSE(VLVL,"%s:%s:pkt:    nb of measure  %d\n",            SOFT, DIRECTION2, pkt->n_measure_id);
	break;
      }
    case WORLDSENS_S_CONNECT_RSP_NOK:
      {
	struct _worldsens_s_connect_rsp_nok *pkt = (struct _worldsens_s_connect_rsp_nok *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_CONNECT_RSP_NOK");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	break;
      }
    case WORLDSENS_S_SYNC_RELEASE:
      {
	struct _worldsens_s_sync_release *pkt = (struct _worldsens_s_sync_release *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_RELEASE");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_next  %"PRIu64"\n",           SOFT, DIRECTION2, pkt->rp_next);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_duration  %"PRIu64"\n",       SOFT, DIRECTION2, pkt->rp_duration);
	break;
      }
    case WORLDSENS_S_BACKTRACK:
      {
	struct _worldsens_s_backtrack *pkt = (struct _worldsens_s_backtrack *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_BACKTRACK");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_next  %"PRIu64"\n",           SOFT, DIRECTION2, pkt->rp_next);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_duration  %"PRIu64"\n",       SOFT, DIRECTION2, pkt->rp_duration);
	break;
      }

    case WORLDSENS_S_BYTE_RX:
      {
	struct _worldsens_s_byte_rx *pkt = (struct _worldsens_s_byte_rx *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_BYTE_RX");
	VERBOSE(VLVL,"%s:%s:pkt:    seq %"PRIu64"\n",                SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    node id %d\n",                   SOFT, DIRECTION2, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    antenna %"PRId64"\n",            SOFT, DIRECTION2, pkt->antenna_id);
	VERBOSE(VLVL,"%s:%s:pkt:    wsim modulation %"PRId64"\n",    SOFT, DIRECTION2, pkt->wsim_mod_id);
	VERBOSE(VLVL,"%s:%s:pkt:    frequence %g hz\n",              SOFT, DIRECTION2, pkt->freq);
	VERBOSE(VLVL,"%s:%s:pkt:    power %g dbm\n",                 SOFT, DIRECTION2, pkt->power_dbm);
	VERBOSE(VLVL,"%s:%s:pkt:    sinr %g dbm\n",                  SOFT, DIRECTION2, pkt->sinr);
	VERBOSE(VLVL,"%s:%s:pkt:    data 0x%02x\n",                  SOFT, DIRECTION2, pkt->data);
	break;
      }
    case WORLDSENS_S_BYTE_SR_RX:
      {
	struct _worldsens_s_byte_sr_rx *pkt = (struct _worldsens_s_byte_sr_rx *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_BYTE_SR_RX");
	VERBOSE(VLVL,"%s:%s:pkt:    seq %"PRIu64"\n",                SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_next  %"PRIu64"\n",           SOFT, DIRECTION2, pkt->rp_next);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_duration  %"PRIu64"\n",       SOFT, DIRECTION2, pkt->rp_duration);
	VERBOSE(VLVL,"%s:%s:pkt:    node id %d\n",                   SOFT, DIRECTION2, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    antenna %"PRId64"\n",            SOFT, DIRECTION2, pkt->antenna_id);
	VERBOSE(VLVL,"%s:%s:pkt:    wsim modulation %"PRId64"\n",    SOFT, DIRECTION2, pkt->wsim_mod_id);
	VERBOSE(VLVL,"%s:%s:pkt:    frequence %g hz\n",              SOFT, DIRECTION2, pkt->freq);
	VERBOSE(VLVL,"%s:%s:pkt:    power %g dbm\n",                 SOFT, DIRECTION2, pkt->power_dbm);
	VERBOSE(VLVL,"%s:%s:pkt:    sinr %g dbm\n",                  SOFT, DIRECTION2, pkt->sinr);
	VERBOSE(VLVL,"%s:%s:pkt:    data 0x%02x\n",                  SOFT, DIRECTION2, pkt->data);
	break;
      }
    case WORLDSENS_S_MEASURE_RSP:
      {
	struct _worldsens_s_measure_rsp *pkt = (struct _worldsens_s_measure_rsp *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_MEASURE_RSP");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    node id %d\n",                   SOFT, DIRECTION2, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    measure id %"PRId64"\n",         SOFT, DIRECTION2, pkt->measure_id);
	VERBOSE(VLVL,"%s:%s:pkt:    measure value %f\n",             SOFT, DIRECTION2, pkt->measure_val);
	break;
      }
    case WORLDSENS_S_MEASURE_SR_RSP:
      {
	struct _worldsens_s_measure_sr_rsp *pkt = (struct _worldsens_s_measure_sr_rsp *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_MEASURE_SR_RSP");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    node id %d\n",                   SOFT, DIRECTION2, pkt->node_id);
	VERBOSE(VLVL,"%s:%s:pkt:    measure id %"PRId64"\n",         SOFT, DIRECTION2, pkt->measure_id);
	VERBOSE(VLVL,"%s:%s:pkt:    measure value %f\n",             SOFT, DIRECTION2, pkt->measure_val);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_next  %"PRIu64"\n",           SOFT, DIRECTION2, pkt->rp_next);
	VERBOSE(VLVL,"%s:%s:pkt:    rp_duration  %"PRIu64"\n",       SOFT, DIRECTION2, pkt->rp_duration);
	break;
      }
    case WORLDSENS_S_KILLSIM:
      {
	struct _worldsens_s_killsim *pkt = (struct _worldsens_s_killsim *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_KILLSIM");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	break;
      }
    case WORLDSENS_S_KILL:
      {
	struct _worldsens_s_kill *pkt = (struct _worldsens_s_kill *)msg;
	VERBOSE(VLVL,"%s:%s:pkt:    type %s\n",                      SOFT, DIRECTION2, "WORLDSENS_S_KILL");
	VERBOSE(VLVL,"%s:%s:pkt:    seq  %"PRIu64"\n",               SOFT, DIRECTION2, pkt->seq);
	VERBOSE(VLVL,"%s:%s:pkt:    node id  %d\n",                  SOFT, DIRECTION2, pkt->node_id);
	break;
      }
    default:
      {
	VERBOSE(VLVL,"%s:pkt:Invalide packet type: %d \n", SOFT, header->type);
      }
    }
  VERBOSE(VLVL,"%s:=======:pkt: stop =====================\n", SOFT);

  return 0;
}



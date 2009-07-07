/**
 *  \file   wsnet2_pkt.c
 *  \brief  WorldSens client v2, packet format
 *  \author Guillaume Chelius, Antoine Fraboulet, Loic Lemaitre
 *  \date   2007
 **/

#include "wsnet2_pkt.h"
#include "wsnet2_net.h"

#define WSIM

#if defined(WSIM)
#include "liblogger/logger.h"
#else
#include <stdio.h>
#define ERROR(x...) fprintf(stderr,x)
#define VERBOSE(x...) fprintf(stderr,x)
#endif

#define UNUSED __attribute__((unused))

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


static inline void swap2(uint8_t *v)
{
  uint16_t r;
  uint8_t *pv = (uint8_t *) v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[1];
  pr[1] = pv[0];
  *(uint16_t *)v = r;
}

static inline void swap4(uint8_t *v)
{
  uint32_t r;
  uint8_t *pv = (uint8_t *) v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[3];
  pr[1] = pv[2];
  pr[2] = pv[1];
  pr[3] = pv[0];
  *(uint32_t *)v = r;
}

static inline void swap8(uint8_t *v)
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
  *(uint64_t *)v = r;
}

#define SWAPN(v)				     \
  do {						     \
    if (sizeof(v)      == 4)			     \
      swap4((uint8_t*)&v);			     \
    else if (sizeof(v) == 8)			     \
      swap8((uint8_t*)&v);			     \
    else if (sizeof(v) == 2)			     \
      swap2((uint8_t*)&v);			     \
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
      SWAPN (pkt->byte_tx.modulation_id);
      SWAPN (pkt->byte_tx.freq);
      SWAPN (pkt->byte_tx.power);
      SWAPN (pkt->byte_tx.duration);
      SWAPN (pkt->byte_tx.data);
      SWAPN (pkt->byte_tx.period);
      break;

    case WORLDSENS_C_MEASURE_REQ:
      SWAPN (pkt->measure_req.type);
      SWAPN (pkt->measure_req.node_id);
      SWAPN (pkt->measure_req.measure_id);
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
      SWAPN (pkt->cnx_rsp_ok.nb_models);
      /* .names field left alone */
      break;

    case WORLDSENS_S_CONNECT_RSP_NOK:
      SWAPN (pkt->cnx_rsp_nok.type);
      SWAPN (pkt->cnx_rsp_nok.seq);
      break;

    case WORLDSENS_S_SYNC_REQ:
      SWAPN (pkt->sync_req.type);
      SWAPN (pkt->sync_req.seq);
      SWAPN (pkt->sync_req.rp_current);
      SWAPN (pkt->sync_req.rp_next);
      SWAPN (pkt->sync_req.rp_duration);
      break;

    case WORLDSENS_S_SYNC_RELEASE:
      SWAPN (pkt->sync_release.type);
      SWAPN (pkt->sync_release.seq);
      SWAPN (pkt->sync_release.rp_current);
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
      SWAPN (pkt->byte_rx.antenna_id);
      SWAPN (pkt->byte_rx.modulation_id);
      SWAPN (pkt->byte_rx.freq);
      SWAPN (pkt->byte_rx.data);
      break;

    case WORLDSENS_S_MEASURE_RSP:
      SWAPN (pkt->measure_rsp.type);
      SWAPN (pkt->measure_rsp.seq);
      SWAPN (pkt->measure_rsp.measure_id);
      SWAPN (pkt->measure_rsp.measure_val);
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

  VERBOSE(VLVL,"Libwsnet2:pkt: start ====================\n");
  switch (header->type)
    {
      /*WSIM->WSNET2*/
    case WORLDSENS_C_CONNECT_REQ:
      {
	struct _worldsens_c_connect_req *pkt = (struct _worldsens_c_connect_req *)msg;
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   type %s\n",              "WORLDSENS_C_CONNECT_REQ");
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   node %d\n",              pkt->node_id);
	break;
      }
    case WORLDSENS_C_SYNC_ACK:
      {
	struct _worldsens_c_sync_ack *pkt = (struct _worldsens_c_sync_ack *)msg;
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   type %s\n",              "WORLDSENS_C_SYNC_ACK");
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   node %d\n",              pkt->node_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   rp_id %d\n",             pkt->rp_id);
	break;
      }
    case WORLDSENS_C_BYTE_TX:
      {
	struct _worldsens_c_byte_tx *pkt = (struct _worldsens_c_byte_tx *)msg;
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   type %s\n",              "WORLDSENS_C_BYTE_TX");
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   node %d\n",              pkt->node_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   antenna %d\n",           pkt->antenna_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   modulation %d\n",        pkt->modulation_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   frequence %f\n",         pkt->freq);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   power %f\n",             pkt->power);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   duration %d\n",          pkt->duration);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   data %02x\n",            pkt->data);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   period %d\n",            pkt->period);
	break;
      }
    case WORLDSENS_C_MEASURE_REQ:
      {
	struct _worldsens_c_measure_req *pkt = (struct _worldsens_c_measure_req *)msg;
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   type %s\n",              "WORLDSENS_C_MEASURE_REQ");
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   node %d\n",              pkt->node_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   measure id %d\n",        pkt->measure_id);
	break;
      }
    case WORLDSENS_C_DISCONNECT:
      {
	struct _worldsens_c_disconnect *pkt = (struct _worldsens_c_disconnect *)msg;
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   type %s\n",              "WORLDSENS_C_DISCONNECT");
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   node %d\n",              pkt->node_id);
	break;
      }

      /* WSNET2 -> WSIM */
    case WORLDSENS_S_CONNECT_RSP_OK:
      {
	struct _worldsens_s_connect_rsp_ok *pkt = (struct _worldsens_s_connect_rsp_ok *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_CONNECT_RSP_OK");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_current  %d\n",       pkt->rp_current);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_next  %d\n",          pkt->rp_next);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_duration  %d\n",      pkt->rp_duration);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    nb of antenna  %d\n",    pkt->n_antenna_id);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    nb of modulation  %d\n", pkt->n_modulation_id);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    nb of measure  %d\n",    pkt->n_measure_id);
	break;
      }
    case WORLDSENS_S_CONNECT_RSP_NOK:
      {
	struct _worldsens_s_connect_rsp_nok *pkt = (struct _worldsens_s_connect_rsp_nok *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_CONNECT_RSP_NOK");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	break;
      }
    case WORLDSENS_S_SYNC_REQ:
      {
	struct _worldsens_s_sync_req *pkt = (struct _worldsens_s_sync_req *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_SYNC_REQ");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_current  %d\n",       pkt->rp_current);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_next  %d\n",          pkt->rp_next);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_duration  %d\n",      pkt->rp_duration);
	break;
      }
    case WORLDSENS_S_SYNC_RELEASE:
      {
	struct _worldsens_s_sync_release *pkt = (struct _worldsens_s_sync_release *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_SYNC_REQ");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_current  %d\n",       pkt->rp_current);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_next  %d\n",          pkt->rp_next);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_duration  %d\n",      pkt->rp_duration);
	break;
      }
    case WORLDSENS_S_BACKTRACK:
      {
	struct _worldsens_s_backtrack *pkt = (struct _worldsens_s_backtrack *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_SYNC_REQ");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_next  %d\n",          pkt->rp_next);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    rp_duration  %d\n",      pkt->rp_duration);
	break;
      }
    case WORLDSENS_S_BYTE_RX:
      {
	struct _worldsens_s_byte_rx *pkt = (struct _worldsens_s_byte_rx *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   type %s\n",              "WORLDSENS_S_BYTE_RX");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   seq %d\n",               pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   antenna %d\n",           pkt->antenna_id);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   modulation %d\n",        pkt->modulation_id);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   frequence %f\n",         pkt->freq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   data %02x\n",            pkt->data);

/* 	int i = 0; */
/* 	while ((wsens.radio[i].callback != NULL) && (i < MAX_CALLBACKS)) { */
/* 	  VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   ****\n"); */
/* 	  VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   node %i\n",              pkt->nodes_infos[i].node_id); */
/* 	  VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   antenna %i\n",           pkt->nodes_infos[i].antenna_id); */
/* 	  VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   ber %d\n",               pkt->nodes_infos[i].ber); */
/* 	  VERBOSE(VLVL,"Libwsnet2:rcv:pkt:   power %d\n",             pkt->nodes_infos[i].power); */
/* 	  i++; */
/* 	} */
	
	break;
      }
    case WORLDSENS_S_MEASURE_RSP:
      {
	struct _worldsens_s_measure_rsp *pkt = (struct _worldsens_s_measure_rsp *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_MEASURE_RSP");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   measure id %d\n",        pkt->measure_id);
	VERBOSE(VLVL,"Libwsnet2:sent:pkt:   measure value %f\n",     pkt->measure_val);
	break;
      }
    case WORLDSENS_S_KILLSIM:
      {
	struct _worldsens_s_killsim *pkt = (struct _worldsens_s_killsim *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_KILLSIM");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	break;
      }
    case WORLDSENS_S_KILL:
      {
	struct _worldsens_s_kill *pkt = (struct _worldsens_s_kill *)msg;
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    type %s\n",              "WORLDSENS_S_KILL");
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    seq  %d\n",              pkt->seq);
	VERBOSE(VLVL,"Libwsnet2:rcv:pkt:    node id  %d\n",          pkt->node_id);
	break;
      }
    default:
      {
	VERBOSE(VLVL,"Libwsnet2:pkt:Invalide packet type: %d \n",header->type);
      }
    }
  VERBOSE(VLVL,"Libwsnet2:pkt: stop =====================\n");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


uint64_t ntohll  (uint64_t v)
{
  SWAPN(v);
  return v;
}

uint64_t htonll  (uint64_t v)
{
  SWAPN(v);
  return v;
}

double   ntohdbl (double v)
{
  SWAPN(v);
  return v;
}

double   htondbl (double v)
{
  SWAPN(v);
  return v;
}

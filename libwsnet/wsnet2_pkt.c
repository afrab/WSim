/**
 *  \file   wsnet2_pkt.c
 *  \brief  WorldSens client v2, packet format
 *  \author Guillaume Chelius, Antoine Fraboulet
 *  \date   2007
 **/

#include "wsnet2_pkt.h"

#if defined(WSIM)
#include "liblogger/logger.h"
#else
#include <stdio.h>
#define ERROR(x...) fprintf(stderr,x)
#endif

#define UNUSED __attribute__((unused))

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define WORLDSENS_USES_LITTLEENDIAN 1

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
  uint8_t *pv = (uint8_t *) &v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[1];
  pr[1] = pv[0];
  *(uint16_t*)v = r;
}

static inline void swap4(uint8_t *v)
{
  uint32_t r;
  uint8_t *pv = (uint8_t *) &v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[3];
  pr[1] = pv[2];
  pr[2] = pv[1];
  pr[3] = pv[0];
  *(uint32_t*)v = r;
}

static inline void swap8(uint8_t *v)
{
  uint64_t r;
  uint8_t *pv = (uint8_t *) &v;
  uint8_t *pr = (uint8_t *) &r;
  pr[0] = pv[7];
  pr[1] = pv[6];
  pr[2] = pv[5];
  pr[3] = pv[4];
  pr[4] = pv[3];
  pr[5] = pv[2];
  pr[6] = pv[1];
  pr[7] = pv[0];
  *(uint64_t*)v = r;
}

#define SWAPN(v)                     \
do {                                 \
  if (sizeof(v)      == 4)           \
    swap4((uint8_t*)&v);             \
  else if (sizeof(v) == 8)           \
    swap8((uint8_t*)&v);             \
  else if (sizeof(v) == 2)           \
    swap2((uint8_t*)&v);             \
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
      SWAPN (pkt->cnx_req.pkt_id);
      SWAPN (pkt->cnx_req.node_id);
      break;

    case WORLDSENS_C_SYNC_ACK:
      SWAPN (pkt->sync_ack.pkt_id);
      SWAPN (pkt->sync_ack.node_id);
      SWAPN (pkt->sync_ack.rp_id);
      break;

    case WORLDSENS_C_BYTE_TX:
      SWAPN (pkt->byte_tx.pkt_id);
      SWAPN (pkt->byte_tx.node_id);
      SWAPN (pkt->byte_tx.antenna_id);
      SWAPN (pkt->byte_tx.modulation_id);
      SWAPN (pkt->byte_tx.freq);
      SWAPN (pkt->byte_tx.power);
      SWAPN (pkt->byte_tx.duration);
      SWAPN (pkt->byte_tx.data);
      break;

    case WORLDSENS_C_MEASURE_REQ:
      SWAPN (pkt->measure_req.pkt_id);
      SWAPN (pkt->measure_req.measure_id);
      break;

    case WORLDSENS_C_DISCONNECT:
      SWAPN (pkt->disconnect.pkt_id);
      SWAPN (pkt->disconnect.node_id);
      break;

    case WORLDSENS_S_CONNECT_RSP_OK:
      SWAPN (pkt->cnx_rsp.pkt_id);
      SWAPN (pkt->cnx_rsp.seq);
      SWAPN (pkt->cnx_rsp.rp_next);
      SWAPN (pkt->cnx_rsp.rp_duration);
      /* .names field left alone */
      break;

    case WORLDSENS_S_SYNC_REQ:
      SWAPN (pkt->sync_req.pkt_id);
      SWAPN (pkt->sync_req.seq);
      SWAPN (pkt->sync_req.rp_next);
      SWAPN (pkt->sync_req.rp_duration);
      break;

    case WORLDSENS_S_SYNC_RELEASE:
      SWAPN (pkt->sync_release.pkt_id);
      SWAPN (pkt->sync_release.seq);
      SWAPN (pkt->sync_release.rp_next);
      SWAPN (pkt->sync_release.rp_duration);
      break;

    case WORLDSENS_S_BYTE_RX:
      SWAPN (pkt->byte_rx.pkt_id);
      SWAPN (pkt->byte_rx.seq);
      SWAPN (pkt->byte_rx.antenna_id);
      SWAPN (pkt->byte_rx.modulation_id);
      SWAPN (pkt->byte_rx.freq);
      SWAPN (pkt->byte_rx.power);
      SWAPN (pkt->byte_rx.data);
      SWAPN (pkt->byte_rx.ber);
      break;

    case WORLDSENS_S_MEASURE_RSP:
      SWAPN (pkt->measure_rsp.pkt_id);
      SWAPN (pkt->measure_rsp.seq);
      SWAPN (pkt->measure_rsp.measure_id);
      SWAPN (pkt->measure_rsp.measure_val);
      break;

    case WORLDSENS_S_KILLSIM:
      SWAPN (pkt->killsim.pkt_id);
      SWAPN (pkt->killsim.seq);
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

int worldsens_packet_dump(union _worldsens_pkt UNUSED *pkt)
{
  
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


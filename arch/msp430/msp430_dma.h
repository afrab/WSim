/**
 *  \file   msp430_dma.h
 *  \brief  MSP430 DMA engine
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_DMA_H
#define MSP430_DMA_H

#if defined(__msp430_have_dma)

#define DMA_START 0x1E0
#define DMA_END   0x1F6

enum dma_addr_t {
  DMACTL0      = 0x0122,
  DMACTL1      = 0x0124,

  DMA0CTL      = 0x01E0,
  DMA0SA       = 0x01E2,
  DMA0DA       = 0x01E4,
  DMA0SZ       = 0x01E6,

  DMA1CTL      = 0x01E8,
  DMA1SA       = 0x01EA,
  DMA1DA       = 0x01EC,
  DMA1SZ       = 0x01EE,

  DMA2CTL      = 0x01F0,
  DMA2SA       = 0x01F2,
  DMA2DA       = 0x01F4,
  DMA2SZ       = 0x01F6
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) dmactl0_t {
  uint16_t
    reserved:4,
    dma2_tselx:4,
    dma1_tselx:4,
    dma0_tselx:4;
};
#else
struct __attribute__ ((packed)) dmactl0_t {
  uint16_t
    dma0_tselx:4,
    dma1_tselx:4,
    dma2_tselx:4,
    reserved:4;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) dmactl1_t {
  uint16_t
    reserved:13,
    dma_onfetch:1,
    round_robin:1,
    ennmi:1;
};
#else
struct __attribute__ ((packed)) dmactl1_t {
  uint16_t
    ennmi:1,
    round_robin:1,
    dma_onfetch:1,
    reserved:13;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) dmaXctl_t {
  uint16_t
    reserved:1,
    dt:3,
    dstincr:2,
    srcincr:2,
    dstbyte:1,
    srcbyte:1,
    level:1,
    en:1,
    ifg:1,
    ie:1,
    abort:1,
    req:1;
};
#else
struct __attribute__ ((packed)) dmaXctl_t {
  uint16_t
    req:1,
    abort:1,
    ie:1,
    ifg:1,
    en:1,
    level:1,
    srcbyte:1,
    dstbyte:1,
    srcincr:2,
    dstincr:2,
    dt:3,
    reserved:1;
};
#endif

struct dmaXchannel_t {
  union {
    struct dmaXctl_t  b;
    uint16_t          s;
  } dmaXctl;

  uint16_t dmaXsa;
  uint16_t dmaXda;
  uint16_t dmaXsz;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct msp430_dma_t {
  union {
    struct dmactl0_t  b;
    uint16_t          s;
  } dmactl0;

  union {
    struct dmactl1_t  b;
    uint16_t          s;
  } dmactl1;

  struct dmaXchannel_t channel[3];
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_dma_reset ();
void    msp430_dma_update();
int16_t msp430_dma_read  (uint16_t addr);
void    msp430_dma_write (uint16_t addr, int16_t val);
int     msp430_dma_chkifg();

#endif /* have_dma */
#endif

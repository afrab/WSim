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

enum dma_state_t {
  DMA_RESET = 0, 
  DMA_IDLE  = 1,
  DMA_WAIT  = 2, 
  DMA_HOLD  = 3, 
  DMA_DEC   = 4,
  DMA_BURST = 5
};

struct dmaXchannel_t {
  union {
    struct dmaXctl_t  b;
    uint16_t          s;
  } dmaXctl;

  uint16_t dmaXsa;
  uint16_t dmaXda;
  uint16_t dmaXsz;

  uint16_t t_size;
  uint16_t t_srcadd;
  uint16_t t_dstadd;
  
  enum dma_state_t state; 
  int      mclk_wait;
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
  uint32_t           dma_triggers;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_dma_reset ();
void    msp430_dma_update();
int16_t msp430_dma_read  (uint16_t addr);
void    msp430_dma_write (uint16_t addr, int16_t val);
int     msp430_dma_chkifg();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if 0
/* 00  x */"DMAREQ bit (software trigger)"
/*       */"TACCR2 CCIFG bit"
/*       */"TBCCR2 CCIFG bit"
/*     x */"URXIFG0 (UART/SPI mode), USART0 data received (I2C mode)"
/*     x */"UTXIFG0 (UART/SPI mode), USART0 transmit ready (I2C mode)"
/* 05    */"DAC12_0CTL DAC12IFG bit"
/*       */"ADC12 ADC12IFGx bit"
/*       */"TACCR0 CCIFG bit"
/*       */"TBCCR0 CCIFG bit"
/*     x */"URXIFG1 bit"
/* 10  x */"UTXIFG1 bit"
/*       */"Multiplier ready"
/*     x */"No action"
/*     x */"No action"
/*     x */"DMAxIFG bit triggers DMA channel x"
/* 15  x */"External trigger DMAE0"
/*     x */"DMA2IFG bit triggers DMA channel 0"
/*     x */"DMA0IFG bit triggers DMA channel 1"
/*     x */"DMA1IFG bit triggers DMA channel 2"
#endif

#define DMA_SET_TRIGGER(trig)			\
  do {						\
    MCU_DMA.dma_triggers |= trig;		\
  } while(0)

#define DMA_CLEAR_TRIGGERS()			\
  do {						\
    MCU_DMA.dma_triggers = 0;			\
  } while (0)

#define DMA_TRIG_DMAREQ    	(1 <<  0)
#define DMA_TRIG_TACCR2    	(1 <<  1)
#define DMA_TRIG_TBCCR2    	(1 <<  2)
#define DMA_TRIG_URXIFG0   	(1 <<  3)
#define DMA_TRIG_UTXIFG0   	(1 <<  4)
#define DMA_TRIG_DAC12IFG  	(1 <<  5)
#define DMA_TRIG_ADC12IFG  	(1 <<  6)
#define DMA_TRIG_TACCR0    	(1 <<  7)
#define DMA_TRIG_TBCCR0    	(1 <<  8)
#define DMA_TRIG_URXIFG1   	(1 <<  9)
#define DMA_TRIG_UTXIFG1   	(1 << 10)
#define DMA_TRIG_MULTIPLIER	(1 << 11)
#define DMA_TRIG_NOACTION0 	(1 << 12)
#define DMA_TRIG_NOACTION1 	(1 << 13)
#define DMA_TRIG_DMAxIFG 	(1 << 14) /* should no be used */
#define DMA_TRIG_DMAE0  	(1 << 15)
/* replaces DMA_TRIG_DMAxIFG for channels 0, 1 and 2 */
#define DMA_TRIG_DMA2IFG        (1 << 16) /* channel 0 */
#define DMA_TRIG_DMA0IFG        (1 << 17) /* channel 1 */
#define DMA_TRIG_DMA1IFG        (1 << 18) /* channel 2 */

#define DMA_SET_REQ()           DMA_SET_TRIGGER( DMA_TRIG_DMAREQ     )
#define DMA_SET_TACCR2()        DMA_SET_TRIGGER( DMA_TRIG_TACCR2     )
#define DMA_SET_TBCCR2()        DMA_SET_TRIGGER( DMA_TRIG_TBCCR2     )
#define DMA_SET_URXIFG0()       DMA_SET_TRIGGER( DMA_TRIG_URXIFG0    )
#define DMA_SET_UTXIFG0()       DMA_SET_TRIGGER( DMA_TRIG_UTXIFG0    )
#define DMA_SET_DAC12IFG()      DMA_SET_TRIGGER( DMA_TRIG_DAC12IFG   )
#define DMA_SET_ADC12IFG()      DMA_SET_TRIGGER( DMA_TRIG_ADC12IFG   )
#define DMA_SET_TACCR0()        DMA_SET_TRIGGER( DMA_TRIG_TACCR0     )
#define DMA_SET_TBCCR0()        DMA_SET_TRIGGER( DMA_TRIG_TBCCR0     )
#define DMA_SET_URXIFG1()       DMA_SET_TRIGGER( DMA_TRIG_URXIFG1    )
#define DMA_SET_UTXIFG1()       DMA_SET_TRIGGER( DMA_TRIG_UTXIFG1    )
#define DMA_SET_MULTIPLIER()    DMA_SET_TRIGGER( DMA_TRIG_MULTIPLIER )
#define DMA_SET_NOACTION0()     DMA_SET_TRIGGER( DMA_TRIG_NOACTION0  )
#define DMA_SET_NOACTION1()     DMA_SET_TRIGGER( DMA_TRIG_NOACTION1  )
#define DMA_SET_DMAxIFG()       ERROR("msp430:dma: DMA_SET_DMAxIFG() should not be used\n")
#define DMA_SET_DMAE0()         DMA_SET_TRIGGER( DMA_TRIG_DMAE0      )

#define DMA_SET_DMA2IFG()       DMA_SET_TRIGGER( DMA_TRIG_DMA2IFG    )
#define DMA_SET_DMA0IFG()       DMA_SET_TRIGGER( DMA_TRIG_DMA0IFG    )
#define DMA_SET_DMA1IFG()       DMA_SET_TRIGGER( DMA_TRIG_DMA1IFG    )

#define DMA_SET_DMAxIFG_FOR_CHANN( chann )	\
  do {						\
    DMA_SET_TRIGGER( 1 << (16 + chann ) );	\
  } while(0)

#define DMA_SET_DMAxIFG_FROM_CHANN( chann )	\
  do {						\
    int target = (chann + 1) % 3;		\
    DMA_SET_TRIGGER( 1 << (16 + target ));	\
  } while(0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else  /* have_dma */

#define DMA_SET_REQ()           do { } while (0)
#define DMA_SET_TACCR2()        do { } while (0)
#define DMA_SET_TBCCR2()        do { } while (0)
#define DMA_SET_URXIFG0()       do { } while (0)
#define DMA_SET_UTXIFG0()       do { } while (0)
#define DMA_SET_DAC12IFG()      do { } while (0)
#define DMA_SET_ADC12IFG()      do { } while (0)
#define DMA_SET_TACCR0()        do { } while (0)
#define DMA_SET_TBCCR0()        do { } while (0)
#define DMA_SET_URXIFG1()       do { } while (0)
#define DMA_SET_UTXIFG1()       do { } while (0)
#define DMA_SET_MULTIPLIER()    do { } while (0)
#define DMA_SET_NOACTION0()     do { } while (0)
#define DMA_SET_NOACTION1()     do { } while (0)
#define DMA_SET_DMAxIFG()       ERROR("msp430:dma: DMA_SET_DMAxIFG() should not be used\n")
#define DMA_SET_DMAE0()         do { } while (0)

#endif /* have_dma */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif

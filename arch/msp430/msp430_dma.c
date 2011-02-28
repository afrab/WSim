/**
 *  \file   msp430_dma.c
 *  \brief  MSP430 DMA engine
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_dma)

#define DMA_CHANNELS 3

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_dma_reset()
{
  int chann;

  MCU_DMA.dmactl0.s    = 0;
  MCU_DMA.dmactl1.s    = 0;
  for(chann=0; chann < DMA_CHANNELS; chann++)
    {
      MCU_DMA.channel[ chann ].dmaXctl.s = 0;
      /* SA, DA and SZ are not changed on Reset */
      /* MCU_DMA.channel[ chann ].dmaXsa    = 0;      */
      /* MCU_DMA.channel[ chann ].dmaXda    = 0;      */
      /* MCU_DMA.channel[ chann ].dmaXsz    = 0;      */
      MCU_DMA.channel[ chann ].state    = DMA_RESET;
      MCU_DMA.channel[ chann ].t_size   = 0;
      MCU_DMA.channel[ chann ].t_srcadd = 0;
      MCU_DMA.channel[ chann ].t_dstadd = 0;
    }
  DMA_CLEAR_TRIGGERS();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline int DMA_TRIG_IS_SET(int UNUSED chann, int UNUSED trig)
{
  if (trig == 14) /* DMA_TRIG_DMAxIFG */
    return MCU_DMA.dma_triggers & (1 << (16+chann));

  return MCU_DMA.dma_triggers & (1 << trig);
}


void msp430_dma_update()
{
  int chann;
  int incr[] = { 0, 0, -1, +1 };

  for(chann=0; chann < DMA_CHANNELS; chann++)
    {
      /* if (MCU_DMA.channel[ chann ].dmaXctl.b.en) */
	{
	  /* channel        */
	  struct dmaXchannel_t *pchan = & MCU_DMA.channel[ chann ];
	  /* transfer mode  */
	  int UNUSED single = (pchan->dmaXctl.b.dt & 0x1) == 0x0;
	  int block  = (pchan->dmaXctl.b.dt & 0x1) == 0x1;
	  int burst  = (pchan->dmaXctl.b.dt & 0x2) == 0x2;
	  int repeat = (pchan->dmaXctl.b.dt & 0x4) == 0x4;
	  
	  if (block) ERROR("msp430:dma:channel %d: BLOCK mode not implemented\n", chann);
	  if (burst) ERROR("msp430:dma:channel %d: BURST mode not implemented\n", chann);
	  
	  switch (pchan->state) 
	    {
	      /***********/
	    case DMA_RESET:
	      /***********/
	      if (pchan->dmaXctl.b.en == 1)
		{
		  pchan->state         = DMA_IDLE;
		  pchan->t_srcadd      = pchan->dmaXsa;
		  pchan->t_dstadd      = pchan->dmaXda;
		  pchan->t_size        = pchan->dmaXsz;

		  if (pchan->dmaXsz == 0)
		    {
		      pchan->state         = DMA_RESET;
		      pchan->dmaXctl.b.ifg = 1;
		      pchan->dmaXctl.b.en  = 0;
		      WARNING("msp430:dma:channel %d: starting DMA with transfer size == 0\n", chann);
		      break; /* break switch */
		    }
		}
	      /* fall */

	      /***********/
	    case DMA_IDLE:
	      /***********/
	      if (pchan->dmaXctl.b.abort == 0)
		{
		  pchan->state = DMA_WAIT;
		}
	      /* fall */

	      /***********/
	    case DMA_WAIT:
	      /***********/
	      {
		int trig = (MCU_DMA.dmactl0.s >> ( chann * 4)) & 0x0f;
		if (DMA_TRIG_IS_SET( chann, trig ))
		  {
		    pchan->state     = DMA_HOLD;
		    pchan->mclk_wait = 2;
		  }
	      }
	      break; /* don't fall, wait 2 MCLK cycles */

	      /***********/
	    case DMA_HOLD:
	      /***********/
	      /* wait 2 MCLK done -> copy */
	      if (pchan->dmaXctl.b.srcbyte) 
		{
		  uint8_t byte = msp430_read_byte ( pchan->t_srcadd );
		  if (pchan->dmaXctl.b.dstbyte) 
		    {
		      msp430_write_byte ( pchan->t_dstadd, byte );
		    }
		  else
		    {
		      msp430_write_short( pchan->t_dstadd, 0x0000 | byte );
		    }
		}
	      else
		{
		  uint16_t word = msp430_read_short( pchan->t_srcadd );
		  if (pchan->dmaXctl.b.dstbyte) 
		    {
		      msp430_write_byte ( pchan->t_dstadd, word & 0x00ff );
		    }
		  else
		    {
		      msp430_write_short( pchan->t_dstadd, word );
		    }
		}
	      pchan->state = DMA_DEC;
	      /* fall */

	      /***********/
	    case DMA_DEC:
	      /***********/
	      {
		/* increment mode */
		int srcadd = incr[ pchan->dmaXctl.b.srcincr ] * ( 2 - pchan->dmaXctl.b.srcbyte );
		int dstadd = incr[ pchan->dmaXctl.b.dstincr ] * ( 2 - pchan->dmaXctl.b.dstbyte ); 

		pchan->dmaXsz --;
		pchan->t_srcadd += srcadd;
		pchan->t_dstadd += dstadd;
		
		if (! MCU_DMA.channel[ chann ].dmaXctl.b.en)
		  {
		    pchan->state = DMA_RESET;
		  }
		else if (pchan->dmaXsz > 0)
		  {
		    pchan->dmaXctl.b.ifg     = 1;
		    if (! repeat)
		      {
			pchan->dmaXctl.b.en  = 0;
			pchan->state         = DMA_RESET;
		      }
		    else
		      {
			pchan->t_srcadd      = pchan->dmaXsa;
			pchan->t_dstadd      = pchan->dmaXda;
			pchan->dmaXsz        = pchan->t_size;
			pchan->state         = DMA_WAIT;
			pchan->dmaXctl.b.req = 0;
		      }
		  }
		else /* dmaXsz > 0 */
		  {
		    pchan->state         = DMA_WAIT;
		    pchan->dmaXctl.b.req = 0;
		  }
	      }
	      break;
		
	      /***********/
	    case DMA_BURST:
	      /***********/
	      break;
	    }
	}
    }

  DMA_CLEAR_TRIGGERS();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_dma_read  (uint16_t addr)
{
  int16_t val = 0;

  if (addr == DMACTL0)
    {
      val = MCU_DMA.dmactl0.s;
      HW_DMSG_DMA("msp430:dma: read  DMACTL0 = 0x%04x\n", val);
    }
  else if (addr == DMACTL1)
    {
      val = MCU_DMA.dmactl1.s;
      HW_DMSG_DMA("msp430:dma: read  DMACTL1 = 0x%04x\n", val);
    }
  else if (DMA0CTL <= addr && addr <= DMA2SZ)
    {
      int chann = 0;
      int base  = 0;
      struct dmaXchannel_t *pchan; 

      if       (DMA0CTL <= addr && addr <= DMA0SZ)   chann = 0;
      else if  (DMA1CTL <= addr && addr <= DMA1SZ)   chann = 1;
      else if  (DMA2CTL <= addr && addr <= DMA2SZ)   chann = 2;

      pchan = & MCU_DMA.channel[chann];

      base = DMA0CTL + (chann * 8); /* 4 * 2bytes registers */
      switch (addr - base)
	{
	case 0:
	  val = pchan->dmaXctl.s;
	  HW_DMSG_DMA("msp430:dma:chann %d: read  DMAxCTL = 0x%04x\n", chann, val);
	  break;
	case 2:
	  val = pchan->dmaXsa;
	  HW_DMSG_DMA("msp430:dma:chann %d: read  DMAxSA  = 0x%04x\n", chann, val);
	  break;
	case 4:
	  val = pchan->dmaXda;
	  HW_DMSG_DMA("msp430:dma:chann %d: read  DMAxDA  = 0x%04x\n", chann, val);
	  break;
	case 6:
	  val = pchan->dmaXsz;
	  HW_DMSG_DMA("msp430:dma:chann %d: read  DMAxSZ  = 0x%04x\n", chann, val);
	  break;
	default:
	  ERROR("msp430:dma: read  [0x%04x] = 0x%04x, addr error\n",
		addr,val,mcu_get_pc());
	  break;
	}
    }
  else
    {
      ERROR("msp430:dma: read  [0x%04x] = 0x%04x, block not implemented (PC=0x%04x)\n",
	    addr,val,mcu_get_pc());
    }

  return val;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* msp430_trig_str(int chann, int sel)
{
  char UNUSED *trig_str[] = {
/* 00 */"DMAREQ bit (software trigger)",
	"TACCR2 CCIFG bit",
	"TBCCR2 CCIFG bit",
	"URXIFG0 (UART/SPI mode), USART0 data received (I2C mode)",
	"UTXIFG0 (UART/SPI mode), USART0 transmit ready (I2C mode)",
/* 05 */"DAC12_0CTL DAC12IFG bit",
	"ADC12 ADC12IFGx bit",
	"TACCR0 CCIFG bit",
	"TBCCR0 CCIFG bit",
	"URXIFG1 bit",
/* 10 */"UTXIFG1 bit",
	"Multiplier ready",
	"No action",
	"No action",
	"DMAxIFG bit triggers DMA channel x",
/* 15 */"External trigger DMAE0",
	"DMA2IFG bit triggers DMA channel 0",
	"DMA0IFG bit triggers DMA channel 1",
	"DMA1IFG bit triggers DMA channel 2"
  };

  if (sel == 14) /* DMA_TRIG_DMAxIFG */
    return trig_str[16+chann];

  return trig_str[sel];
}

/* ************************************************** */

void msp430_dmaxctl_str(int UNUSED chann, struct dmaXctl_t UNUSED *old, struct dmaXctl_t UNUSED *new)
{
  char UNUSED *incr_str[] = { "", "", "--", "++" };
  char UNUSED *mode_str[] = {
    "Single transfer",
    "Block transfer",
    "Burst-block transfer",
    "Burst-block transfer",
    "Repeated single transfer",
    "Repeated block transfer",
    "Repeated burst-block transfer",
    "Repeated burst-block transfer"
  };

  HW_DMSG_DMA("msp430:dma:chann %d:    -- %s + %s\n", chann,
	      (new->en)?"enable":"disable",
	      (new->ie)?"ie":"no irq");
  HW_DMSG_DMA("msp430:dma:chann %d:    -- *(%s*)dst%s=*(%s*)src%s\n", chann,
	      (new->dstbyte)?"char":"short",
	      incr_str[new->dstincr],
	      (new->srcbyte)?"char":"short",
	      incr_str[new->srcincr]);
  HW_DMSG_DMA("msp430:dma:chann %d:    -- %s, %s\n", chann,
	      mode_str[new->dt],
	      (new->level)?"level":"edge");
}

/* ************************************************** */

void msp430_dma_write (uint16_t addr, int16_t val)
{
  int i;
  if (addr == DMACTL0)
    {
      union {
	struct dmactl0_t      b;
	uint16_t              s;
      } dmactl0;
      dmactl0.s = val;
      HW_DMSG_DMA("msp430:dma: write DMACTL0 = 0x%04x\n", val);
      if (MCU_DMA.dmactl0.s != dmactl0.s)
	{
	  /* 0xX210 channel configurations */
	  for(i=0; i < DMA_CHANNELS; i++)
	    {
	      int conf_msk = 0x0f << (i*4);
	      if ((MCU_DMA.dmactl0.s & conf_msk) != (dmactl0.s & conf_msk))
		{
		  int UNUSED cnf = (dmactl0.s & conf_msk) >> (i*4);
		  HW_DMSG_DMA("msp430:dma:    -- configuration for channel %i = %s\n",
			      i, msp430_trig_str(i,cnf));
		}
	    }
	}
      MCU_DMA.dmactl0.s = dmactl0.s;
    }
  else if (addr == DMACTL1)
    {
      union {
	struct dmactl1_t      b;
	uint16_t              s;
      } dmactl1;
      dmactl1.s = val;
      HW_DMSG_DMA("msp430:dma: write DMACTL1 = 0x%04x\n", val);
      if (MCU_DMA.dmactl1.s != dmactl1.s)
	{
	  HW_DMSG_DMA("msp430:dma:    -- (onfetch, r-robin, ennmi) = (%d,%d,%d)\n",
		      dmactl1.b.dma_onfetch , dmactl1.b.round_robin , dmactl1.b.ennmi);
	}
      MCU_DMA.dmactl1.s = dmactl1.s;
    }
  else if (DMA0CTL <= addr && addr <= DMA2SZ)
    {
      int chann = 0;
      int base  = 0;
      struct dmaXchannel_t *pchan; 

      if       (DMA0CTL <= addr && addr <= DMA0SZ)   chann = 0;
      else if  (DMA1CTL <= addr && addr <= DMA1SZ)   chann = 1;
      else if  (DMA2CTL <= addr && addr <= DMA2SZ)   chann = 2;

      pchan = & MCU_DMA.channel[chann];

      base = DMA0CTL + (chann * 8); /* 4 * 2bytes registers */
      switch (addr - base)
	{
	case 0:
	  {
	    union {
	      struct dmaXctl_t      b;
	      uint16_t              s;
	    } dmaxctl;
	    dmaxctl.s = val;
	    HW_DMSG_DMA("msp430:dma:chann %d: write DMAxCTL = 0x%04x\n", chann, val);
	    if (pchan->dmaXctl.s != dmaxctl.s)
	      {
		msp430_dmaxctl_str(chann,&pchan->dmaXctl.b,&dmaxctl.b);
		if (dmaxctl.b.en && !pchan->dmaXctl.b.en)
		  {
		    HW_DMSG_DMA("msp430:dma:chann %d:    +- en  :: enable DMA\n", chann);
		  }
		if (dmaxctl.b.req && !pchan->dmaXctl.b.req)
		  {
		    HW_DMSG_DMA("msp430:dma:chann %d:    +- req :: start DMA\n", chann);
		  }
		if (dmaxctl.b.ifg && !pchan->dmaXctl.b.ifg)
		  {
		    HW_DMSG_DMA("msp430:dma:chann %d:    +- ifg :: interrupt request\n", chann);
		  }
	      }
	    pchan->dmaXctl.s = dmaxctl.s;
	  }
	  break;
	case 2:
	  HW_DMSG_DMA("msp430:dma:chann %d: write DMAxSA = 0x%04x\n", chann, val);
	  pchan->dmaXsa = val;
	  break;
	case 4:
	  HW_DMSG_DMA("msp430:dma:chann %d: write DMAxDA = 0x%04x\n", chann, val);
	  pchan->dmaXda = val;
	  break;
	case 6:
	  HW_DMSG_DMA("msp430:dma:chann %d: write DMAxSZ = 0x%04x\n", chann, val);
	  pchan->dmaXsz = val;
	  break;
	default:
	  ERROR("msp430:dma: write [0x%04x] = 0x%04x, addr error\n",
		addr,val,mcu_get_pc());
	  break;
	}
    }
  else
    {
      ERROR("msp430:dma: write [0x%04x] = 0x%04x, block not implemented (PC=0x%04x)\n",
	    addr,val,mcu_get_pc());
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_dma_chkifg()
{
  int i;
  for(i=0; i< DMA_CHANNELS; i++)
    {
      if (MCU_DMA.channel[i].dmaXctl.b.ie && MCU_DMA.channel[i].dmaXctl.b.ifg)
	{
	  msp430_interrupt_set(INTR_DMA);
	  return 1;
	}
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif


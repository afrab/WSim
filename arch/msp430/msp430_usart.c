
/**
 *  \file   msp430_usart.c
 *  \brief  MSP430 USART definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h> 
#include <string.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/*****************************************************/
/*****************************************************/
/*****************************************************/

#define HW_SPY_MODE 0

#if HW_SPY_MODE != 0
#define HW_SPY(x...) HW_DMSG_MCUDEV(x)
#else
#define HW_SPY(x...) do { } while (0)
#endif


/* defined in msp430_debug.h */
#if defined(DEBUG_USART)  
char *str_ssel[] = { "external UCLK", "ACLK", "SMCLK", "SMCLK" };
#endif

/*****************************************************/
/*****************************************************/
/*****************************************************/

#define TRACER_UART_IDLE     0x00 

#define TRACER_UART_TX_RECV  0x01  /* data written to TX buff    */
#define TRACER_UART_TX_READY 0x02  /* data written to TX shifter */
#define TRACER_UART_RX_RECV  0x01  /* data written to Rx shifter */
#define TRACER_UART_RX_READY 0x02  /* data written to Rx buff    */

#define TRACER_SPI_TX_RECV   0x11
#define TRACER_SPI_TX_READY  0x12
#define TRACER_SPI_RX_RECV   0x11
#define TRACER_SPI_RX_READY  0x12

#define TRACER_I2C_TX_RECV   0x21
#define TRACER_I2C_TX_READY  0x22
#define TRACER_I2C_RX_RECV   0x21
#define TRACER_I2C_RX_READY  0x22

/*****************************************************/
/*****************************************************/
/*****************************************************/





/**
 * [slau049e.pdf, page 262]
 *
 * In asynchronous mode, the USART connects the MSP430 to an external
 * system via two external pins, URXD and UTXD. UART mode is selected when
 * the SYNC bit is cleared.
 *
 *
 *   UART mode features include:
 *
 * - 7- or 8-bit data with odd, even, or non-parity
 * - Independent transmit and receive shift registers
 * - Separate transmit and receive buffer registers
 * - LSB-first data transmit and receive
 * - Built-in idle-line and address-bit communication protocols for
 *    multiprocessor systems
 * - Receiver start-edge detection for auto-wake up from LPMx modes
 * - Programmable baud rate with modulation for fractional baud rate support
 * - Status flags for error detection and suppression and address detection
 * - Independent interrupt capability for receive and transmit
 **/

/**
 * In synchronous mode, the USART connects the MSP430 to an external
 * system via three or four pins: SIMO, SOMI, UCLK, and STE. SPI mode is
 * selected when the SYNC bit is set and the I2C bit is cleared.
 *
 * SPI mode features include:
 * - 7- or 8-bit data length
 * - 3-pin and 4-pin SPI operation
 * - Master or slave modes
 * - Independent transmit and receive shift registers
 * - Separate transmit and receive buffer registers
 * - Selectable UCLK polarity and phase control
 * - Programmable UCLK frequency in master mode
 * - Independent interrupt capability for receive and transmit
 * Figure 14-1 shows the USART when configured for SPI mode.
 **/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define USART_RESET(USART)                                                    \
  MCU.USART.uxrxbuf            = 0;					      \
  MCU.USART.uxrxbuf_full       = 0;                                           \
  MCU.USART.uxrx_shift_buf     = 0;                                           \
  MCU.USART.uxrx_shift_empty   = 1;                                           \
  MCU.USART.uxrx_shift_ready   = 0;                                           \
  MCU.USART.uxrx_shift_delay   = 0;                                           \
  MCU.USART.uxrx_slave_rx_done = 0;					      \
                                                                              \
  MCU.USART.uxtxbuf            = 0;                                           \
  MCU.USART.uxtxbuf_full       = 0;                                           \
  MCU.USART.uxtx_full_delay    = 0;                                           \
  MCU.USART.uxtctl.b.txept     = 1;                                           \
  MCU.USART.uxtx_shift_buf     = 0;                                           \
  MCU.USART.uxtx_shift_empty   = 1;                                           \
  MCU.USART.uxtx_shift_ready   = 0;                                           \
  MCU.USART.uxtx_shift_delay   = 0;                                           \
                                                                              \
  MCU.USART.mode             = USART_MODE_UART;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* SPI mode                                                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************/                                     
/**** SPI transmit enable ****/                                     
/* 
   TODO: SPI à vérifer : baisser urxex pendant que rx shift != empty arrête réception ?? 
   TODO: AFR: check that SPI SPEED is at most UBRCLK/2
      HW_SPY("msp430:usart%d:spi:br0   %x\n",NUM,MCU.USART.uxbr0);
      HW_SPY("msp430:usart%d:spi:br1   %x\n",NUM,MCU.USART.uxbr1);
*/

#define SPI_MODE_UPDATE(USART,NUM,ME,IE,IFG)                                  \
do {                                                                          \
  if (MCU.sfr.ME.b.urxe##NUM == 1)   /* b.spie device ON */                   \
    {                                                                         \
      /* Tx buffer */							\
      if (MCU.USART.uxtxbuf_full == 1)                                        \
        {                                                                     \
	  if (MCU.USART.uxtx_shift_empty == 1 &&			      \
	      (MCU.USART.uxctl.b.mm ==1 || MCU.USART.uxrx_slave_rx_done == 1))\
	    /* if SPI in slave mode, tx is done immediately as delay has  */  \
	    /* been taken into account by the SPI master device on rx */      \
            {                                                                 \
               MCU.USART.uxtx_shift_buf   = MCU.USART.uxtxbuf;                \
               MCU.USART.uxtxbuf_full     = 0;                                \
               MCU.USART.uxtx_full_delay  = 0;                                \
               MCU.USART.uxtx_shift_empty = 0;                                \
               MCU.USART.uxtx_shift_ready = 0;                                \
	       if (MCU.USART.uxctl.b.mm == 1) /* master */		      \
		 {							      \
		   MCU.USART.uxtx_shift_delay = 7+MCU.USART.uxctl.b.charb;    \
		 }							      \
	       else							      \
		 {							      \
		   MCU.USART.uxtx_shift_delay = 0;			      \
		   MCU.USART.uxrx_slave_rx_done = 0;			      \
		 }							      \
               MCU.sfr.IFG.b.utxifg##NUM  = 1;                                \
               if (MCU.sfr.IE.b.utxie##NUM)                                   \
                 {                                                            \
                    msp430_interrupt_set(INTR_USART##NUM##_TX);               \
                 }                                                            \
	       else							      \
		 {							      \
		   DMA_SET_UTXIFG##NUM ();				      \
		 }							      \
               HW_DMSG_USART("msp430:usart%d: SPI tx buf -> shifter (%d)\n",  \
			     NUM,MCU.USART.uxtx_shift_delay);		      \
	       HW_SPY("msp430:usart%d: SPI send (0x%x,%c) at 0x%04x\n",NUM,   \
		       MCU.USART.uxtxbuf, isprint(MCU.USART.uxtxbuf) ?	      \
		       MCU.USART.uxtxbuf : '.', mcu_get_pc());		      \
            }                                                                 \
         } /* uxtfubuf_full */                                                \
                                                                              \
      /* TX shift register */                                                 \
      if (MCU.USART.uxtx_shift_empty == 0)                                    \
         {                                                                    \
            switch (MCU.USART.uxtctl.b.ssel)                                  \
              {                                                               \
            case 0:                                                           \
              ERROR("msp430:usart%d: SPI TX clk = UCLK, not supported\n",NUM);\
              MCU.USART.uxtx_shift_delay = 0;                                 \
              break;                                                          \
            case 1:                                                           \
              MCU.USART.uxtx_shift_delay -= MCU_CLOCK.ACLK_increment;         \
	      HW_DMSG_USART("msp430:usart%d, aclk SPI delay %d\n",NUM,        \
                              NUM,MCU.USART.uxtx_shift_delay);                \
              break;                                                          \
            case 2:                                                           \
            case 3:                                                           \
              MCU.USART.uxtx_shift_delay -= MCU_CLOCK.SMCLK_increment;        \
	      /*HW_DMSG_USART("msp430:usart%d, smclk SPI delay %d\n",*/	\
	      /*  NUM,MCU.USART.uxtx_shift_delay);	*/		\
              break;                                                          \
              }                                                               \
                                                                              \
            if (MCU.USART.uxtx_shift_delay <= 0)                              \
              {                                                               \
               /*tracer_event_record(TRACER_USART##NUM,TRACER_SPI_TX_READY);*/\
                 MCU.USART.uxtx_shift_ready = 1;                              \
                 MCU.USART.uxtctl.b.txept   = 1;                              \
              }                                                               \
         } /* tx_shift_empty */                                               \
                                                                              \
      /* RX shift register */                                                 \
      if (MCU.USART.uxrx_shift_empty == 0)                                    \
        {                                                                     \
          if (MCU.USART.uxctl.b.listen == 1)                                  \
            {                                                                 \
               ERROR("msp430:usart%d: SPI listen mode not supported\n",NUM);  \
            }                                                                 \
                                                                              \
          switch (MCU.USART.uxtctl.b.ssel)                                    \
            {                                                                 \
          case 0:                                                             \
            ERROR("msp430:usart%d: SPI RX clk = UCLK not supported\n",NUM);   \
            MCU.USART.uxrx_shift_delay = 0;                                   \
            break;                                                            \
          case 1:                                                             \
            MCU.USART.uxrx_shift_delay -= MCU_CLOCK.ACLK_increment;           \
            break;                                                            \
          case 2:                                                             \
          case 3:                                                             \
            MCU.USART.uxrx_shift_delay -= MCU_CLOCK.SMCLK_increment;          \
            break;                                                            \
            }                                                                 \
                                                                              \
          if (MCU.USART.uxrx_shift_delay <= 0)                                \
            {                                                                 \
               MCU.USART.uxrx_shift_ready = 1;                                \
            }                                                                 \
        } /* shift empty */                                                   \
                                                                              \
      if (MCU.USART.uxrx_shift_ready == 1)                                    \
        {                                                                     \
          if (MCU.USART.uxrxbuf_full == 1)                                    \
            {                                                                 \
               MCU.USART.uxrctl.b.oe = 1;                                     \
               /* sends out lots of debug message for burst writes */         \
               /* ERROR("msp430:usart%d: SPI Rx Overrun (0x%x,%c)\n",NUM, */  \
	       /*	     MCU.USART.uxrx_shift_buf,		*/	\
	       /*   isgraph(MCU.USART.uxrx_shift_buf) ?	*/		\
	       /*   MCU.USART.uxrx_shift_buf : '.');	*/		\
            }                                                                 \
          /*tracer_event_record(TRACER_USART##NUM, TRACER_SPI_RX_READY);*/    \
          MCU.USART.uxrxbuf          = MCU.USART.uxrx_shift_buf;              \
          MCU.USART.uxrxbuf_full     = 1;                                     \
          MCU.USART.uxrx_shift_ready = 0;                                     \
          MCU.USART.uxrx_shift_empty = 1;                                     \
          HW_DMSG_USART("msp430:usart%d: SPI rx shifter -> rx buf\n",NUM);    \
	  HW_SPY("msp430:usart%d: SPI receive (0x%x,%c)\n",NUM,	              \
                  MCU.USART.uxrxbuf, isgraph(MCU.USART.uxrxbuf) ?	      \
                  MCU.USART.uxrxbuf : '.');	                              \
          MCU.sfr.IFG.b.urxifg##NUM  = 1;                                     \
          if (MCU.sfr.IE.b.urxie##NUM)                                        \
            {                                                                 \
               msp430_interrupt_set(INTR_USART##NUM##_RX);                    \
            }                                                                 \
	  else								      \
	    {								      \
	      DMA_SET_URXIFG##NUM ();					      \
	    }								      \
        } /* shift ready */                                                   \
    } /* urxe. spie */                                                        \
} while (0)
/**** end SPI transmit    ****/                                     
/*****************************/                                     


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* UART mode                                                                 */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static inline int32_t USART_MODE_UPDATE_BITCLK(int ssel, int num)
{
  int32_t res = 0;
  switch (ssel)
    {
    case 0:
      ERROR("msp430:usart%d: UART clk = UCLK\n",num);
      res = 0;
      break;
    case 1:
      res = MCU_CLOCK.ACLK_increment;
      break;
    case 2:
    case 3:
      res = MCU_CLOCK.SMCLK_increment;
      break;
    }
  return res;
}

/*****************************/                                     
/**** UART transmit enable ****/                                     
#define USART_MODE_UPDATE(USART,NUM,ME,IE,IFG)                                \
do {                                                                          \
                                                                              \
  /* TX buffer */                                                             \
  if (MCU.sfr.ME.b.utxe##NUM == 1) /* tx enabled */                           \
    {                                                                         \
      if (MCU.USART.uxtxbuf_full == 1)                                        \
        {                                                                     \
          if ((MCU.USART.uxtx_full_delay <= 0) && (MCU.USART.uxtx_shift_empty == 1)) \
            {                                                                 \
               MCU.USART.uxtx_shift_buf   = MCU.USART.uxtxbuf;                \
               MCU.USART.uxtxbuf_full     = 0;                                \
               MCU.USART.uxtx_full_delay  = 0;                                \
               MCU.USART.uxtx_shift_empty = 0;                                \
               MCU.USART.uxtx_shift_ready = 0;                                \
               MCU.USART.uxtx_shift_delay = MCU.USART.uxbr_div;               \
               MCU.sfr.IFG.b.utxifg##NUM  = 1;                                \
               if (MCU.sfr.IE.b.utxie##NUM)                                   \
                 {                                                            \
                    msp430_interrupt_set(INTR_USART##NUM##_TX);               \
                 }                                                            \
	       else							      \
		 {							      \
		   DMA_SET_UTXIFG##NUM ();				      \
		 }							      \
               HW_DMSG_USART("msp430:usart%d: UART tx buf -> shifter (delay %d, val %d)\n", \
			     NUM,MCU.USART.uxtx_shift_delay, MCU.USART.uxtxbuf);            \
	       HW_SPY("msp430:usart%d: UART send (0x%x,%c)\n",NUM,            \
                  MCU.USART.uxtxbuf, isgraph(MCU.USART.uxtxbuf) ?             \
                  MCU.USART.uxtxbuf : '.');                                   \
            }                                                                 \
          else                                                                \
            {                                                                 \
              /* dec uxtx_full_delay */                                       \
              MCU.USART.uxtx_full_delay -=                                    \
                       USART_MODE_UPDATE_BITCLK(MCU.USART.uxtctl.b.ssel,NUM); \
            }                                                                 \
         } /* uxtfubuf_full */                                                \
    } /* tx enabled */                                                        \
                                                                              \
                                                                              \
  /* TX shift register */                                                     \
  /* transmit is flushed even if me.uxte is disabled */                       \
  if (MCU.USART.uxtx_shift_empty == 0)                                        \
    {                                                                         \
      MCU.USART.uxtx_shift_delay -=                                           \
            USART_MODE_UPDATE_BITCLK(MCU.USART.uxtctl.b.ssel,NUM);            \
      if (MCU.USART.uxtx_shift_delay <= 0)                                    \
	{                                                                     \
	  /*tracer_event_record(TRACER_USART##NUM,TRACER_UART_TX_READY);*/    \
	  MCU.USART.uxtx_shift_ready = 1;                                     \
	  MCU.USART.uxtctl.b.txept   = 1;                                     \
	}								      \
    } /* tx_shift_empty */						      \
                                                                              \
                                                                              \
  /* RX shift register */                                                     \
  /* finish receive even if me.uxrx is off */                                 \
  /* if (MCU.sfr.ME.b.urxe##NUM == 1) */                                      \
    {                                                                         \
      /* RX shift register */                                                 \
      if (MCU.USART.uxrx_shift_empty == 0)                                    \
        {                                                                     \
          if (MCU.USART.uxctl.b.listen == 1)                                  \
            {                                                                 \
               ERROR("msp430:usart%d: UART listen mode not supported\n",NUM); \
            }                                                                 \
								              \
	  MCU.USART.uxrx_shift_delay -=					      \
	    USART_MODE_UPDATE_BITCLK(MCU.USART.uxtctl.b.ssel,NUM);	      \
                                                                              \
          if (MCU.USART.uxrx_shift_delay <= 0)                                \
            {                                                                 \
               MCU.USART.uxrx_shift_ready = 1;                                \
            }                                                                 \
        } /* shift empty */                                                   \
                                                                              \
                                                                              \
      if (MCU.USART.uxrx_shift_ready == 1)                                    \
        {                                                                     \
          if (MCU.USART.uxrxbuf_full == 1)                                    \
            {                                                                 \
               MCU.USART.uxrctl.b.oe = 1;                                     \
               ERROR("msp430:usart%d: UART Rx Overrun (0x%x,%c)\n",NUM,       \
		     MCU.USART.uxrx_shift_buf,                                \
                     isgraph(MCU.USART.uxrx_shift_buf) ?                      \
                     MCU.USART.uxrx_shift_buf : '.');                         \
            }								      \
          TRACER_TRACE_USART##NUM##RX(TRACER_UART_RX_READY);                  \
          MCU.USART.uxrxbuf          = MCU.USART.uxrx_shift_buf;              \
          MCU.USART.uxrxbuf_full     = 1;                                     \
	  /*   moved to update_done   */				\
          /*MCU.USART.uxrx_shift_ready = 0;*/				\
	  /*MCU.USART.uxrx_shift_empty = 1;*/				\
          HW_DMSG_USART("msp430:usart%d: UART rx shifter -> rx buf\n",NUM);   \
	  HW_SPY("msp430:usart%d: UART receive (0x%x,%c)\n",NUM,              \
                  MCU.USART.uxrxbuf, isgraph(MCU.USART.uxrxbuf) ?             \
                  MCU.USART.uxrxbuf : '.');                                   \
          MCU.sfr.IFG.b.urxifg##NUM  = 1;                                     \
          if (MCU.sfr.IE.b.urxie##NUM)                                        \
            {                                                                 \
               msp430_interrupt_set(INTR_USART##NUM##_RX);                    \
            }                                                                 \
	  else								      \
	    {								      \
	      DMA_SET_URXIFG##NUM ();					      \
	    }								      \
        } /* shift ready */                                                   \
    } /* me.urxe */                                                           \
} while (0)
/**** end UART transmit    ****/                                     
/*****************************/                                     

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define USART_UPDATE(USART,NUM,ME,IE,IFG)                                     \
  switch (MCU.USART.mode)                                                     \
    {                                                                         \
  case USART_MODE_UART:                                                       \
    USART_MODE_UPDATE(USART,NUM,ME,IE,IFG);                                   \
    break;                                                                    \
  case USART_MODE_SPI:                                                        \
    SPI_MODE_UPDATE(USART,NUM,ME,IE,IFG);                                     \
    break;                                                                    \
  case USART_MODE_I2C:                                                        \
    ERROR("msp430:usart%d: I2C not implemented\n",NUM);                       \
    break;                                                                    \
    }


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define USART_READ(USART,NUM,IE,IFG)                                          \
  int8_t res;                                                                 \
  switch (addr)                                                               \
    {                                                                         \
    case U##NUM##CTL        :                                                 \
      res = MCU.USART.uxctl.s;                                                \
      HW_DMSG_USART("msp430:usart%d: read uxctl = 0x%02x\n",NUM,res & 0xff);  \
      break;                                                                  \
    case U##NUM##TCTL       :                                                 \
      res = MCU.USART.uxtctl.s;                                               \
      HW_DMSG_USART("msp430:usart%d: read uxtctl = 0x%02x [PC=0x%04x]\n",NUM,res & 0xff, mcu_get_pc()); \
      break;                                                                  \
    case U##NUM##RCTL       :                                                 \
      res = MCU.USART.uxrctl.s;                                               \
      HW_DMSG_USART("msp430:usart%d: read uxrctl = 0x%02x\n",NUM,res & 0xff); \
      break;                                                                  \
    case U##NUM##MCTL       :                                                 \
      res = MCU.USART.uxmctl;                                                 \
      HW_DMSG_USART("msp430:usart%d: read uxmctl = 0x%02x\n",NUM,res & 0xff); \
      break;                                                                  \
    case U##NUM##BR0        :                                                 \
      res = MCU.USART.uxbr0;                                                  \
      HW_DMSG_USART("msp430:usart%d: read uxbr0 = 0x%02x\n",NUM,res & 0xff);  \
      break;                                                                  \
    case U##NUM##BR1        :                                                 \
      res = MCU.USART.uxbr1;                                                  \
      HW_DMSG_USART("msp430:usart%d: read uxbr1 = 0x%02x\n",NUM,res & 0xff);  \
      break;                                                                  \
    case U##NUM##RXBUF      :                                                 \
      res = MCU.USART.uxrxbuf;                                                \
      MCU.sfr.IFG.b.urxifg##NUM = 0;                                          \
      MCU.USART.uxrxbuf_full    = 0;                                          \
      TRACER_TRACE_USART##NUM##RX(TRACER_UART_IDLE);                          \
      switch (MCU.USART.mode) {                                               \
      case USART_MODE_UART:                                                   \
        break;                                                                \
      case USART_MODE_SPI:                                                    \
        etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                      \
                           ETRACER_PER_EVT_READ_COMMAND,                      \
                          (ETRACER_PER_ARG_WR_DST_FIFO | ETRACER_ACCESS_LVL_BUS), 0);\
        break;                                                                \
      case USART_MODE_I2C:                                                    \
        break;                                                                \
      }                                                                       \
      MCU.USART.uxrctl.b.oe     = 0;                                          \
      HW_DMSG_USART("msp430:usart%d: read uxrxbuf = 0x%02x\n",NUM,res & 0xff);\
      break;                                                                  \
    case U##NUM##TXBUF      :                                                 \
      res = MCU.USART.uxtxbuf;                                                \
      MCU.sfr.IFG.b.utxifg##NUM = 0;                                          \
      HW_DMSG_USART("msp430:usart%d: read uxtxifg = 0x%02x\n",NUM,res & 0xff);\
      break;                                                                  \
    default                 :                                                 \
      res = 0;                                                                \
      ERROR("msp430:usart%d: read bad address 0x%04x\n",NUM,addr);            \
      break;                                                                  \
    }                                                                         \
  return res;                                                                 

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(HW_SPY_MODE)
#define HW_SPY_PRINT_CONFIG(USART,NUM)					      \
  HW_SPY("msp430:usart%d:spi: ============\n",NUM);                           \
  HW_SPY("msp430:usart%d:spi:ctl   %x\n",NUM,MCU.USART.uxctl.s);              \
  HW_SPY("msp430:usart%d:spi:tctl  %x\n",NUM,MCU.USART.uxtctl.s);             \
  HW_SPY("msp430:usart%d:spi:rctl  %x\n",NUM,MCU.USART.uxrctl.s);             \
  HW_SPY("msp430:usart%d:spi:br0   %x\n",NUM,MCU.USART.uxbr0);                \
  HW_SPY("msp430:usart%d:spi:br1   %x\n",NUM,MCU.USART.uxbr1);                \
  HW_SPY("msp430:usart%d:spi:mctl  %x\n",NUM,MCU.USART.uxmctl);               \
  HW_SPY("msp430:usart%d:spi:rxbuf %x\n",NUM,MCU.USART.uxrxbuf);              \
  HW_SPY("msp430:usart%d:spi:txbuf %x\n",NUM,MCU.USART.uxtxbuf);              \
  HW_SPY("msp430:usart%d:spi: ============\n",NUM);                            
#else                                                                         
#define HW_SPY_PRINT_CONFIG(USART,NUM) do {} while(0)
#endif

/*
 * Baud Rate Timing:  slau049f.pdf page 261 
 * 
 *
 */
#define msp430_usart_set_baudrate(NUM,USART)				      \
  {                     						      \
    USART.uxbr_div = ((USART.uxbr1 << 8 | USART.uxbr0)) *                     \
                     (USART.uxctl.b.charb + 7);				      \
    HW_DMSG_USART("msp430:usart%d:   baudrate : uxbr = %d, div = %d\n",	      \
		  NUM,(USART.uxbr1 << 8 | USART.uxbr0), USART.uxbr_div);      \
  }


#define USART_WRITE(USART,NUM,IE,IFG)                                         \
  switch (addr)                                                               \
    {                                                                         \
    case U##NUM##CTL        :                                                 \
      {                                                                       \
	union {                                                               \
	  struct uxctl_t      b;                                              \
	  struct uxctl_spi_t  b_spi;                                          \
	  uint8_t             s;                                              \
	} ctl;                                                                \
	                                                                      \
	ctl.s = val;                                                          \
	HW_DMSG_USART("msp430:usart%d: write uxctl = 0x%02x\n",NUM,val & 0xff);                    \
	/* HW_DMSG_USART("msp430:usart%d:   swrst  = %d\n",NUM, ctl.b.swrst); */ \
	/* HW_DMSG_USART("msp430:usart%d:   sync   = %d\n",NUM, ctl.b.sync);  */ \
	/* HW_DMSG_USART("msp430:usart%d:   mm     = %d (multi master)\n",NUM, ctl.b.mm); */  \
	/* HW_DMSG_USART("msp430:usart%d:   listen = %d (loopback)\n",NUM, ctl.b.listen); */  \
	/* HW_DMSG_USART("msp430:usart%d:   spb    = %d (0:1 stop, 1:2 stop bits)\n",NUM, ctl.b.spb); */  \
	/* ctl.b.sync == 0                         : UART */                  \
	/* ctl.b.sync == 1, ctl.b.spb == 0         : SPI  */                  \
	/* ctl.b.sync == 1, ctl.b.spb == 1         : I2C  */                  \
        if (ctl.b.sync == 0)                                                  \
          {                                                                   \
            MCU.USART.mode = USART_MODE_UART;                                 \
            if (MCU.USART.uxctl.b.sync == 1)                                  \
              {                                                               \
                HW_SPY("msp430:usart%d:   switch to USART mode\n",NUM);       \
                HW_DMSG_USART("msp430:usart%d:   mode   = USART\n",NUM);      \
              }                                                               \
          }                                                                   \
        else /* ctl.b.sync == 1 */                                            \
          {                                                                   \
            if (ctl.b.spb == 0)                                               \
              {                                                               \
                MCU.USART.mode = USART_MODE_SPI;                              \
                if (MCU.USART.uxctl.b.sync == 0)                              \
                  {                                                           \
                     HW_SPY("msp430:usart%d:   switch to SPI mode\n",NUM);    \
                     HW_SPY_PRINT_CONFIG(USART,NUM);                          \
                     HW_DMSG_USART("msp430:usart%d:   mode   = SPI\n",NUM);   \
                  }                                                           \
              }                                                               \
            else /* ctl.b.spb == 1 */                                         \
              {                                                               \
		/* #if defined(__msp430_have_usart0_with_i2c) */              \
                if (NUM == 0) /* only uart0 can have i2c */                   \
		  {                                                           \
                     MCU.USART.mode = USART_MODE_I2C;                         \
                     ERROR("msp430:usart%d:   i2c mode is not implemented\n",NUM);                 \
                     HW_DMSG_USART("msp430:usart%d:   mode   = I2C (not supported)\n",NUM);        \
                  }                                                           \
                else                                                          \
                 {                                                            \
		     ERROR("msp430:usart%d:   i2c does not exist on USART%d\n",NUM,NUM);           \
		 }							      \
              }                                                               \
          }                                                                   \
                                                                              \
	HW_DMSG_USART("msp430:usart%d:   length = %d bits\n",NUM,             \
                                           (ctl.b.charb == 1) ? 8:7);         \
                                                                              \
        if ((ctl.b.swrst == 1) && (MCU.USART.uxctl.b.swrst == 0))             \
          {                                                                   \
             /* reset  URXIEx, UTXIEx, URXIFGx, OE, and FE bits */            \
             /* set UTXIFGx flag                                */            \
             HW_DMSG_USART("msp430:usart%d:   swrst  = 1, reset flags\n",     \
                            NUM);                                             \
             MCU.sfr.IE.b.urxie##NUM     = 0;                                 \
             MCU.sfr.IE.b.utxie##NUM     = 0;                                 \
             MCU.sfr.IFG.b.urxifg##NUM   = 0;                                 \
             MCU.USART.uxrctl.b.oe       = 0;                                 \
             MCU.USART.uxrctl.b.fe       = 0;                                 \
             MCU.sfr.IFG.b.utxifg##NUM   = 1;                                 \
             /* finishing current transaction */                              \
             MCU.USART.uxtxbuf_full      = 0;                                 \
             MCU.USART.uxtx_full_delay   = 0;                                 \
             MCU.USART.uxtctl.b.txept    = 1;                                 \
             MCU.USART.uxtx_shift_empty  = 1;                                 \
             MCU.USART.uxrxbuf_full      = 0;                                 \
             MCU.USART.uxrx_shift_empty  = 1;                                 \
          }                                                                   \
	MCU.USART.uxctl.s = val;                                              \
        if (ctl.b.charb != MCU.USART.uxctl.b.charb) {                         \
           msp430_usart_set_baudrate(NUM, MCU.USART);                         \
        }                                                                     \
      }                                                                       \
      break;                                                                  \
                                                                              \
    case U##NUM##TCTL       :  /* transmit ctrl register */                   \
      {                                                                       \
	union {                                                               \
	  struct uxtctl_t     b;                                              \
	  struct uxtctl_spi_t b_spi;                                          \
	  uint8_t             s;                                              \
	} tctl;                                                               \
        tctl.s = val;                                                         \
        HW_DMSG_USART("msp430:usart%d: write uxtctl = 0x%02x\n",NUM,val & 0xff);                   \
        HW_DMSG_USART("msp430:usart%d:   spi:ckph    = %d (clock phase)\n",NUM,tctl.b.ckph);       \
        HW_DMSG_USART("msp430:usart%d:   ckpl        = %d (clock polarity)\n",NUM,tctl.b.ckpl);    \
        HW_DMSG_USART("msp430:usart%d:   ssel        = %d (%s)\n",            \
                      NUM,tctl.b.ssel,str_ssel[tctl.b.ssel]);                 \
        HW_DMSG_USART("msp430:usart%d:   uart:urxse  = %d (recv start edge)\n",NUM,tctl.b.urxse);  \
        HW_DMSG_USART("msp430:usart%d:   uart:txwake = %d (0:data 1:addr)\n",NUM,tctl.b.txwake);   \
        if (tctl.b.txwake == 1) {                                             \
           MCU.USART.next_tx_type = usart_data;                               \
        } else {                                                              \
           MCU.USART.next_tx_type = usart_address;                            \
        }                                                                     \
        HW_DMSG_USART("msp430:usart%d:   spi:stc     = %d (4pin spi, 3pin spi)\n",NUM,tctl.b.stc); \
        HW_DMSG_USART("msp430:usart%d:   txept       = %d (tx empty flag)\n",NUM,tctl.b.txept);    \
	MCU.USART.uxtctl.s = val;                                             \
        if (tctl.b.ssel != MCU.USART.uxtctl.b.ssel)                           \
          {                                                                   \
             msp430_usart_set_baudrate(NUM, MCU.USART);                       \
          }                                                                   \
      }                                                                       \
      break;                                                                  \
    case U##NUM##RCTL       : /* receive ctrl register */                     \
      HW_DMSG_USART("msp430:usart%d: write uxrctl = 0x%02x\n",NUM,val & 0xff);\
      MCU.USART.uxrctl.s = val;                                               \
      break;                                                                  \
    case U##NUM##MCTL       : /* modulation ctrl register */                  \
      HW_DMSG_USART("msp430:usart%d: write uxmctl = 0x%02x\n",NUM,val & 0xff);\
      MCU.USART.uxmctl = val;                                                 \
      msp430_usart_set_baudrate(NUM, MCU.USART);                              \
      break;                                                                  \
    case U##NUM##BR0        : /* baud rate ctrl register 0 */                 \
      HW_DMSG_USART("msp430:usart%d: write uxbr0  = 0x%02x\n",NUM,val & 0xff);\
      MCU.USART.uxbr0 = val;                                                  \
      msp430_usart_set_baudrate(NUM, MCU.USART);                              \
      break;                                                                  \
    case U##NUM##BR1        : /* baud rate ctrl register 1 */                 \
      HW_DMSG_USART("msp430:usart%d: write uxbr1  = 0x%02x\n",NUM,val & 0xff);\
      MCU.USART.uxbr1 = val;                                                  \
      msp430_usart_set_baudrate(NUM, MCU.USART);                              \
      break;                                                                  \
    case U##NUM##RXBUF      :                                                 \
      ERROR("msp430:usart%d: writing to read only register RXBUFF\n",NUM);    \
      break;                                                                  \
    case U##NUM##TXBUF      :                                                 \
      HW_DMSG_USART("msp430:usart%d: write uxtxbuf  = 0x%02x [PC=0x%04x]\n",NUM,val & 0xff, \
		    mcu_get_pc());					\
      if ((MCU.USART.uxtxbuf_full == 1) && (MCU.USART.uxctl.b.mm == 1)) /* master */ \
        {                                                                     \
          WARNING("msp430:usart%d:    overwriting tx buffer with [0x%02x]\n", \
                NUM,val & 0xff);                                              \
        }                                                                     \
      MCU.USART.uxtxbuf            = val;                                     \
      MCU.USART.uxtxbuf_full       = 1;                                       \
      MCU.USART.uxtx_full_delay    = 1; /* go to shifter after xx BITCLK */   \
      MCU.USART.uxtctl.b.txept     = 0;                                       \
      MCU.sfr.IFG.b.utxifg##NUM    = 0;                                       \
      switch (MCU.USART.mode) {                                               \
      case USART_MODE_UART:                                                   \
        TRACER_TRACE_USART##NUM##TX(TRACER_UART_TX_RECV);		      \
        /* can be a dupe from the platform file */                            \
        etracer_slot_event(ETRACER_PER_ID_MCU_USART + NUM,                    \
                           ETRACER_PER_EVT_WRITE_COMMAND,                     \
                           ETRACER_PER_ARG_WR_DST_FIFO, 0);                   \
        break;                                                                \
      case USART_MODE_SPI:                                                    \
        TRACER_TRACE_USART##NUM##TX(TRACER_SPI_TX_RECV);                      \
        etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                      \
                           ETRACER_PER_EVT_WRITE_COMMAND,                     \
                           ETRACER_PER_ARG_WR_DST_FIFO | ETRACER_ACCESS_LVL_BUS, 0); \
        HW_SPY("msp430:usart%d:spi: write byte %x\n",NUM,val & 0xff);         \
        HW_SPY_PRINT_CONFIG(USART,NUM);                                       \
        break;                                                                \
      case USART_MODE_I2C:                                                    \
        TRACER_TRACE_USART##NUM##TX(TRACER_I2C_TX_RECV);		      \
        break;                                                                \
      }                                                                       \
      break;                                                                  \
    }  /* switch */                                                           

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define USART_CHKIFG(NUM,IE,IFG)                                              \
   int ret = 0;                                                               \
   if (MCU.sfr.IFG.b.utxifg##NUM && MCU.sfr.IE.b.utxie##NUM)                  \
     {                                                                        \
        msp430_interrupt_set(INTR_USART##NUM##_TX);                           \
        ret = 1;                                                              \
     }                                                                        \
   if (MCU.sfr.IFG.b.urxifg##NUM && MCU.sfr.IE.b.urxie##NUM)                  \
     {                                                                        \
        msp430_interrupt_set(INTR_USART##NUM##_RX);                           \
        ret = 1;                                                              \
     }                                                                        \
   return ret;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* read from external peripherals */

#define SPI_READ(USART,NUM,ME,val)                                            \
  if (MCU.USART.mode == USART_MODE_SPI)                                       \
    {                                                                         \
      if (MCU.USART.uxtx_shift_ready == 1)                                    \
	{                                                                     \
	  val = MCU.USART.uxtx_shift_buf;                                     \
	  MCU.USART.uxtx_shift_ready = 0;                                     \
	  MCU.USART.uxtx_shift_empty = 1;                                     \
          TRACER_TRACE_USART##NUM##TX(TRACER_UART_IDLE);		      \
          etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                    \
                             ETRACER_PER_EVT_WRITE_COMMAND,                   \
                            (ETRACER_PER_ARG_WR_SRC_FIFO | (ETRACER_ACCESS_LVL_SPI + NUM)), 0); \
	  return 1;                                                           \
	}                                                                     \
    }                                                                         \
  return 0;


#define SPI_WRITE_OK(USART)                                                   \
  (MCU.USART.mode == USART_MODE_SPI && MCU.USART.uxrx_shift_empty == 1)      

/* usart SPI write from peripherals */
#if 0
// removed from RX, a byte is received while TX
      if (MCU.USART.uxrx_shift_empty != 1)                                    \
	{                                                                     \
	  ERROR("msp430:usart%d: SPI rx value while rx shift not empty (%d)\n", \
	          NUM,MCU.USART.uxrx_shift_delay);                            \
	}                                                                     \
      else                                                                    \

#endif

#define SPI_WRITE(USART,NUM,ME,val)                                           \
  if (MCU.sfr.ME.b.urxe##NUM == 1)  /* verif Digi IO _SEL & _DIR */           \
    {                                                                         \
        {                                                                     \
	  TRACER_TRACE_USART##NUM##RX(TRACER_SPI_RX_RECV);		      \
          MCU.USART.uxrx_shift_buf   = val;                                   \
          MCU.USART.uxrx_shift_empty = 0;                                     \
          MCU.USART.uxrx_shift_ready = 0;				      \
	  /* no delay, as delay has been taken into account during tx*/	      \
          MCU.USART.uxrx_shift_delay = 0;				      \
	  if (MCU.USART.uxctl.b.mm == 0)				      \
	    MCU.USART.uxrx_slave_rx_done = 1;  				      \
          etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                    \
                             ETRACER_PER_EVT_WRITE_COMMAND,                   \
                            (ETRACER_PER_ARG_WR_DST_FIFO | (ETRACER_ACCESS_LVL_SPI + NUM)), 0); \
          HW_DMSG_USART("msp430:usart%d: SPI rx value 0x%02x\n",NUM,val);  \
        }                                                                     \
    }                                                                         \
  else                                                                        \
    {                                                                         \
      if (MCU.USART.uxrx_shift_empty != 1)                                    \
        {                                                                     \
          HW_DMSG_USART("msp430:usart%d: SPI receive disable while shift not empty\n",NUM); \
        }                                                                     \
      ERROR("msp430:usart%d: SPI data RX but not in receive enable state\n",NUM);    \
    } 





#define UART_READ(USART,NUM,ME,val)                                           \
  if (MCU.USART.mode == USART_MODE_UART)                                      \
    {                                                                         \
      if (MCU.USART.uxtx_shift_ready == 1)                                    \
	{                                                                     \
	  val = MCU.USART.uxtx_shift_buf;                                     \
	  MCU.USART.uxtx_shift_ready = 0;                                     \
	  MCU.USART.uxtx_shift_empty = 1;                                     \
          TRACER_TRACE_USART##NUM##TX(TRACER_UART_IDLE);		      \
	  return 1;                                                           \
	}                                                                     \
    }                                                                         \
  return 0;


#define UART_WRITE_OK(USART)                                                  \
  (MCU.USART.mode == USART_MODE_UART && MCU.USART.uxrx_shift_empty == 1)      


#define UART_WRITE(USART,NUM,ME,val)                                          \
  if (MCU.sfr.ME.b.urxe##NUM == 1)  /* verif Digi IO _SEL & _DIR */           \
    {                                                                         \
      if (MCU.USART.uxrx_shift_empty != 1)                                    \
	{                                                                     \
	  ERROR("msp430:usart%d: UART rx value while rx shift not empty (%d)\n",\
	          NUM,MCU.USART.uxrx_shift_delay);                            \
	}                                                                     \
      else                                                                    \
        {                                                                     \
	  TRACER_TRACE_USART##NUM##RX(TRACER_UART_RX_RECV);                   \
          MCU.USART.uxrx_shift_buf   = val;                                   \
          MCU.USART.uxrx_shift_empty = 0;                                     \
          MCU.USART.uxrx_shift_ready = 0;                                     \
          MCU.USART.uxrx_shift_delay = MCU.USART.uxbr_div;                    \
          HW_DMSG_USART("msp430:usart%d: UART rx shifter set to 0x%02x (delay %d)\n",\
	                NUM,val,MCU.USART.uxrx_shift_delay);		      \
        }                                                                     \
    }                                                                         \
  else                                                                        \
    {                                                                         \
      if (MCU.USART.uxrx_shift_empty != 1)                                    \
        {                                                                     \
          HW_DMSG_USART("msp430:usart%d: UART receive disable while shift not empty\n",NUM);\
        }                                                                     \
      ERROR("msp430:usart%d: UART data RX but not in receive enable state\n",NUM);   \
    } 
  
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_usart0)

void msp430_usart0_create()
{
  msp430_io_register_range8(USART0_START,USART0_END,msp430_usart0_read,msp430_usart0_write);
}

void msp430_usart0_reset()
{
  MCU.usart0.uxctl.s  = 1;    // swrst
  MCU.usart0.uxtctl.s = 1;    // txept
  MCU.usart0.uxrctl.s = 0;

  MCU.sfr.me1.s       = 0;   
  MCU.sfr.ie1.s       = 0;
  MCU.sfr.ifg1.s     |= 0x82; // utxie0 & ofie

  USART_RESET(usart0);
}

/***************************/
/********* MCU API *********/

/* update usart within internal device loop */
void msp430_usart0_update()
{
  USART_UPDATE(usart0,0,me1,ie1,ifg1)
}

void msp430_usart0_update_done()
{
  if (MCU.usart0.uxrx_shift_ready == 1)
    {
      /*   moved from update  */
      MCU.usart0.uxrx_shift_ready = 0;
      MCU.usart0.uxrx_shift_empty = 1;
    }
}

/* usart read from MCU */
int8_t msp430_usart0_read(uint16_t addr)
{
  USART_READ(usart0,0,ie1,ifg1)
}

/* usart write from MCU */
void msp430_usart0_write(uint16_t addr, int8_t val)
{
  USART_WRITE(usart0,0,ie1,ifg1)
}

/* usart chk ifg for MCU interrupt */
int msp430_usart0_chkifg()
{
  USART_CHKIFG(0,ie1,ifg1)
}

/*******************************************/
/********* External Peripheral API *********/

/* usart SPI read from peripherals */
int msp430_usart0_dev_read_spi(uint8_t *val)
{
  SPI_READ(usart0,0,me1,*val)
}

/* usart SPI write from peripherals */
void msp430_usart0_dev_write_spi(uint8_t val)
{
  SPI_WRITE(usart0,0,me1,val)
}

int msp430_usart0_dev_write_spi_ok()
{
  return SPI_WRITE_OK(usart0);
}

/* usart UART read from peripherals */
int msp430_usart0_dev_read_uart(uint8_t *val)
{
  UART_READ(usart0,0,me1,*val)
}

/* usart UART write from peripherals */
void msp430_usart0_dev_write_uart(uint8_t val)
{
  UART_WRITE(usart0,0,me1,val)
}

int msp430_usart0_dev_write_uart_ok()
{
  return UART_WRITE_OK(usart0);
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_usart1)
void msp430_usart1_reset()
{
  MCU.usart1.uxctl.s  = 1;     // swrst
  MCU.usart1.uxtctl.s = 1;     // txept
  MCU.usart1.uxrctl.s = 0;

  MCU.sfr.me2.s       = 0;   
  MCU.sfr.ie2.s       = 0;
  MCU.sfr.ifg2.s     |= 0x020; // utxifg1

  USART_RESET(usart1);
}

void msp430_usart1_create()
{
  msp430_io_register_range8(USART1_START,USART1_END,msp430_usart1_read,msp430_usart1_write);
}

/***************************/
/********* MCU API *********/

/* update usart within internal device loop */
void msp430_usart1_update()
{
  USART_UPDATE(usart1,1,me2,ie2,ifg2)
}

/* usart read from MCU */
int8_t msp430_usart1_read(uint16_t addr)
{
  USART_READ(usart1,1,ie2,ifg2)
}

/* usart write from MCU */
void msp430_usart1_write(uint16_t addr, int8_t val)
{
  USART_WRITE(usart1,1,ie2,ifg2)
}

/* usart chk ifg for MCU interrupt */
int msp430_usart1_chkifg()
{
  USART_CHKIFG(1,ie2,ifg2)
}

/*******************************************/
/********* External Peripheral API *********/

/* usart SPI read from peripherals */
int msp430_usart1_dev_read_spi(uint8_t *val)
{
  SPI_READ(usart1,1,me2,*val)
}

/* usart SPI write from peripherals */
void msp430_usart1_dev_write_spi(uint8_t val)
{
  SPI_WRITE(usart1,1,me2,val)
}

int msp430_usart1_dev_write_spi_ok()
{
  return SPI_WRITE_OK(usart1);
}

/* usart UART read from peripherals */
int msp430_usart1_dev_read_uart(uint8_t *val)
{
  UART_READ(usart1,1,me2,*val)
}

/* usart UART write from peripherals */
void msp430_usart1_dev_write_uart(uint8_t val)
{
  UART_WRITE(usart1,1,me2,val)
}

int msp430_usart1_dev_write_uart_ok()
{
  return UART_WRITE_OK(usart1);
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

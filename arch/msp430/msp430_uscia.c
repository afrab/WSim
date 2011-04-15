
/**
 *  \file   msp430_uscia.c
 *  \brief  MSP430 USCIA definition (based on "msp430_usart.c" UART MODE)
 *  \author Julien Carpentier
 *  \date   2011
 **/

#include <stdio.h> 
#include <string.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/*****************************************************/
/*****************************************************/
/*****************************************************/

#define HW_SPY_MODE 1

#if HW_SPY_MODE != 0
#define HW_SPY(x...) HW_DMSG_MCUDEV(x)
#else
#define HW_SPY(x...) do { } while (0)
#endif

/* defined in msp430_debug.h */
// #if defined(DEBUG_USCIA)  
// char *str_ssel[] = { "external UCLK", "ACLK", "SMCLK", "SMCLK" };
// #endif

/*****************************************************/
/*****************************************************/
/*****************************************************/

// #define TRACER_UART_IDLE     0 
// 
// #define TRACER_UART_TX_RECV  1  /* data written to TXBUF      */
// #define TRACER_UART_TX_READY 2  /* data TX shifter ok         */
// #define TRACER_UART_RX_RECV  3  /* data written to Rx shifter */
// #define TRACER_UART_RX_READY 4  /* data written to Rx buff    */
// 

/*****************************************************/
/*****************************************************/
/*****************************************************/

//slau144e.pdf (page 461)
/**
In asynchronous mode, the USCI_Ax modules connect the MSP430 to an
external system via two external pins, UCAxRXD and UCAxTXD. UART mode
is selected when the UCSYNC bit is cleared.
UART mode features include:
- 7- or 8-bit data with odd, even, or non-parity
- Independent transmit and receive shift registers
- Separate transmit and receive buffer registers
- LSB-first or MSB-first data transmit and receive
- Built-in idle-line and address-bit communication protocols for multiprocessor systems   
- Receiver start-edge detection for auto-wake up from LPMx modes
- Programmable baud rate with modulation for fractional baud rate support
- Status flags for error detection and suppression
- Status flags for address detection
- Independent interrupt capability for receive and transmit
**/


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#if defined(HW_SPY_MODE) 
#define HW_SPY_PRINT_CONFIG()					         \
  HW_SPY("msp430:uscia0: ============\n");                               \
  HW_SPY("msp430:uscia0:ctl0    %x\n",   MCU.uscia0.ucaxctl0.s);         \
  HW_SPY("msp430:uscia0:ctl1    %x\n",   MCU.uscia0.ucaxctl1.s);         \
  HW_SPY("msp430:uscia0:br0     %x\n",   MCU.uscia0.ucaxbr0);            \
  HW_SPY("msp430:uscia0:br1     %x\n",   MCU.uscia0.ucaxbr1);            \
  HW_SPY("msp430:uscia0:mctl    %x\n",   MCU.uscia0.ucaxmctl.s);         \
  HW_SPY("msp430:uscia0:stat    %x\n",   MCU.uscia0.ucaxstat.s);         \
  HW_SPY("msp430:uscia0:rxbuf   %x\n",   MCU.uscia0.ucaxrxbuf);          \
  HW_SPY("msp430:uscia0:txbuf   %x\n",   MCU.uscia0.ucaxtxbuf);          \
  HW_SPY("msp430:uscia0:abctl   %x\n",   MCU.uscia0.ucaxctl1.s);         \
  HW_SPY("msp430:uscia0:irtctl  %x\n",   MCU.uscia0.ucaxctl1.s);         \
  HW_SPY("msp430:uscia0:irrctl  %x\n",   MCU.uscia0.ucaxctl1.s);         \
  HW_SPY("msp430:uscia0: ============\n");                            
#else                                                                         
#define HW_SPY_PRINT_CONFIG() do {} while(0)
#endif

                       

#if defined(__msp430_have_uscia0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_uscia0_create()
{
  msp430_io_register_addr8 (UCA0ABCTL,msp430_uscia0_read,msp430_uscia0_write);
  msp430_io_register_addr8 (UCA0IRTCTL,msp430_uscia0_read,msp430_uscia0_write);
  msp430_io_register_addr8 (UCA0IRRCTL,msp430_uscia0_read,msp430_uscia0_write);
  msp430_io_register_range8(USCIA0_START,USCIA0_END,msp430_uscia0_read,msp430_uscia0_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline int32_t USCIA_MODE_UPDATE_BITCLK(int ucssel)
{
  int32_t res = 0;
  switch (ucssel)
    {
    case 0:
      ERROR("msp430:uscia0: USCIA clk = UCLK\n");
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

void msp430_uscia0_set_baudrate()                                  
{   
  MCU.uscia0.ucaxbr_div = ((MCU.uscia0.ucaxbr1 << 8 | MCU.uscia0.ucaxbr0) / 2) * (7+!(MCU.uscia0.ucaxctl0.b.uc7bit)); 
  HW_DMSG_USCIA("msp430:uscia0:   baudrate = div N %d/bit - %d/byte\n",      
		  ((MCU.uscia0.ucaxbr1 << 8 | MCU.uscia0.ucaxbr0) / 2),MCU.uscia0.ucaxbr_div);  
}

void msp430_uscia0_reset()
{
  MCU.uscia0.ucaxrxbuf             = 0;
  MCU.uscia0.ucaxrxbuf_full        = 0;
  MCU.uscia0.ucaxrx_shift_buf      = 0;
  MCU.uscia0.ucaxrx_shift_empty    = 1;
  MCU.uscia0.ucaxrx_shift_ready    = 0;                                           
  MCU.uscia0.ucaxrx_shift_delay    = 0;                                           
  MCU.uscia0.ucaxrx_slave_rx_done  = 0;  
                                                                               
  MCU.uscia0.ucaxtxbuf             = 0;                                           
  MCU.uscia0.ucaxtxbuf_full        = 0;                                           
  MCU.uscia0.ucaxtx_full_delay     = 0;                                                                           
  MCU.uscia0.ucaxtx_shift_buf      = 0;                                           
  MCU.uscia0.ucaxtx_shift_empty    = 1;                                           
  MCU.uscia0.ucaxtx_shift_ready    = 0;                                           
  MCU.uscia0.ucaxtx_shift_delay    = 0; 
}

/***************************/
/********* MCU API *********/

/* update uscia0 within internal device loop */
void msp430_uscia0_update()
{
   do {                                                                          
                                                                              
    /* TX buffer */                                                                               
     if (MCU.uscia0.ucaxtxbuf_full == 1)                                        
	  {                                                                     
	    if ((MCU.uscia0.ucaxtx_full_delay <= 0) && (MCU.uscia0.ucaxtx_shift_empty == 1)) 
	      {                                                                 
		MCU.uscia0.ucaxtx_shift_buf   = MCU.uscia0.ucaxtxbuf;                
		MCU.uscia0.ucaxtxbuf_full     = 0;                                
		MCU.uscia0.ucaxtx_full_delay  = 0;                                
		MCU.uscia0.ucaxtx_shift_empty = 0;                                
		MCU.uscia0.ucaxtx_shift_ready = 0;                                
		MCU.uscia0.ucaxtx_shift_delay = MCU.uscia0.ucaxbr_div;               
		MCU.sfr.ifg2.b.uca0txifg = 1;                                
		if (MCU.sfr.ie2.b.uca0txie)                                   
		  {                                                            
		      msp430_interrupt_set(INTR_USCIX0_TX);               
		  }                                                            
		else      
		  {      
		    DMA_SET_UTXIFG0();
		  }		      
		HW_DMSG_USCIA("msp430:uscia0: USCIA tx buf -> shifter (delay %d, val 0x%02x)\n", 
			      MCU.uscia0.ucaxtx_shift_delay, MCU.uscia0.ucaxtxbuf);            
		HW_SPY("msp430:uscia0: USCIA send (0x%x,%c)\n",            
		    MCU.uscia0.ucaxtxbuf, isgraph(MCU.uscia0.ucaxtxbuf) ?             
		    MCU.uscia0.ucaxtxbuf : '.');                                   
	      }                                                                 
	    else                                                                
	      {                                                                 
		/* dec ucaxtx_full_delay */                                       
		MCU.uscia0.ucaxtx_full_delay -=                                    
			USCIA_MODE_UPDATE_BITCLK(MCU.uscia0.ucaxctl1.b.ucssel);
	      }                                                                 
	} /* ucaxtfubuf_full */                                                                                                     


    /* TX shift register */                                                                      
    if (MCU.uscia0.ucaxtx_shift_empty == 0)                                        
      {                                                                         
	MCU.uscia0.ucaxtx_shift_delay -=                                           
	      USCIA_MODE_UPDATE_BITCLK(MCU.uscia0.ucaxctl1.b.ucssel);            
	if (MCU.uscia0.ucaxtx_shift_delay <= 0)                                    
	  {                                                                     
	    /*tracer_event_record(TRACER_USCIA0,TRACER_USCIA_TX_READY);*/    
	    MCU.uscia0.ucaxtx_shift_ready = 1;                                     
	  }	      
      } /* tx_shift_empty */	      


    /* RX shift register */                                                 
     if (MCU.uscia0.ucaxrx_shift_empty == 0)                                    
	{                                                                     
	    if (MCU.uscia0.ucaxstat.b.uclisten == 1)                                  
	      {                                                                 
		  ERROR("msp430:uscia0: USCIA listen mode not supported\n"); 
	      }                                                                              
	    MCU.uscia0.ucaxrx_shift_delay -=      
	      USCIA_MODE_UPDATE_BITCLK(MCU.uscia0.ucaxctl1.b.ucssel);	       
										
	    if (MCU.uscia0.ucaxrx_shift_delay <= 0)                                 
	      {                                                                  
		  MCU.uscia0.ucaxrx_shift_ready = 1;                                 
	      }                                                                  
	} /* shift empty */                                                    


      if (MCU.uscia0.ucaxrx_shift_ready == 1)                                     
	{                                                                      
	    if (MCU.uscia0.ucaxrxbuf_full == 1)                                     
	      {                                                                  
		  MCU.uscia0.ucaxstat.b.ucoe = 1;                                      
		  ERROR("msp430:uscia0: USCIA Rx Overrun (0x%x,%c)\n",       
		      MCU.uscia0.ucaxrx_shift_buf,                                 
		      isgraph(MCU.uscia0.ucaxrx_shift_buf) ?                       
		      MCU.uscia0.ucaxrx_shift_buf : '.');                          
	      }                                                                  
	    /*tracer_event_record(TRACER_USCIA0,TRACER_USCIA_RX_READY);*/     
	    MCU.uscia0.ucaxrxbuf          = MCU.uscia0.ucaxrx_shift_buf;               
	    MCU.uscia0.ucaxrxbuf_full     = 1;                                      
	    MCU.uscia0.ucaxrx_shift_ready = 0;                                      
	    MCU.uscia0.ucaxrx_shift_empty = 1;                                      
	    HW_DMSG_USCIA("msp430:uscia0: USCIA rx shifter -> rx buf\n");    
	    HW_SPY("msp430:uscia0: USCIA receive (0x%x,%c)\n",               
		    MCU.uscia0.ucaxrxbuf, isgraph(MCU.uscia0.ucaxrxbuf) ?              
		    MCU.uscia0.ucaxrxbuf : '.');                                    
	    MCU.sfr.ifg2.b.uca0rxifg  = 1; 
	    
	    if (MCU.sfr.ie2.b.uca0rxie)                                         
	      {                                                                  
		msp430_interrupt_set(INTR_USCIX0_RX);                     
	      }                                                                  
	    else       
	      {      
		DMA_SET_URXIFG0();
	      }      
	 } /* shift ready */  
	  
    } while (0);
}

/* uscia0 read from MCU */
int8_t msp430_uscia0_read(uint16_t addr)
{
  int8_t res;                                                                 
  switch (addr)                                                               
    {                                                                         
    case UCA0CTL0        :                                                 
      res = MCU.uscia0.ucaxctl0.s;                                                
      HW_DMSG_USCIA("msp430:uscia0: read ucaxctl0 = 0x%02x\n", res & 0xff);  
      break;      
    case UCA0CTL1        :                                                 
      res = MCU.uscia0.ucaxctl1.s;                                                
      HW_DMSG_USCIA("msp430:uscia0: read ucaxctl1 = 0x%02x\n", res & 0xff);  
      break;
    case UCA0BR0        :                                                 
      res = MCU.uscia0.ucaxbr0;                                                  
      HW_DMSG_USCIA("msp430:uscia0: read ucaxbr0 = 0x%02x\n", res & 0xff);  
      break;                                                                 
    case UCA0BR1        :                                                 
      res = MCU.uscia0.ucaxbr1;                                                 
      HW_DMSG_USCIA("msp430:uscia0: read ucaxbr1 = 0x%02x\n", res & 0xff);  
      break;    
    case UCA0MCTL       :                                                 
      res = MCU.uscia0.ucaxmctl.s;                                                 
      HW_DMSG_USCIA("msp430:uscia0: read ucaxmctl = 0x%02x\n", res & 0xff); 
      break;  
    case UCA0STAT       :                                               
      res = MCU.uscia0.ucaxstat.s;                                                 
      HW_DMSG_USCIA("msp430:uscia0: read ucaxstat = 0x%02x\n", res & 0xff); 
      break;  
    case UCA0RXBUF      :                                                 
      res = MCU.uscia0.ucaxrxbuf;                                                
      MCU.sfr.ifg2.b.uca0rxifg   = 0;                                          
      MCU.uscia0.ucaxrxbuf_full  = 0;                                          
      /*TRACER_TRACE_USCIA(TRACER_USCIA_IDLE);*/                                                                                                 \
      MCU.uscia0.ucaxstat.b.ucoe = 0;                                          
      HW_DMSG_USCIA("msp430:uscia0: read ucaxrxbuf = 0x%02x\n", res & 0xff);
      break;                                                                  
    case UCA0TXBUF      :                                                 
      res = MCU.uscia0.ucaxtxbuf;                                                
      MCU.sfr.ifg2.b.uca0txifg = 0;                                          
      HW_DMSG_USCIA("msp430:uscia0: read ucaxtxbuf = 0x%02x\n", res & 0xff);
      break;                                
    case UCA0ABCTL      :                                                 
      res = MCU.uscia0.ucaxabctl.s;                                               
      HW_DMSG_USCIA("msp430:uscia0: read ucaxabctl = 0x%02x [PC=0x%04x]\n", res & 0xff, mcu_get_pc()); 
      break;  
    case UCA0IRTCTL     :                                                 
      res = MCU.uscia0.ucaxirtctl.s;                                               
      HW_DMSG_USCIA("msp430:uscia0: read ucaxirtctl = 0x%02x [PC=0x%04x]\n", res & 0xff, mcu_get_pc()); 
      break;                                                                  
    case UCA0IRRCTL     :                                                 
      res = MCU.uscia0.ucaxirrctl.s;                                              
      HW_DMSG_USCIA("msp430:uscia0: read ucaxirrctl = 0x%02x\n", res & 0xff); 
      break;                                                                                                                                                                                                  
    default                 :                                                 
      res = 0;                                                                
      ERROR("msp430:uscia0: read bad address 0x%04x\n", addr);            
      break;                                                                  
    }                                                                         
  return res;                                    
}

/* uscia0 write from MCU */
void msp430_uscia0_write(uint16_t UNUSED addr, int8_t UNUSED val)
{
  switch (addr)                                                               
    {                                                                         
    case UCA0CTL0        :		/* ctrl register 0 */                                            
      { 
	/*temporary value*/
	union {                                                               
	  struct ucaxctl0_t   b; 
	  uint8_t             s;                                              
	} ctl0;                                                                                                                             
	ctl0.s = val;                                               
	/*debug message*/
	HW_DMSG_USCIA("msp430:uscia0: write ucaxctl0 = 0x%02x\n", val & 0xff);                    
	HW_DMSG_USCIA("msp430:uscia0: ucpen = %d\n", 	ctl0.b.ucpen);
	HW_DMSG_USCIA("msp430:uscia0: ucpar = %d\n", 	ctl0.b.ucpar);
	HW_DMSG_USCIA("msp430:uscia0: ucmsb = %d\n", 	ctl0.b.ucmsb);
	HW_DMSG_USCIA("msp430:uscia0: uc7bit = %d\n", 	ctl0.b.uc7bit);
	HW_DMSG_USCIA("msp430:uscia0: ucspb = %d\n", 	ctl0.b.ucspb);
	HW_DMSG_USCIA("msp430:uscia0: ucmode = %d\n", 	ctl0.b.ucmode);
	HW_DMSG_USCIA("msp430:uscia0: ucsync = %d\n", 	ctl0.b.ucsync);                                                                     
	HW_DMSG_USCIA("msp430:uscia0: length = %d bits\n",             
                                           (ctl0.b.uc7bit == 1) ? 7:8);         
        /*modifications*/                                                                          
        MCU.uscia0.ucaxctl0.s = val;
	if (ctl0.b.uc7bit != MCU.uscia0.ucaxctl0.b.uc7bit) 
	{                         
           msp430_uscia0_set_baudrate();                         
        } 
      }                                                                       
      break;
      
    case UCA0CTL1       :		/* ctrl register 1 */                                            
      { 
	/*temporary value*/
	union {                                                               
	  struct ucaxctl1_t   b; 
	  uint8_t             s;                                              
	} ctl1;                                                                                                                             
	ctl1.s = val;                                               
	/*debug message*/
	HW_DMSG_USCIA("msp430:uscia0: write ucaxctl1 = 0x%02x\n", val & 0xff);                    
	HW_DMSG_USCIA("msp430:uscia0: ucssel  = %d\n", 	ctl1.b.ucssel);
	HW_DMSG_USCIA("msp430:uscia0: ucrxeie  = %d\n", ctl1.b.ucrxeie);
	HW_DMSG_USCIA("msp430:uscia0: ucbrkie = %d\n", 	ctl1.b.ucbrkie);
	HW_DMSG_USCIA("msp430:uscia0: ucdorm = %d\n", 	ctl1.b.ucdorm);
	HW_DMSG_USCIA("msp430:uscia0: uctxaddr = %d\n", ctl1.b.uctxaddr);
	HW_DMSG_USCIA("msp430:uscia0: uctxbrk = %d\n", 	ctl1.b.uctxbrk);
	HW_DMSG_USCIA("msp430:uscia0: ucswrst = %d\n", 	ctl1.b.ucswrst);

        /*modifications*/                                                                          
	if ((ctl1.b.ucswrst == 0) && (MCU.uscia0.ucaxctl1.b.ucswrst == 0))       
          {                                                                   
             /* reset  UCA0RXIE, UCA0TXIE, UCA0RXIFG, UCOE, and UCFE bits */            
             /* set    UCA0TXIFG flag                                     */           
             HW_DMSG_USCIA("msp430:uscia0:   swrst  = 1, reset flags\n");                                             
             MCU.sfr.ie2.b.uca0rxie           = 0;                                 
             MCU.sfr.ie2.b.uca0txie           = 0;                                 
             MCU.sfr.ifg2.b.uca0rxifg         = 0; 
	     MCU.sfr.ifg2.b.uca0txifg         = 1; 
             MCU.uscia0.ucaxstat.b.ucoe       = 0;                                 
             MCU.uscia0.ucaxstat.b.ucfe      = 0;                                                                     
             /* finishing current transaction */                              
             MCU.uscia0.ucaxtxbuf_full        = 0;                                 
             MCU.uscia0.ucaxtx_full_delay     = 0;             
             MCU.uscia0.ucaxtx_shift_empty    = 1;                                 
             MCU.uscia0.ucaxrxbuf_full        = 0;                                 
             MCU.uscia0.ucaxrx_shift_empty    = 1;                                 
          }
          
          MCU.uscia0.ucaxctl1.s = val;  
          if (ctl1.b.ucssel != MCU.uscia0.ucaxctl1.b.ucssel)                           
          {                                                                   
             msp430_uscia0_set_baudrate();                       
          }             
      }                                                                       
      break;
      
     case UCA0BR0       : 		/* baud rate ctrl register 0 */                 
      HW_DMSG_USCIA("msp430:uscia0: write ucaxbr0  = 0x%02x\n",val & 0xff);
      MCU.uscia0.ucaxbr0 = val;                                                  
      msp430_uscia0_set_baudrate();                              
      break;     
      
     case UCA0BR1        : 		/* baud rate ctrl register 1 */                 
      HW_DMSG_USCIA("msp430:uscia0: write ucaxbr1  = 0x%02x\n",val & 0xff);
      MCU.uscia0.ucaxbr1 = val;                                                  
      msp430_uscia0_set_baudrate();                              
      break;
      
     case UCA0MCTL       :		/* Modulation ctrl register */                                            
	{ 
	  /*temporary value*/
	  union {                                                               
	    struct ucaxmctl_t   b; 
	    uint8_t             s;                                              
	  } mctl;                                                                                                                             
	  mctl.s = val;                                               
	  /*debug message*/
	  HW_DMSG_USCIA("msp430:uscia0: write ucaxctl1 = 0x%02x\n", val & 0xff);                    
	  HW_DMSG_USCIA("msp430:uscia0: ucbrf  = %d\n", 	mctl.b.ucbrf);
	  HW_DMSG_USCIA("msp430:uscia0: ucbrs  = %d\n", 	mctl.b.ucbrs);
	  HW_DMSG_USCIA("msp430:uscia0: ucos16 = %d\n", 	mctl.b.ucos16);
	  
	  /*modifications*/                                                                          
	  MCU.uscia0.ucaxmctl.s = val;  
	  msp430_uscia0_set_baudrate(); 
	}                                                                       
	break;
      
      case UCA0STAT       :		/* Status ctrl register */ 
	{
	  /*temporary value*/
	  union {                                                               
	    struct ucaxstat_t   b;
	    uint8_t             s;                                              
	  } stat;                                                                                                                                      
	  stat.s = val;          
	  /*debug message*/
	  HW_DMSG_USCIA("msp430:uscia0: write ucbxstat = 0x%02x\n", val & 0xff);                   
	  HW_DMSG_USCIA("msp430:uscia0: uclisten = %d\n",stat.b.uclisten);
	  HW_DMSG_USCIA("msp430:uscia0: ucfe = %d\n",	 stat.b.ucfe);
	  HW_DMSG_USCIA("msp430:uscia0: ucoe = %d\n", 	 stat.b.ucoe);
	  HW_DMSG_USCIA("msp430:uscia0: ucpe = %d\n", 	 stat.b.ucpe);
	  HW_DMSG_USCIA("msp430:uscia0: ucbrk = %d\n", 	 stat.b.ucbrk);
	  HW_DMSG_USCIA("msp430:uscia0: ucrxerr = %d\n", stat.b.ucrxerr);
	  HW_DMSG_USCIA("msp430:uscia0: ucaddr = %d\n",  stat.b.ucaddr);
	  HW_DMSG_USCIA("msp430:uscia0: ucbusy = %d\n",	 stat.b.ucbusy);
	  
	  /*modifications*/
	  MCU.uscia0.ucaxstat.s = val;
	}
	break;
	
      case UCA0RXBUF      :                                                 
	ERROR("msp430:uscia0: writing to read only register RXBUFF\n");    
	break;  
	
      case UCA0TXBUF      :    
	{
	  HW_DMSG_USCIA("msp430:uscia0: write ucaxtxbuf  = 0x%02x [PC=0x%04x]\n",val &0xff, mcu_get_pc());
	  if (MCU.uscia0.ucaxtxbuf_full == 1)
	  {                                                                     
	    WARNING("msp430:uscia0:    overwriting tx buffer with [0x%02x]\n",val & 0xff);                                              
	  }                                                                     
	  MCU.uscia0.ucaxtxbuf            = val;                                     
	  MCU.uscia0.ucaxtxbuf_full       = 1;                                       
	  MCU.uscia0.ucaxtx_full_delay    = 1; /* go to shifter after xx BITCLK */                                      
	  MCU.sfr.ifg2.b.uca0txifg        = 0;                                                                             \
	  /*TRACER_TRACE_USCIA(TRACER_USCIA_TX_RECV);*/			     
	  /* can be a dupe from the platform file */                            
	  /*etracer_slot_event(ETRACER_PER_ID_MCU_USCIA,                    
			    ETRACER_PER_EVT_WRITE_COMMAND,                     
			    ETRACER_PER_ARG_WR_DST_FIFO, 0);*/                                                                                                                                                                                                     
	}
	break; 
      
      case UCA0IRTCTL       :		/* IrDA Transmit ctrl register */ 
	{
	  /*temporary value*/
	  union {                                                               
	    struct ucaxirtctl_t   b;
	    uint8_t               s;                                              
	  } irtctl;                                                                                                                                      
	  irtctl.s = val;          
	  /*debug message*/
	  HW_DMSG_USCIA("msp430:uscia0: write ucbxabctl = 0x%02x\n", val & 0xff);                   
	  HW_DMSG_USCIA("msp430:uscia0: ucirtxpl  = %d\n",  irtctl.b.ucirtxpl);
	  HW_DMSG_USCIA("msp430:uscia0: ucirtxclk = %d\n",  irtctl.b.ucirtxclk);
	  HW_DMSG_USCIA("msp430:uscia0: uciren    = %d\n",  irtctl.b.uciren);
	  
	  /*modifications*/
	  MCU.uscia0.ucaxirtctl.s = val;
	}
	break;
	
      case UCA0IRRCTL       :		/* IrDA Receive ctrl register */ 
	{
	  /*temporary value*/
	  union {                                                               
	    struct ucaxirrctl_t   b;
	    uint8_t              s;                                              
	  } irrctl;                                                                                                                                      
	  irrctl.s = val;          
	  /*debug message*/
	  HW_DMSG_USCIA("msp430:uscia0: write ucbxabctl = 0x%02x\n", val & 0xff);                   
	  HW_DMSG_USCIA("msp430:uscia0: ucirrxfl  = %d\n",  irrctl.b.ucirrxfl);
	  HW_DMSG_USCIA("msp430:uscia0: ucirrxpl   = %d\n", irrctl.b.ucirrxpl);
	  HW_DMSG_USCIA("msp430:uscia0: ucirrxfe   = %d\n", irrctl.b.ucirrxfe);
	  /*modifications*/
	  MCU.uscia0.ucaxirrctl.s = val;
	}
	break;
	
      case UCA0ABCTL        :		/* Auto Baud Rate ctrl register */ 
	{
	/*temporary value*/
	  union {                                                               
	    struct ucaxabctl_t   b;
	    uint8_t              s;                                              
	  } abctl;                                                                                                                                      
	  abctl.s = val;          
	  /*debug message*/
	  HW_DMSG_USCIA("msp430:uscia0: write ucbxabctl = 0x%02x\n", val & 0xff);                   
	  HW_DMSG_USCIA("msp430:uscia0: reserved0 = %d\n",  abctl.b.reserved0);
	  HW_DMSG_USCIA("msp430:uscia0: ucdelim  = %d\n",   abctl.b.ucdelim);
	  HW_DMSG_USCIA("msp430:uscia0: ucstoe   = %d\n",   abctl.b.ucstoe);
	  HW_DMSG_USCIA("msp430:uscia0: ucbtoe   = %d\n",   abctl.b.ucbtoe);
	  HW_DMSG_USCIA("msp430:uscia0: reserved1 = %d\n",  abctl.b.reserved1);
	  HW_DMSG_USCIA("msp430:uscia0: ucabden  = %d\n",   abctl.b.ucabden);
    
	  /*modifications*/
	  MCU.uscia0.ucaxabctl.s = val;
	}
	break;
	
      }  /* switch */   
}

/* uscia0 chk ifg for MCU interrupt */
int msp430_uscia0_chkifg()
{
   int ret = 0;                                                               
   if (MCU.sfr.ifg2.b.uca0txifg  && MCU.sfr.ie2.b.uca0txie)                  
     {                                                                        
        msp430_interrupt_set(INTR_USCIX0_TX);                           
        ret = 1;                                                              
     }                                                                        
   if (MCU.sfr.ifg2.b.uca0rxifg && MCU.sfr.ie2.b.uca0rxie)                  
     {                                                                        
        msp430_interrupt_set(INTR_USCIX0_RX);                           
        ret = 1;                                                              
     }                                                                        
   return ret;
}

/*******************************************/
/********* External Peripheral API *********/

/* uscia0 SPI read from peripherals */
int msp430_uscia0_dev_read_uart(uint8_t *val)
{
      int ret = 0;
      if (MCU.uscia0.ucaxtx_shift_ready == 1)                                    
	{                                                                     
	  *val = MCU.uscia0.ucaxtx_shift_buf;                                     
	  MCU.uscia0.ucaxtx_shift_ready = 0;                                     
	  MCU.uscia0.ucaxtx_shift_empty = 1;                                     
          /*TRACER_TRACE_USCIA(TRACER_UART_IDLE);*/			      
	  ret = 1;                          
	}
      return ret;
}

/* uscia0 SPI write from peripherals */
void msp430_uscia0_dev_write_uart(uint8_t val)
{                                                                
      if (MCU.uscia0.ucaxrx_shift_empty != 1)                                    
	{                                                                     
	  ERROR("msp430:uscia0: USCIA rx value while rx shift not empty (%d)\n", 
			      MCU.uscia0.ucaxrx_shift_delay);                            
	}                                                                     
      else                                                                    
        {                                                                     
	  /*TRACER_TRACE_USCIA(TRACER_UART_RX_RECV);*/
          MCU.uscia0.ucaxrx_shift_buf   = val;                                   
          MCU.uscia0.ucaxrx_shift_empty = 0;                                     
          MCU.uscia0.ucaxrx_shift_ready = 0;                                     
          MCU.uscia0.ucaxrx_shift_delay = MCU.uscia0.ucaxbr_div;                    
          HW_DMSG_USCIA("msp430:uscia0: USCIA rx shift reg value 0x%02x\n",val);    
        }                                                                     
}

/* uscia0 SPI verification */
int msp430_uscia0_dev_write_uart_ok()
{
    return MCU.uscia0.ucaxrx_shift_empty == 1;
}



#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

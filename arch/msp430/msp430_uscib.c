
/**
 *  \file   msp430_uscib.c
 *  \brief  MSP430 USCIB definition (based on "msp430_usart.c" SPI MODE)
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

#define HW_SPY_MODE 0

#if HW_SPY_MODE != 0
#define HW_SPY(x...) HW_DMSG_MCUDEV(x)
#else
#define HW_SPY(x...) do { } while (0)
#endif

/* defined in msp430_debug.h */
#if defined(DEBUG_USCIB)  
char *str_ssel[] = { "external UCLK", "ACLK", "SMCLK", "SMCLK" };
#endif

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
// #define TRACER_SPI_TX_RECV   6
// #define TRACER_SPI_TX_READY  7
// #define TRACER_SPI_RX_RECV   8
// #define TRACER_SPI_RX_READY  9
// 
// #define TRACER_I2C_TX_RECV   11
// #define TRACER_I2C_TX_READY  12
// #define TRACER_I2C_RX_RECV   13
// #define TRACER_I2C_RX_READY  14


/*****************************************************/
/*****************************************************/
/*****************************************************/

//slau144e.pdf (page 497)
/**
 * In synchronous mode, the USCI connects the MSP430 to an externalvia three or four pins: UCxSIMO, UCxSOMI, UCxCLK, and UCxSmode is selected when the UCSYNC bit is set and SPI mode (3-pin ois selected with the UCMODEx bits.
 * SPI mode features include:
 * - 7- or 8-bit data length
 * - LSB-first or MSB-first data transmit and receive
 * - 3-pin and 4-pin SPI operation
 * - Master or slave modes
 * - Independent transmit and receive shift registers
 * - Separate transmit and receive buffer registers
 * - Continuous transmit and receive operation
 * - Selectable clock polarity and phase control
 * - Programmable clock frequency in master mode
 * - Independent interrupt capability for receive and transmit
 * - Slave operation in LPM4
 * Fig 16−1 shows the USCI when configured for SPI mode.
**/


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(HW_SPY_MODE)
#define HW_SPY_PRINT_CONFIG()					         \
  HW_SPY("msp430:uscib0: ============\n");                               \
  HW_SPY("msp430:uscib0:ctl0  %x\n",  MCU.uscib0.ucbxctl0.s);            \
  HW_SPY("msp430:uscib0:ctl   %x\n",  MCU.uscib0.ucbxctl1.s);            \
  HW_SPY("msp430:uscib0:br0   %x\n",  MCU.uscib0.ucbxbr0);               \
  HW_SPY("msp430:uscib0:br1   %x\n",  MCU.uscib0.ucbxbr1);               \
  HW_SPY("msp430:uscib0:stat  %x\n",  MCU.uscib0.ucbxstat);              \
  HW_SPY("msp430:uscib0:rxbuf %x\n",  MCU.uscib0.ucbxrxbuf);             \
  HW_SPY("msp430:uscib0:txbuf %x\n",  MCU.uscib0.ucbxtxbuf);             \
  HW_SPY("msp430:uscib0: ============\n");                            
#else                                                                         
#define HW_SPY_PRINT_CONFIG() do {} while(0)
#endif

                       

#if defined(__msp430_have_uscib0)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_uscib0_set_baudrate()                                  
{   

  MCU.uscib0.ucbxbr_div = ((MCU.uscib0.ucbxbr1 << 8 | MCU.uscib0.ucbxbr0) / 2) * (7+!(MCU.uscib0.ucbxctl0.b.uc7bit)); 
  HW_DMSG_USCIB("msp430:uscib0:   baudrate = div N %d/bit - %d/byte\n",      
		  ((MCU.uscib0.ucbxbr1 << 8 | MCU.uscib0.ucbxbr0) / 2),MCU.uscib0.ucbxbr_div); 

}

void msp430_uscib0_reset()
{

  MCU.uscib0.ucbxctl1.b.ucswrst     = 1; 
  
  MCU.uscib0.ucbxrxbuf              = 0;
  MCU.uscib0.ucbxrxbuf_full         = 0;
  MCU.uscib0.ucbxrx_shift_buf       = 0;
  MCU.uscib0.ucbxrx_shift_empty     = 1;
  MCU.uscib0.ucbxrx_shift_ready     = 0;
  MCU.uscib0.ucbxrx_shift_delay     = 0;
  MCU.uscib0.ucbxrx_slave_rx_done   = 0;
                                   
  MCU.uscib0.ucbxtxbuf              = 0;
  MCU.uscib0.ucbxtxbuf_full         = 0;
  MCU.uscib0.ucbxtx_full_delay      = 0;                                    
  MCU.uscib0.ucbxtx_shift_empty     = 1;
  MCU.uscib0.ucbxtx_shift_ready     = 0;
  MCU.uscib0.ucbxtx_shift_delay     = 0;

}

/***************************/
/********* MCU API *********/

/* update uscib0 within internal device loop */
void msp430_uscib0_update()
{
  do {                                                                                                                                                
      /* Tx buffer */
      if (MCU.uscib0.ucbxtxbuf_full == 1)                                        
        {                                                                     
	  if (MCU.uscib0.ucbxtx_shift_empty == 1 &&      
	      (MCU.uscib0.ucbxctl0.b.ucmst == 1 || MCU.uscib0.ucbxrx_slave_rx_done == 1))
	    /* if SPI in slave mode, tx is done immediately as delay has  */  
	    /* been taken into account by the SPI master device on rx */      
            {                                                                 
               MCU.uscib0.ucbxtx_shift_buf   = MCU.uscib0.ucbxtxbuf;                
               MCU.uscib0.ucbxtxbuf_full     = 0;                                
               MCU.uscib0.ucbxtx_full_delay  = 0;                                
               MCU.uscib0.ucbxtx_shift_empty = 0;                                
               MCU.uscib0.ucbxtx_shift_ready = 0;   
        
	       if (MCU.uscib0.ucbxctl0.b.ucmst == 1)  /* master mode */     
		 {
		   //UC7BIT, character length : 1 = 7bit ; 0 = 8bit
		   MCU.uscib0.ucbxtx_shift_delay = 7+!(MCU.uscib0.ucbxctl0.b.uc7bit);
		 }      
	       else      
		 {      
		   MCU.uscib0.ucbxtx_shift_delay = 0;      
		   MCU.uscib0.ucbxrx_slave_rx_done = 0;	      
		 }
              
		MCU.sfr.ifg2.b.ucb0txifg  = 1;                                         
		if (MCU.sfr.ie2.b.ucb0txie)                                 
                 {                                                            
                    msp430_interrupt_set(INTR_USCIB0_TX);           
                 }   
                 
                 
               HW_DMSG_USCIB("msp430:uscib0: SPI tx buf -> shifter (%d)\n",  
			     MCU.uscib0.ucbxtx_shift_delay);		      
	       HW_SPY("msp430:uscib0: SPI send (0x%x,%c) at 0x%04x\n",   
		       MCU.uscib0.ucbxtxbuf, isprint(MCU.uscib0.ucbxtxbuf) ?	      
		       MCU.uscib0.ucbxtxbuf : '.', mcu_get_pc());		      
            }                                                                 
         } /* ucbxtfubuf_full */                                                
                                                                              
      /* TX shift register */                                                 
      if (MCU.uscib0.ucbxtx_shift_empty == 0)                                    
         {                                                                    
            switch (MCU.uscib0.ucbxctl1.b.ucssel)                                  
              {                                                               
            case 0: // NA                                                     
              ERROR("msp430:usci0: SPI TX clk = UCLK, not supported\n");
	      MCU.uscib0.ucbxtx_shift_delay = 0;                                 
              break;                                                          
            case 1: // ACLK                                                          
              MCU.uscib0.ucbxtx_shift_delay -= MCU_CLOCK.ACLK_increment;         
	      HW_DMSG_USCIB("msp430:uscib0, aclk SPI delay %d\n",
			     MCU.uscib0.ucbxtx_shift_delay);                
              break;                                                          
            case 2: // SMCLK                                                          
            case 3: // SMCLK                                                         
              MCU.uscib0.ucbxtx_shift_delay -= MCU_CLOCK.SMCLK_increment;        
	      HW_DMSG_USCIB("msp430:uscib0, smclk SPI delay %d\n",
			     MCU.uscib0.ucbxtx_shift_delay);                  
              break;                                                          
              }                                                               
                                                                              
            if (MCU.uscib0.ucbxtx_shift_delay <= 0)                              
              {                                                               
               /*tracer_event_record(TRACER_uscib0##NUM,TRACER_SPI_TX_READY);*/
                 MCU.uscib0.ucbxtx_shift_ready = 1;                              
                 //MCU.uscib0.ucbxctl1.b.txept   = 1;         //empty flag MSP430x2xx ??                      
              }                                                               
         } /* tx_shift_empty */                                               
                                                                              
      /* RX shift register */                                              
      if (MCU.uscib0.ucbxrx_shift_empty == 0)                                    
        {                                                                     
          if (MCU.uscib0.ucbxstat.b.uclisten == 1)                          
            {                                                                 
               ERROR("msp430:uscib0: SPI listen mode not supported\n");  
            }                                                                 
                                                                              
          switch (MCU.uscib0.ucbxctl1.b.ucssel)                                    
            {                                                                 
          case 0:                                                             
            ERROR("msp430:uscib0: SPI RX clk = UCLK not supported\n");   
            MCU.uscib0.ucbxrx_shift_delay = 0;                                   
            break;                                                            
          case 1:                                                             
            MCU.uscib0.ucbxrx_shift_delay -= MCU_CLOCK.ACLK_increment;           
            break;                                                            
          case 2:                                                             
          case 3:                                                             
            MCU.uscib0.ucbxrx_shift_delay -= MCU_CLOCK.SMCLK_increment;          
            break;                                                            
            }                                                                 
                                                                              
          if (MCU.uscib0.ucbxrx_shift_delay <= 0)                                
            {                                                                 
               MCU.uscib0.ucbxrx_shift_ready = 1;                                
            }                                                                 
        } /* shift empty */                                                   
                                                                              
      if (MCU.uscib0.ucbxrx_shift_ready == 1)                                    
        {                                                                     
          if (MCU.uscib0.ucbxrxbuf_full == 1)                                    
            {                                                                 
               MCU.uscib0.ucbxstat.b.ucoe = 1;                                     
               /* sends out lots of debug message for burst writes */         
               /* ERROR("msp430:uscib0: SPI Rx Overrun (0x%x,%c)\n", */  
	       /*	     MCU.uscib0.ucbxrx_shift_buf,		*/	
	       /*   isgraph(MCU.uscib0.ucbxrx_shift_buf) ?	*/		
	       /*   MCU.uscib0.ucbxrx_shift_buf : '.');	*/		
            }                                                                 
          /*tracer_event_record(TRACER_uscib0, TRACER_SPI_RX_READY);*/    
          MCU.uscib0.ucbxrxbuf          = MCU.uscib0.ucbxrx_shift_buf;              
          MCU.uscib0.ucbxrxbuf_full     = 1;                                     
          MCU.uscib0.ucbxrx_shift_ready = 0;                                     
          MCU.uscib0.ucbxrx_shift_empty = 1;                                     
          HW_DMSG_USCIB("msp430:uscib0: SPI rx shifter -> rx buf\n");    
	  HW_SPY("msp430:uscib0: SPI receive (0x%x,%c)\n",	              
                  MCU.uscib0.ucbxrxbuf, isgraph(MCU.uscib0.ucbxrxbuf) ?	      
                  MCU.uscib0.ucbxrxbuf : '.');	                              
          MCU.sfr.ifg2.b.ucb0rxifg  = 1;                                     
          if (MCU.sfr.ie2.b.ucb0rxie)                                        
            {                                                                 
               msp430_interrupt_set(INTR_USCIB0_RX);                    
            }                                                                 
        } /* shift ready */                                                   
    } /* urxe. spie */                                                        
    while (0);  
}

/* uscib0 read from MCU */
int8_t msp430_uscib0_read(uint16_t addr)
{
  int8_t res = 0;

  switch (addr)                                                              
    {                                                                         
    case UCB0CTL0 :                                                 
      res = MCU.uscib0.ucbxctl0.s;                                               
      HW_DMSG_USCIB("msp430:uscib0: read ucbxctl0 = 0x%02x\n",res & 0xff);  
      break;                                                                 
    case UCB0CTL1 :                                                 
      res = MCU.uscib0.ucbxctl1.s;                                               
      HW_DMSG_USCIB("msp430:uscib0: read ucbxctl1 = 0x%02x\n",res & 0xff); 
      break;                                                                  
    case UCB0BR0 :                                                 
      res = MCU.uscib0.ucbxbr0;                                               
      HW_DMSG_USCIB("msp430:uscib0: read ucbxbr0 = 0x%02x\n",res & 0xff); 
      break;                                                                  
    case UCB0BR1 :                                                 
      res = MCU.uscib0.ucbxbr1;                                                 
      HW_DMSG_USCIB("msp430:uscib0: read ucbxbr1 = 0x%02x\n",res & 0xff);
      break;                                                                  
    case UCB0STAT :                                                 
      res = MCU.uscib0.ucbxstat.s;                                                  
      HW_DMSG_USCIB("msp430:uscib0: read ucbxstat = 0x%02x\n",res & 0xff);  
      break;                                                                  
    case UCB0RXBUF :                                                 
      res = MCU.uscib0.ucbxrxbuf;                                                
      MCU.sfr.ifg2.b.ucb0rxifg   = 0;                                          
      MCU.uscib0.ucbxrxbuf_full  = 0; 
      /*TRACER_TRACE_USCIB0(TRACER_USCIB0_IDLE);
      etracer_slot_event(ETRACER_PER_ID_MCU_SPI,
                           ETRACER_PER_EVT_READ_COMMAND,
                          (ETRACER_PER_ARG_WR_DST_FIFO | ETRACER_ACCESS_LVL_BUS), 0);*/
      MCU.uscib0.ucbxstat.b.ucoe = 0;                                          
      HW_DMSG_USCIB("msp430:uscib0: read ucbxrxbuf = 0x%02x\n",res & 0xff);
      break;                                                                                                                                
    case UCB0TXBUF :                                                 
      res = MCU.uscib0.ucbxtxbuf;                                                
      MCU.sfr.ifg2.b.ucb0txifg = 0;                                          
      HW_DMSG_USCIB("msp430:uscib0: read ucbxtxbuf = 0x%02x\n",res & 0xff);
      break;                                                                  
    default :                                                 
      res = 0;                                                                
      ERROR("msp430:uscib0: read bad address 0x%04x\n",addr);           
      break;                                                                  
    }   

  return res;  
}

/* uscib0 write from MCU */
void msp430_uscib0_write(uint16_t addr, int8_t val)
{
  switch (addr)                                                               
    {                                                                         
    case UCB0CTL0        :		/* ctrl register 0 */                                            
      { 
	/*temporary value*/
	union {                                                               
	  struct ucbxctl0_t   b;
	  uint8_t             s;                                              
	} ctl0;                                                                                                                             
	ctl0.s = val;                                               
	/*debug message*/
	HW_DMSG_USCIB("msp430:uscib0: write ucbxctl0 = 0x%02x\n", val & 0xff);                    
	HW_DMSG_USCIB("msp430:uscib0: ucckph = %d\n", 	ctl0.b.ucckph);
	HW_DMSG_USCIB("msp430:uscib0: ucckpl = %d\n", 	ctl0.b.ucckpl);
	HW_DMSG_USCIB("msp430:uscib0: ucmsb = %d\n", 	ctl0.b.ucmsb);
	HW_DMSG_USCIB("msp430:uscib0: uc7bit = %d\n", 	ctl0.b.uc7bit);
	HW_DMSG_USCIB("msp430:uscib0: ucmst = %d\n", 	ctl0.b.ucmst);
	HW_DMSG_USCIB("msp430:uscib0: ucmode = %d\n", 	ctl0.b.ucmode);
	HW_DMSG_USCIB("msp430:uscib0: ucsync = %d\n", 	ctl0.b.ucsync);                                                                     
	HW_DMSG_USCIB("msp430:uscib0:   length = %d bits\n",             
                                           (ctl0.b.uc7bit == 1) ? 7:8);         
        /*modifications*/                                                                          
        MCU.uscib0.ucbxctl0.s = val;
	if (ctl0.b.uc7bit != MCU.uscib0.ucbxctl0.b.uc7bit) {                         
           msp430_uscib0_set_baudrate();                         
        } 
      }                                                                       
      break;                                                                  
             
      
    case UCB0CTL1       :		/* ctrl register 1 */                
      {                                                                       
	/*temporary value*/
	union {                                                               
	  struct ucbxctl1_t   b;
	  uint8_t             s;                                              
	} ctl1;                                                                                                                                      
	ctl1.s = val;                                                                   
	/*debug message*/
        HW_DMSG_USCIB("msp430:uscib0: write ucbxctl1 = 0x%02x\n", val & 0xff);                   
        HW_DMSG_USCIB("msp430:uscib0: ucssel = %d\n", 	ctl1.b.ucssel);
	HW_DMSG_USCIB("msp430:uscib0: unused = %d\n", 	ctl1.b.unused);
	HW_DMSG_USCIB("msp430:uscib0: ucswrst = %d\n", 	ctl1.b.ucswrst);
	/*modifications*/
	if ((ctl1.b.ucswrst == 0) && (MCU.uscib0.ucbxctl1.b.ucswrst == 0))       
          {                                                                   
             /* reset  UCB0RXIE, UCB0TXIE, UCB0RXIFG, UCOE, and UCFE bits */            
             /* set    UCB0TXIFG flag                                     */           
             HW_DMSG_USCIB("msp430:uscib0:   swrst  = 1, reset flags\n");                                             
             MCU.sfr.ie2.b.ucb0rxie           = 0;                                 
             MCU.sfr.ie2.b.ucb0txie           = 0;                                 
             MCU.sfr.ifg2.b.ucb0rxifg         = 0; 
	     MCU.sfr.ifg2.b.ucb0txifg         = 1; 
             MCU.uscib0.ucbxstat.b.ucoe       = 0;                                 
             MCU.uscib0.ucbxstat.b.ucfce      = 0;                                                                     
             /* finishing current transaction */                              
             MCU.uscib0.ucbxtxbuf_full        = 0;                                 
             MCU.uscib0.ucbxtx_full_delay     = 0;             
             MCU.uscib0.ucbxtx_shift_empty    = 1;                                 
             MCU.uscib0.ucbxrxbuf_full        = 0;                                 
             MCU.uscib0.ucbxrx_shift_empty    = 1;                                 
          }  
          MCU.uscib0.ucbxctl1.s = val;    
	}
	break;  

	
    case UCB0BR0        :		/* baud rate ctrl register 0 */                 
      /*debug message*/
      HW_DMSG_USCIB("msp430:uscib0: write ucbxbr0  = 0x%02x\n", val & 0xff);
      /*modifications*/
      MCU.uscib0.ucbxbr0 = val;                                                  
      msp430_uscib0_set_baudrate();                              
      break;  
      
      
    case UCB0BR1        :		/* baud rate ctrl register 1 */                 
      /*debug message*/
      HW_DMSG_USCIB("msp430:uscib0: write ucbxbr1  = 0x%02x\n", val & 0xff);
      /*modifications*/
      MCU.uscib0.ucbxbr1 = val;                                                  
      msp430_uscib0_set_baudrate();                            
      break;   
      
      
    case UCB0STAT       :		/* Status register */  
      {
      /*temporary value*/
	union {                                                               
	  struct ucbxstat_t   b;
	  uint8_t             s;                                              
	} stat;                                                                                                                                      
	stat.s = val;          
	/*debug message*/
        HW_DMSG_USCIB("msp430:uscib0: write ucbxstat = 0x%02x\n", val & 0xff);                   
        HW_DMSG_USCIB("msp430:uscib0: uclisten = %d\n", stat.b.uclisten);
	HW_DMSG_USCIB("msp430:uscib0: ucfce = %d\n", 	stat.b.ucfce);
	HW_DMSG_USCIB("msp430:uscib0: unused = %d\n", 	stat.b.unused);
	HW_DMSG_USCIB("msp430:uscib0: ucbusy = %d\n", 	stat.b.ucbusy);
	/*modifications*/
	MCU.uscib0.ucbxstat.s = val;
      }
      break;
      
                                                             
    case UCB0RXBUF      :                                                
      ERROR("msp430:uscib0: writing to read only register UCB0RXBUF\n");    
      break;                
      
      
    case UCB0TXBUF      :    
      {
      HW_DMSG_USCIB("msp430:uscib0: write ucbxtxbuf  = 0x%02x [PC=0x%04x]\n",val & 0xff, 
		    mcu_get_pc());					
      if ((MCU.uscib0.ucbxtxbuf_full == 1) && (MCU.uscib0.ucbxctl0.b.ucmst == 1)) /* master */ 
        {                                                                     
          WARNING("msp430:uscib0:    overwriting tx buffer with [0x%02x]\n",val & 0xff);                                              
        }                                                                     
      MCU.uscib0.ucbxtxbuf               = val;                                     
      MCU.uscib0.ucbxtxbuf_full          = 1;                                       
      MCU.uscib0.ucbxtx_full_delay       = 1;                                   
      MCU.sfr.ifg2.b.ucb0txifg           = 0;                                       
      /*TRACER_TRACE_USCIB0(TRACER_USCIB0_TX_RECV); 
      etracer_slot_event(ETRACER_PER_ID_MCU_USCIB0,                      
                           ETRACER_PER_EVT_WRITE_COMMAND,                     
                           ETRACER_PER_ARG_WR_DST_FIFO | ETRACER_ACCESS_LVL_BUS, 0); */
      
        HW_SPY("msp430:uscib0: write byte %x\n",val & 0xff);         
        HW_SPY_PRINT_CONFIG();                                      
                                                      
      }                                                                       
      break;    
 
    }  /* switch */                     
}

/* uscib0 chk ifg for MCU interrupt */
int msp430_uscib0_chkifg()
{
  int ret = 0;                                                               
   if (MCU.sfr.ifg2.b.ucb0txifg  && MCU.sfr.ie2.b.ucb0txie)                  
     {                                                                        
        msp430_interrupt_set(INTR_USCIB0_TX);                           
        ret = 1;                                                              
     }                                                                        
   if (MCU.sfr.ifg2.b.ucb0rxifg && MCU.sfr.ie2.b.ucb0rxie)                  
     {                                                                        
        msp430_interrupt_set(INTR_USCIB0_RX);                           
        ret = 1;                                                              
     }                                                                        
   return ret;
}

/*******************************************/
/********* External Peripheral API *********/

/* uscib0 SPI read from peripherals */
int msp430_uscib0_dev_read_spi(uint8_t *val)
{
                                                                                                                 
    if (MCU.uscib0.ucbxtx_shift_ready == 1)                                   
      {                                                                    
	  *val = MCU.uscib0.ucbxtx_shift_buf;                                     
	  MCU.uscib0.ucbxtx_shift_ready = 0;                                     
	  MCU.uscib0.ucbxtx_shift_empty = 1;                                     
          /*TRACER_TRACE_USCIB0(TRACER_USCIB0_IDLE);			      
          etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                    
                             ETRACER_PER_EVT_WRITE_COMMAND,                   
                            (ETRACER_PER_ARG_WR_SRC_FIFO | (ETRACER_ACCESS_LVL_SPI + NUM)), 0); */
	  return 1;                                                           
      }
    else
      
	  return 0;

}

/* uscib0 SPI write from peripherals */
void msp430_uscib0_dev_write_spi(uint8_t val)
{
                                                                   
    //TRACER_TRACE_USCIB0(TRACER_SPI_RX_RECV);      
    MCU.uscib0.ucbxrx_shift_buf   = val;                                   
    MCU.uscib0.ucbxrx_shift_empty = 0;                                     
    MCU.uscib0.ucbxrx_shift_ready = 0;      
    /* no delay, as delay has been taken into account during tx*/	      
    MCU.uscib0.ucbxrx_shift_delay = 0;      
    
    if (MCU.uscib0.ucbxctl0.b.ucmst == 0)
      {
	    MCU.uscib0.ucbxrx_slave_rx_done = 1;      
          /*etracer_slot_event(ETRACER_PER_ID_MCU_SPI + NUM,                    
                             ETRACER_PER_EVT_WRITE_COMMAND,                   
                            (ETRACER_PER_ARG_WR_DST_FIFO | (ETRACER_ACCESS_LVL_SPI + NUM)), 0);*/
          HW_DMSG_USCIB("msp430:uscib0: SPI rx value 0x%02x\n",val);  
       }                                                                     
     
}

/* uscib0 SPI verification */
int msp430_uscib0_dev_write_spi_ok()
{
  return MCU.uscib0.ucbxrx_shift_empty == 1;
}



#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

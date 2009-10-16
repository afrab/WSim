
/**
 *  \file   msp430_digiIO.c
 *  \brief  MSP430 Digital IO ports definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/** 
 * interrupt enable for port 1 and 2 
 * 8bit register
 * not taken into account if PxSEL(port,bit) != 0
 */
#define DIGIIO_IEN(p)    MCU.digiIO.int_enable[p]

/** 
 * edge select
 * 8bit register
 * 0: IFG is set on low to high transition 
 * 1: IFG is set on high to low transition
 */
#define DIGIIO_IES(p)    MCU.digiIO.int_edge_select[p]

/** 
 * interrupt flag
 * 8bit register
 * 0: no interrupt is pending
 * 1: an interrupt is pending
 */
#define DIGIIO_IFG(p)    MCU.digiIO.ifg[p]

/** 
 * input register 
 * 8bit register
 */
#define DIGIIO_IN(p)     MCU.digiIO.in[p]

/** 
 * output register 
 * 8bit register
 */

#define DIGIIO_OUT(p)    MCU.digiIO.out[p]

/* slau056e.pdf page 9-3 */

/**
 * bit access pattern for in/out updated flags 
 */

#define DIGIIO_P0  0x01
#define DIGIIO_P1  0x02
#define DIGIIO_P2  0x04
#define DIGIIO_P3  0x08
#define DIGIIO_P4  0x10
#define DIGIIO_P5  0x20

/* #define DIGIIO_IN_UP(p)      (MCU.digiIO.in_updated   & DIGIIO_P##p) */
/* #define DIGIIO_OUT_UP(p)     (MCU.digiIO.out_updated  & DIGIIO_P##p) */

//#define DIGIIO_IN_SET_UP(p)   do { MCU.digiIO.in_updated  |= DIGIIO_P##p; } while (0)
//#define DIGIIO_OUT_SET_UP(p)  do { MCU.digiIO.out_updated |= DIGIIO_P##p; } while (0)

/* MCU can write on output port even if this port is selected IN */
/* see Figure page 49 of [msp430f1611.pdf]                       */
/* #define _WOK(p)    _DIR(p) & ~_SEL(p) */
#define DIGIIO_MCU_WOK(p)    ~DIGIIO_SEL(p)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_reset()
{
  /* after a reset the pin IO are switched to input mode */
  int i;
  for(i=0; i<6; i++)
    {
      DIGIIO_DIR(i) = 0;
      DIGIIO_SEL(i) = 0;
    }
}

/* ************************************************** */
/* * ACCESS OPERATIONS FOR MCU ********************** */
/* ************************************************** */

int8_t msp430_digiIO_mcu_read (uint16_t addr)
{
  uint8_t r = 0;
  switch (addr)
    {
    case P1IN:  r =  DIGIIO_IN(0); break;
    case P1OUT: r = DIGIIO_OUT(0); break;
    case P1DIR: r = DIGIIO_DIR(0); break;
    case P1IFG: r = DIGIIO_IFG(0); break;
    case P1IES: r = DIGIIO_IES(0); break;
    case P1IE : r = DIGIIO_IEN(0); break;
    case P1SEL: r = DIGIIO_SEL(0); break;


    case P2IN:  r =  DIGIIO_IN(1); break;
    case P2OUT: r = DIGIIO_OUT(1); break;
    case P2DIR: r = DIGIIO_DIR(1); break;
    case P2IFG: r = DIGIIO_IFG(1); break;
    case P2IES: r = DIGIIO_IES(1); break;
    case P2IE : r = DIGIIO_IEN(1); break;
    case P2SEL: r = DIGIIO_SEL(1); break;


#if defined(__msp430_have_port3)
    case P3IN : r =  DIGIIO_IN(2); break;
    case P3OUT: r = DIGIIO_OUT(2); break;
    case P3DIR: r = DIGIIO_DIR(2); break;
    case P3SEL: r = DIGIIO_SEL(2); break;
#endif

#if defined(__msp430_have_port4)
    case P4IN:  r =  DIGIIO_IN(3); break;
    case P4OUT: r = DIGIIO_OUT(3); break;
    case P4DIR: r = DIGIIO_DIR(3); break;
    case P4SEL: r = DIGIIO_SEL(3); break;
#endif

#if defined(__msp430_have_port5)
    case P5IN:  r =  DIGIIO_IN(4); break;
    case P5OUT: r = DIGIIO_OUT(4); break;
    case P5DIR: r = DIGIIO_DIR(4); break;
    case P5SEL: r = DIGIIO_SEL(4); break;
#endif

#if defined(__msp430_have_port6)
    case P6IN:  r =  DIGIIO_IN(5); break;
    case P6OUT: r = DIGIIO_OUT(5); break;
    case P6DIR: r = DIGIIO_DIR(5); break;
    case P6SEL: r = DIGIIO_SEL(5); break;
#endif
    default:
      ERROR("msp430:dio: read  [0x%02x] undefined\n",addr); 
      break;
    }
  HW_DMSG_DIGI_IO("msp430:dio: read  from MCU [%s:0x%02x] = 0x%02x\n",
		  msp430_debug_portname(addr),addr,r);
  return r;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_mcu_write(uint16_t addr, int8_t val)
{
  uint8_t oldval;

  HW_DMSG_DIGI_IO("msp430:dio: write from MCU [%s:0x%02x] = 0x%02x\n",msp430_debug_portname(addr),addr,val & 0xff); 

  switch (addr)
    {
      /* port 1 */
    case P1IN:  ERROR("msp430:dio: write on P1IN (read only)\n"); break;
    case P1OUT: 
      oldval = DIGIIO_OUT(0);
      DIGIIO_OUT(0) = val & DIGIIO_MCU_WOK(0); 
      MCU.digiIO.out_updated[0] = oldval ^ DIGIIO_OUT(0); 
      TRACER_TRACE_PORT1(DIGIIO_OUT(0));
      break; 
    case P1DIR: DIGIIO_DIR(0) = val; break;
    case P1IFG: 
      if (((val & ~DIGIIO_IFG(0)) != 0) && (DIGIIO_IEN(0) & val)) 
	{ /* software generated interrupt */
	  HW_DMSG_DIGI_IO("msp430:dio: software generated interrupt on port 1\n");
	  msp430_interrupt_set(INTR_IOPORT1);
	}
      DIGIIO_IFG(0) = val; 
      break;
    case P1IES: DIGIIO_IES(0) = val; break;
    case P1IE : DIGIIO_IEN(0) = val; break;
    case P1SEL: DIGIIO_SEL(0) = val; break; 

      /* port 2 */
    case P2IN:  ERROR("msp430:dio: write on P2IN (read only)\n"); break;
    case P2OUT:
      oldval = DIGIIO_OUT(1); 
      DIGIIO_OUT(1) = val & DIGIIO_MCU_WOK(1); 
      MCU.digiIO.out_updated[1] = oldval ^ DIGIIO_OUT(1); 
      TRACER_TRACE_PORT2(DIGIIO_OUT(1));
      break; 
    case P2DIR: DIGIIO_DIR(1) = val; break; 
    case P2IFG: 
      if (((val & ~DIGIIO_IFG(1)) != 0) && (DIGIIO_IEN(1) & val)) 
	{ /* software generated interrupt */
	  HW_DMSG_DIGI_IO("msp430:dio: software generated interrupt on port 2\n");
	  msp430_interrupt_set(INTR_IOPORT2);
	}
      DIGIIO_IFG(1) = val; 
      break;
    case P2IES: DIGIIO_IES(1) = val; break;
    case P2IE : DIGIIO_IEN(1) = val; break;
    case P2SEL: DIGIIO_SEL(1) = val; break; 

      /* port 3 */
#if defined(__msp430_have_port3)
    case P3IN : ERROR("msp430:dio: write on P3IN (read only)\n"); break;
    case P3OUT:
      oldval = DIGIIO_OUT(2); 
      DIGIIO_OUT(2) = val & DIGIIO_MCU_WOK(2);  
      MCU.digiIO.out_updated[2] = oldval ^ DIGIIO_OUT(2); 
      TRACER_TRACE_PORT3(DIGIIO_OUT(2));
      break; 
    case P3DIR: DIGIIO_DIR(2) = val; break;
    case P3SEL: DIGIIO_SEL(2) = val; break; 
#endif

      /* port 4 */
#if defined(__msp430_have_port4)
    case P4IN : ERROR("msp430:dio: write on P4IN (read only)\n"); break;
    case P4OUT: 
      oldval = DIGIIO_OUT(3); 
      DIGIIO_OUT(3) = val & DIGIIO_MCU_WOK(3);  
      MCU.digiIO.out_updated[3] = oldval ^ DIGIIO_OUT(3); 
      TRACER_TRACE_PORT4(DIGIIO_OUT(3));
      break;
    case P4DIR: DIGIIO_DIR(3) = val; break;
    case P4SEL: DIGIIO_SEL(3) = val; break; 
#endif

      /* port 5 */
#if defined(__msp430_have_port5)
    case P5IN : ERROR("msp430:dio: write on P5IN (read only)\n"); break;
    case P5OUT: 
      oldval = DIGIIO_OUT(4); 
      DIGIIO_OUT(4) = val & DIGIIO_MCU_WOK(4);  
      MCU.digiIO.out_updated[4] = oldval ^ DIGIIO_OUT(4); 
      TRACER_TRACE_PORT5(DIGIIO_OUT(4));
      break;
    case P5DIR: DIGIIO_DIR(4) = val; break;
    case P5SEL: DIGIIO_SEL(4) = val; break; 
#endif

      /* port 6 */
#if defined(__msp430_have_port6)
    case P6IN : ERROR("msp430:dio: write on P6IN (read only)\n"); break;
    case P6OUT: 
      oldval = DIGIIO_OUT(5); 
      DIGIIO_OUT(5) = val & DIGIIO_MCU_WOK(5);  
      MCU.digiIO.out_updated[5] = oldval ^ DIGIIO_OUT(5); 
      TRACER_TRACE_PORT6(DIGIIO_OUT(5));
      break;
    case P6DIR: DIGIIO_DIR(5) = val; break;
    case P6SEL: DIGIIO_SEL(5) = val; break; 
#endif

    default:
      ERROR("msp430:dio: write [0x%02x] undefined\n",addr); 
      break;
    }
}

/* ************************************************** */
/* * ACCESS OPERATIONS FOR EXTERNAL DEVICES ********* */
/* ************************************************** */

int msp430_digiIO_dev_read (int port_number, uint8_t *val)
{
  //  HW_DMSG_DIGI_IO("   Digital IO read from devices on port %d\n",port_number);
  *val = DIGIIO_OUT(port_number);
  return MCU.digiIO.out_updated[port_number]; // port has been updated ?
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_dev_write(int port_number, uint8_t val, uint8_t bitmask)
{
  uint8_t oldval;

  oldval = DIGIIO_IN(port_number);
  DIGIIO_IN(port_number)    = (oldval & ~bitmask) | ((val & bitmask) & ~DIGIIO_DIR(port_number));

  HW_DMSG_DIGI_IO("msp430:dio: write from devices on port %d val 0x%02x -> 0x%02x\n",port_number + 1,oldval & 0xff,DIGIIO_IN(port_number) & 0xff);

  MCU.digiIO.in_updated[port_number] = oldval ^ DIGIIO_IN(port_number);
  if (MCU.digiIO.in_updated[3] & 0x02) {
    HW_DMSG_DIGI_IO("msp430:dio: SFD updated (telosb)\n");
  }
                                                                            
#if defined(IFG_BIT_IS_SET_ONLY_WITH_IE_BIT_IS_ALSO_SET)
  if ((port_number < 2) &&  ((DIGIIO_IEN(port_number) & bitmask) != 0))
#else /* this seems to be the default behavior */
  if ((port_number < 2))
#endif
    {
      /* high to low : oldval=1 val=0 bitmask=1 DIGIIO_IES=1 */
      /* low to high : oldval=0 val=1 bitmask=1 DIGIIO_IES=0 */
      /*     ies = 1 high to low   //    ies = 0 low to high */
      uint8_t ifg = (~val &  oldval & bitmask &  DIGIIO_IES(port_number)) | 
	            ( val & ~oldval & bitmask & ~DIGIIO_IES(port_number));;

      HW_DMSG_DIGI_IO("        new igf flag 0x%02x\n",ifg);
      DIGIIO_IFG(port_number) |= ifg;
      
      if ((DIGIIO_IEN(port_number) & ifg & bitmask) != 0)
	{
	  HW_DMSG_DIGI_IO("        interrupt on port %d to be scheduled\n",port_number + 1);
	  msp430_interrupt_set((port_number == 0) ? INTR_IOPORT1: INTR_IOPORT2);
	}
    }
}

/* ************************************************** */
/* * ACCESS OPERATIONS FOR INTERNAL DEVICES ********* */
/* ************************************************** */
/*
int msp430_digiIO_internal_dev_read (int port_number, uint8_t *val)
{
  HW_DMSG_DIGI_IO("msp430:dio: read from internal devices on port %d\n",port_number);
  *val = DIGIIO_IN(port_number);
  return MCU.digiIO.in_updated & (1 << port_number); // port has been updated ?
}
*/
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
/*
void msp430_digiIO_internal_dev_write(int port_number, uint8_t val, uint8_t bitmask)
{
  HW_DMSG_DIGI_IO("msp430:dio: write from devices on port %d val 0x%02x\n",port_number,val & 0xff);
  DIGIIO_OUT(port_number)       = (val & DIGIIO_DIR(port_number) & bitmask) | (DIGIIO_OUT(port_number) & ~bitmask);
  MCU.digiIO.out_updated |= (1 << port_number);
}
*/
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_update_done(void)
{ 
  int i;
  for(i=0; i<6; i++)
    {
      MCU.digiIO.in_updated[i]  = 0;
      MCU.digiIO.out_updated[i] = 0;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_digiIO_chkifg(void)
{
  int res = 0;
  /* digi IO ports 1&2 */
  if (DIGIIO_IFG(0) & DIGIIO_IEN(0))
    {
      msp430_interrupt_set(INTR_IOPORT1); 
      res |= 1;
    }
  if (DIGIIO_IFG(1) & DIGIIO_IEN(1))
    {
      msp430_interrupt_set(INTR_IOPORT2); 
      res |= 1;
    }
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

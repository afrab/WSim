
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


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if !defined(DIGIIO_BASE)
#define DIGIIO_BASE  0x0010
#endif

#if !defined(DIGIIO_NEW_OFFSETS)
#define DIGIIO_START DIGIIO_BASE
#define DIGIIO_END   (DIGIIO_BASE + 0x2f)

#define P1IN      (DIGIIO_BASE + 0x10)
#define P1OUT     (DIGIIO_BASE + 0x11)
#define P1DIR     (DIGIIO_BASE + 0x12)
#define P1IFG     (DIGIIO_BASE + 0x13)
#define P1IES     (DIGIIO_BASE + 0x14)
#define P1IE      (DIGIIO_BASE + 0x15)
#define P1SEL     (DIGIIO_BASE + 0x16)
#define P1SEL2    (DIGIIO_BASE + 0x31)
#define P1REN     (DIGIIO_BASE + 0x17)

#define P2IN      (DIGIIO_BASE + 0x18)
#define P2OUT     (DIGIIO_BASE + 0x19)
#define P2DIR     (DIGIIO_BASE + 0x1a)
#define P2IFG     (DIGIIO_BASE + 0x1b)
#define P2IES     (DIGIIO_BASE + 0x1c)
#define P2IE      (DIGIIO_BASE + 0x1d)
#define P2SEL     (DIGIIO_BASE + 0x1e)
#define P2SEL2    (DIGIIO_BASE + 0x32)
#define P2REN     (DIGIIO_BASE + 0x1f)

#define P3IN      (DIGIIO_BASE + 0x08)
#define P3OUT     (DIGIIO_BASE + 0x09)
#define P3DIR     (DIGIIO_BASE + 0x0a)
#define P3SEL     (DIGIIO_BASE + 0x0b)
#define P3SEL2    (DIGIIO_BASE + 0x33)
#define P3REN     (DIGIIO_BASE + 0x00)

#define P4IN      (DIGIIO_BASE + 0x0c)
#define P4OUT     (DIGIIO_BASE + 0x0d)
#define P4DIR     (DIGIIO_BASE + 0x0e)
#define P4SEL     (DIGIIO_BASE + 0x0f)
#define P4SEL2    (DIGIIO_BASE + 0x34)
#define P4REN     (DIGIIO_BASE + 0x01)

#define P5IN      (DIGIIO_BASE + 0x20)
#define P5OUT     (DIGIIO_BASE + 0x21)
#define P5DIR     (DIGIIO_BASE + 0x22)
#define P5SEL     (DIGIIO_BASE + 0x23)
#define P5SEL2    (DIGIIO_BASE + 0x35)
#define P5REN     (DIGIIO_BASE + 0x02)

#define P6IN      (DIGIIO_BASE + 0x24)
#define P6OUT     (DIGIIO_BASE + 0x25)
#define P6DIR     (DIGIIO_BASE + 0x26)
#define P6SEL     (DIGIIO_BASE + 0x27)
#define P6SEL2    (DIGIIO_BASE + 0x36)
#define P6REN     (DIGIIO_BASE + 0x03)

#define P7IN      (DIGIIO_BASE + 0x28)
#define P7OUT     (DIGIIO_BASE + 0x2a)
#define P7DIR     (DIGIIO_BASE + 0x2c)
#define P7SEL     (DIGIIO_BASE + 0x2e)
#define P7SEL2    (DIGIIO_BASE + 0x37)
#define P7REN     (DIGIIO_BASE + 0x04)

#define P8IN      (DIGIIO_BASE + 0x29)
#define P8OUT     (DIGIIO_BASE + 0x2b)
#define P8DIR     (DIGIIO_BASE + 0x2d)
#define P8SEL     (DIGIIO_BASE + 0x2f)
#define P8SEL2    (DIGIIO_BASE + 0x38)
#define P8REN     (DIGIIO_BASE + 0x05)

#else

#define DIGIIO_START DIGIIO_BASE
#define DIGIIO_END   (DIGIIO_BASE + 0x12f)

#define P1IN      (DIGIIO_BASE + 0x000)
#define P1OUT     (DIGIIO_BASE + 0x002)
#define P1DIR     (DIGIIO_BASE + 0x004)
#define P1IFG     (DIGIIO_BASE + 0x01c)
#define P1IES     (DIGIIO_BASE + 0x018)
#define P1IE      (DIGIIO_BASE + 0x01a)
#define P1SEL     (DIGIIO_BASE + 0x00a)
#define P1SEL2    0xffff
#define P1REN     (DIGIIO_BASE + 0x006)

#define P2IN      (DIGIIO_BASE + 0x001)
#define P2OUT     (DIGIIO_BASE + 0x003)
#define P2DIR     (DIGIIO_BASE + 0x005)
#define P2IFG     (DIGIIO_BASE + 0x01d)
#define P2IES     (DIGIIO_BASE + 0x019)
#define P2IE      (DIGIIO_BASE + 0x01b)
#define P2SEL     (DIGIIO_BASE + 0x00b)
#define P2SEL2    0xffff
#define P2REN     (DIGIIO_BASE + 0x007)

#define P3IN      (DIGIIO_BASE + 0x020)
#define P3OUT     (DIGIIO_BASE + 0x022)
#define P3DIR     (DIGIIO_BASE + 0x024)
#define P3SEL     (DIGIIO_BASE + 0x02a)
#define P3SEL2    0xffff
#define P3REN     (DIGIIO_BASE + 0x026)

#define P4IN      (DIGIIO_BASE + 0x021)
#define P4OUT     (DIGIIO_BASE + 0x023)
#define P4DIR     (DIGIIO_BASE + 0x025)
#define P4SEL     (DIGIIO_BASE + 0x02b)
#define P4SEL2    0xffff
#define P4REN     (DIGIIO_BASE + 0x027)

#define P5IN      (DIGIIO_BASE + 0x040)
#define P5OUT     (DIGIIO_BASE + 0x042)
#define P5DIR     (DIGIIO_BASE + 0x044)
#define P5SEL     (DIGIIO_BASE + 0x04a)
#define P5SEL2    0xffff
#define P5REN     (DIGIIO_BASE + 0x046)

#define P6IN      (DIGIIO_BASE + 0x041)
#define P6OUT     (DIGIIO_BASE + 0x043)
#define P6DIR     (DIGIIO_BASE + 0x045)
#define P6SEL     (DIGIIO_BASE + 0x04b)
#define P6SEL2    0xffff
#define P6REN     (DIGIIO_BASE + 0x047)

#define PJIN      (DIGIIO_BASE + 0x120)
#define PJOUT     (DIGIIO_BASE + 0x122)
#define PJDIR     (DIGIIO_BASE + 0x124)
#define PJREN     (DIGIIO_BASE + 0x126)
#endif



#if defined(__msp430_have_port1)
#define IFPORT1(insn) do { insn } while (0)
#else
#define IFPORT1(insn) do { } while (0)
#endif

#if defined(__msp430_have_port2)
#define IFPORT2(insn) do { insn } while (0)
#else
#define IFPORT2(insn) do { } while (0)
#endif

#if defined(__msp430_have_port3)
#define IFPORT3(insn) do { insn } while (0)
#else
#define IFPORT3(insn) do { } while (0)
#endif

#if defined(__msp430_have_port4)
#define IFPORT4(insn) do { insn } while (0)
#else
#define IFPORT4(insn) do { } while (0)
#endif

#if defined(__msp430_have_port5)
#define IFPORT5(insn) do { insn } while (0)
#else
#define IFPORT5(insn) do { } while (0)
#endif

#if defined(__msp430_have_port6)
#define IFPORT6(insn) do { insn } while (0)
#else
#define IFPORT6(insn) do { } while (0)
#endif

#if defined(__msp430_have_port7)
#define IFPORT7(insn) do { insn } while (0)
#else
#define IFPORT7(insn) do { } while (0)
#endif

#if defined(__msp430_have_port8)
#define IFPORT8(insn) do { insn } while (0)
#else
#define IFPORT8(insn) do { } while (0)
#endif

#if defined(__msp430_have_portj)
#define IFPORTJ(insn) do { insn } while (0)
#else
#define IFPORTJ(insn) do { } while (0)
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_id_t MSP430_TRACER_PORT1;
tracer_id_t MSP430_TRACER_PORT2;
tracer_id_t MSP430_TRACER_PORT3;
tracer_id_t MSP430_TRACER_PORT4;
tracer_id_t MSP430_TRACER_PORT5;
tracer_id_t MSP430_TRACER_PORT6;
tracer_id_t MSP430_TRACER_PORT7;
tracer_id_t MSP430_TRACER_PORT8;
tracer_id_t MSP430_TRACER_PORTJ;

#define TRACER_TRACE_PORT1(v)   tracer_event_record(MSP430_TRACER_PORT1,v)
#define TRACER_TRACE_PORT2(v)   tracer_event_record(MSP430_TRACER_PORT2,v)
#define TRACER_TRACE_PORT3(v)   tracer_event_record(MSP430_TRACER_PORT3,v)
#define TRACER_TRACE_PORT4(v)   tracer_event_record(MSP430_TRACER_PORT4,v)
#define TRACER_TRACE_PORT5(v)   tracer_event_record(MSP430_TRACER_PORT5,v)
#define TRACER_TRACE_PORT6(v)   tracer_event_record(MSP430_TRACER_PORT6,v)
#define TRACER_TRACE_PORT7(v)   tracer_event_record(MSP430_TRACER_PORT7,v)
#define TRACER_TRACE_PORT8(v)   tracer_event_record(MSP430_TRACER_PORT8,v)
#define TRACER_TRACE_PORTJ(v)   tracer_event_record(MSP430_TRACER_PORTJ,v)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* msp430_digiIO_portname(uint16_t addr)
{
  switch (addr)
    {
    case P1IN  : return "P1IN"; 
    case P1OUT : return "P1OUT"; 
    case P1DIR : return "P1DIR"; 
    case P1IFG : return "P1IFG"; 
    case P1IES : return "P1IES"; 
    case P1IE  : return "P1IEN"; 
    case P1SEL : return "P1SEL"; 
    case P1SEL2: return "P1SEL2";
    case P1REN : return "P1REN";

    case P2IN  : return "P2IN"; 
    case P2OUT : return "P2OUT"; 
    case P2DIR : return "P2DIR"; 
    case P2IFG : return "P2IFG"; 
    case P2IES : return "P2IES"; 
    case P2IE  : return "P2IEN"; 
    case P2SEL : return "P2SEL"; 
    case P2SEL2: return "P2SEL2";
    case P2REN : return "P2REN";

#if defined(__msp430_have_port3)
    case P3IN  : return "P3IN"; 
    case P3OUT : return "P3OUT"; 
    case P3DIR : return "P3DIR"; 
    case P3SEL : return "P3SEL"; 
    case P3SEL2: return "P3SEL2";
    case P3REN : return "P3REN";
#endif

#if defined(__msp430_have_port4)
    case P4IN  : return "P4IN"; 
    case P4OUT : return "P4OUT"; 
    case P4DIR : return "P4DIR"; 
    case P4SEL : return "P4SEL"; 
    case P4SEL2: return "P4SEL2";
    case P4REN : return "P4REN";
#endif

#if defined(__msp430_have_port5)
    case P5IN  : return "P5IN"; 
    case P5OUT : return "P5OUT"; 
    case P5DIR : return "P5DIR"; 
    case P5SEL : return "P5SEL"; 
    case P5SEL2: return "P5SEL2";
    case P5REN : return "P5REN";
#endif

#if defined(__msp430_have_port6)
    case P6IN  : return "P6IN"; 
    case P6OUT : return "P6OUT"; 
    case P6DIR : return "P6DIR"; 
    case P6SEL : return "P6SEL"; 
    case P6SEL2: return "P6SEL2";
    case P6REN : return "P6REN";
#endif

#if defined(__msp430_have_port7)
    case P7IN  : return "P7IN"; 
    case P7OUT : return "P7OUT"; 
    case P7DIR : return "P7DIR"; 
    case P7SEL : return "P7SEL"; 
    case P7SEL2: return "P7SEL2";
    case P7REN : return "P7REN";
#endif

#if defined(__msp430_have_port8)
    case P8IN  : return "P8IN"; 
    case P8OUT : return "P8OUT"; 
    case P8DIR : return "P8DIR"; 
    case P8SEL : return "P8SEL"; 
    case P8SEL2: return "P8SEL2";
    case P8REN : return "P8REN";
#endif
    
#if defined(__msp430_have_portj)
    case PJIN  : return "PJIN";
    case PJOUT : return "PJOUT";
    case PJDIR : return "PJDIR";
    case PJREN : return "PJREN";
#endif

    default: return "XXX";
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/** 
 * interrupt enable for port 1 and 2 
 * not taken into account if PxSEL(port,bit) != 0
 */
#define DIGIIO_IEN(p)    MCU.digiIO.int_enable[p]

/** 
 * edge select
 * 0: IFG is set on low to high transition 
 * 1: IFG is set on high to low transition
 */
#define DIGIIO_IES(p)    MCU.digiIO.int_edge_select[p]

/** 
 * interrupt flag
 * 0: no interrupt is pending
 * 1: an interrupt is pending
 */
#define DIGIIO_IFG(p)    MCU.digiIO.ifg[p]

/** 
 * input register 
 */
#define DIGIIO_IN(p)     MCU.digiIO.in[p]

/** 
 * output register 
 */
#define DIGIIO_OUT(p)    MCU.digiIO.out[p]

/** 
 * direction register 
 * PxDIR : 0 = input direction, 1 = output direction
 */
#define DIGIIO_DIR(p)    MCU.digiIO.direction[p]

/** 
 * selection : IO or device 
 * PxSEL : 0 = I/O function, 1 = Peripheral module
 */
#define DIGIIO_SEL(p)    MCU.digiIO.selection[p]

/** 
 * selection : IO or device 
 * SEL / SEL2
 *  0     0    I/O function is selected.
 *  0     1    Primary peripheral module function is selected.
 *  1     0    Reserved. See device-specific data sheet.
 *  1     1    Secondary peripheral module function is selected.
 */
#define DIGIIO_SEL2(p)   MCU.digiIO.selection2[p]

/** 
 * resistor enable
 * Bit = 0: Pullup/pulldown resistor disabled
 * Bit = 1: Pullup/pulldown resistor enabled
 */
#define DIGIIO_REN(p)    MCU.digiIO.resistor[p]

/** 
 * MCU can write on output port even if this port is selected IN 
 * see Figure page 49 of [msp430f1611.pdf]
 * #define _WOK(p)    _DIR(p) & ~_SEL(p)
 */
#define DIGIIO_MCU_WOK(p)    ~DIGIIO_SEL(p)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
  
void msp430_digiIO_create()
{
  msp430_io_register_range8(DIGIIO_START,DIGIIO_END,msp430_digiIO_mcu_read,msp430_digiIO_mcu_write);

  IFPORT1 ( MSP430_TRACER_PORT1    = tracer_event_add_id(8,  "port1_out",  "msp430"); );
  IFPORT2 ( MSP430_TRACER_PORT2    = tracer_event_add_id(8,  "port2_out",  "msp430"); );
  IFPORT3 ( MSP430_TRACER_PORT3    = tracer_event_add_id(8,  "port3_out",  "msp430"); );
  IFPORT4 ( MSP430_TRACER_PORT4    = tracer_event_add_id(8,  "port4_out",  "msp430"); );
  IFPORT5 ( MSP430_TRACER_PORT5    = tracer_event_add_id(8,  "port5_out",  "msp430"); );
  IFPORT6 ( MSP430_TRACER_PORT6    = tracer_event_add_id(8,  "port6_out",  "msp430"); );
  IFPORT7 ( MSP430_TRACER_PORT7    = tracer_event_add_id(8,  "port7_out",  "msp430"); );
  IFPORT8 ( MSP430_TRACER_PORT8    = tracer_event_add_id(8,  "port8_out",  "msp430"); );
  IFPORTJ ( MSP430_TRACER_PORTJ    = tracer_event_add_id(8,  "portj_out",  "msp430"); );
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_reset()
{
  /* after a reset the pin IO are switched to input mode */
  int i;
  for(i=0; i<9; i++)
    {
      DIGIIO_DIR (i) = 0;
      DIGIIO_SEL (i) = 0;
      DIGIIO_SEL2(i) = 0;
      DIGIIO_REN (i) = 0;
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
    case P1IN  : r = DIGIIO_IN  (0); break;
    case P1OUT : r = DIGIIO_OUT (0); break;
    case P1DIR : r = DIGIIO_DIR (0); break;
    case P1IFG : r = DIGIIO_IFG (0); break;
    case P1IES : r = DIGIIO_IES (0); break;
    case P1IE  : r = DIGIIO_IEN (0); break;
    case P1SEL : r = DIGIIO_SEL (0); break;
    case P1SEL2: r = DIGIIO_SEL2(0); break;
    case P1REN : r = DIGIIO_REN (0); break;


    case P2IN  : r = DIGIIO_IN  (1); break;
    case P2OUT : r = DIGIIO_OUT (1); break;
    case P2DIR : r = DIGIIO_DIR (1); break;
    case P2IFG : r = DIGIIO_IFG (1); break;
    case P2IES : r = DIGIIO_IES (1); break;
    case P2IE  : r = DIGIIO_IEN (1); break;
    case P2SEL : r = DIGIIO_SEL (1); break;
    case P2SEL2: r = DIGIIO_SEL2(1); break;
    case P2REN : r = DIGIIO_REN (1); break;


#if defined(__msp430_have_port3)
    case P3IN  : r = DIGIIO_IN  (2); break;
    case P3OUT : r = DIGIIO_OUT (2); break;
    case P3DIR : r = DIGIIO_DIR (2); break;
    case P3SEL : r = DIGIIO_SEL (2); break;
    case P3SEL2: r = DIGIIO_SEL2(2); break;
    case P3REN : r = DIGIIO_REN (2); break;
#endif

#if defined(__msp430_have_port4)
    case P4IN  : r = DIGIIO_IN  (3); break;
    case P4OUT : r = DIGIIO_OUT (3); break;
    case P4DIR : r = DIGIIO_DIR (3); break;
    case P4SEL : r = DIGIIO_SEL (3); break;
    case P4SEL2: r = DIGIIO_SEL2(3); break;
    case P4REN : r = DIGIIO_REN (3); break;
#endif

#if defined(__msp430_have_port5)
    case P5IN  : r = DIGIIO_IN  (4); break;
    case P5OUT : r = DIGIIO_OUT (4); break;
    case P5DIR : r = DIGIIO_DIR (4); break;
    case P5SEL : r = DIGIIO_SEL (4); break;
    case P5SEL2: r = DIGIIO_SEL2(4); break;
    case P5REN : r = DIGIIO_REN (4); break;
#endif

#if defined(__msp430_have_port6)
    case P6IN  : r = DIGIIO_IN  (5); break;
    case P6OUT : r = DIGIIO_OUT (5); break;
    case P6DIR : r = DIGIIO_DIR (5); break;
    case P6SEL : r = DIGIIO_SEL (5); break;
    case P6SEL2: r = DIGIIO_SEL2(5); break;
    case P6REN : r = DIGIIO_REN (5); break;
#endif

#if defined(__msp430_have_port7)
    case P7IN  : r = DIGIIO_IN  (6); break;
    case P7OUT : r = DIGIIO_OUT (6); break;
    case P7DIR : r = DIGIIO_DIR (6); break;
    case P7SEL : r = DIGIIO_SEL (6); break;
    case P7SEL2: r = DIGIIO_SEL2(6); break;
    case P7REN : r = DIGIIO_REN (6); break;
#endif

#if defined(__msp430_have_port8)
    case P8IN  : r = DIGIIO_IN  (7); break;
    case P8OUT : r = DIGIIO_OUT (7); break;
    case P8DIR : r = DIGIIO_DIR (7); break;
    case P8SEL : r = DIGIIO_SEL (7); break;
    case P8SEL2: r = DIGIIO_SEL2(7); break;
    case P8REN : r = DIGIIO_REN (7); break;
#endif

#if defined(__msp430_have_portj)
    case P8IN  : r = DIGIIO_IN  (8); break;
    case P8OUT : r = DIGIIO_OUT (8); break;
    case P8DIR : r = DIGIIO_DIR (8); break;
    case P8REN : r = DIGIIO_REN (8); break;
#endif

    default:
      ERROR("msp430:dio: read [0x%02x] undefined\n",addr); 
      break;
    }
  HW_DMSG_DIGI_IO("msp430:dio: read  from MCU [%s:0x%02x] = 0x%02x\n",
		  msp430_digiIO_portname(addr),addr,r);
  return r;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_mcu_write(uint16_t addr, int8_t val)
{
  uint8_t oldval;

  HW_DMSG_DIGI_IO("msp430:dio: write from MCU [%s:0x%02x] = 0x%02x\n",
                  msp430_digiIO_portname(addr),addr,val & 0xff); 

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
    case P1SEL2:DIGIIO_SEL2(0)= val; break; 
    case P1REN: DIGIIO_REN(0) = val; break; 

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
    case P2SEL2:DIGIIO_SEL2(1)= val; break; 
    case P2REN: DIGIIO_REN(1) = val; break; 

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
    case P3SEL2:DIGIIO_SEL2(2)= val; break; 
    case P3REN: DIGIIO_REN(2) = val; break; 
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
    case P4SEL2:DIGIIO_SEL2(3)= val; break; 
    case P4REN: DIGIIO_REN(3) = val; break; 
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
    case P5SEL2:DIGIIO_SEL2(4)= val; break; 
    case P5REN: DIGIIO_REN(4) = val; break; 
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
    case P6SEL2:DIGIIO_SEL2(5)= val; break; 
    case P6REN: DIGIIO_REN(5) = val; break; 
#endif

      /* port 7 */
#if defined(__msp430_have_port7)
    case P7IN : ERROR("msp430:dio: write on P7IN (read only)\n"); break;
    case P7OUT: 
      oldval = DIGIIO_OUT(6); 
      DIGIIO_OUT(6) = val & DIGIIO_MCU_WOK(6);  
      MCU.digiIO.out_updated[6] = oldval ^ DIGIIO_OUT(6); 
      TRACER_TRACE_PORT7(DIGIIO_OUT(6));
      break;
    case P7DIR: DIGIIO_DIR(6) = val; break;
    case P7SEL: DIGIIO_SEL(6) = val; break; 
    case P7SEL2:DIGIIO_SEL2(6)= val; break; 
    case P7REN: DIGIIO_REN(6) = val; break; 
#endif

      /* port 8 */
#if defined(__msp430_have_port8)
    case P8IN : ERROR("msp430:dio: write on P8IN (read only)\n"); break;
    case P8OUT: 
      oldval = DIGIIO_OUT(7); 
      DIGIIO_OUT(7) = val & DIGIIO_MCU_WOK(7);  
      MCU.digiIO.out_updated[7] = oldval ^ DIGIIO_OUT(7); 
      TRACER_TRACE_PORT8(DIGIIO_OUT(7));
      break;
    case P8DIR: DIGIIO_DIR(7) = val; break;
    case P8SEL: DIGIIO_SEL(7) = val; break; 
    case P8SEL2:DIGIIO_SEL2(7)= val; break; 
    case P8REN: DIGIIO_REN(7) = val; break; 
#endif

      /* port J */
#if defined(__msp430_have_portj)
    case PJIN : ERROR("msp430:dio: write on PJIN (read only)\n"); break;
    case PJOUT:
      oldval = DIGIIO_OUT(8);
      DIGIIO_OUT(8) = val & DIGIIO_MCU_WOK(8);
      MCU.digiIO.out_updated[8] = oldval ^ DIGIIO_OUT(8);
      TRACER_TRACE_PORT8(DIGIIO_OUT(8));
      break;
    case PJDIR: DIGIIO_DIR(8) = val; break;
    case PJREN: DIGIIO_REN(8) = val; break;
#endif

    default:
      ERROR("msp430:dio: write [0x%02x] undefined\n",addr); 
      break;
    }
}

/* ************************************************** */
/* * ACCESS OPERATIONS FOR EXTERNAL DEVICES ********* */
/* ************************************************** */

uint8_t msp430_digiIO_dev_read_dir (int port_number)
{
  return DIGIIO_DIR(port_number);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_digiIO_dev_read (int port_number, uint8_t *val)
{
  *val = DIGIIO_OUT(port_number);
  /* port has been updated ? */
  return MCU.digiIO.out_updated[port_number]; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_dev_write(int port_number, uint8_t val, uint16_t bitmask)
{
  uint8_t oldval;

  /* update value */
#if defined(__msp430_have_ren)
  /*
   * Each bit in each PxREN register enables or disables the pullup/pulldown
   * resistor of the corresponding I/O pin. The corresponding bit in the PxOUT
   * register selects if the pin is pulled up or pulled down.
   * Bit = 0: Pullup/pulldown resistor disabled
   * Bit = 1: Pullup/pulldown resistor enabled
   *
   * Pullup/pulldown resistor
   * PxOUT == 0 For pullup:   VIN = VSS;  # digital ground reference
   * PxOUT == 1 For pulldown: VIN = VCC;
   *
   */
  if (bitmask & 0xff00) // Resistor enable : 16 bit bitmask [ren mask][bitmask]
  {
    
    oldval                 = DIGIIO_IN(port_number);
    // keep bits that are not used in bitmask and REN bitmask
    uint8_t keep    = (oldval & ~bitmask & ~(bitmask >> 8));
    // set bits imposed by bitmask only
    uint8_t forced  = (val    &  bitmask & ~(bitmask >> 8)) & ~DIGIIO_DIR(port_number);
    // set bits according to REN register
    uint8_t renval  = ((bitmask >> 8) & DIGIIO_REN(port_number)) & DIGIIO_OUT(port_number) & ~DIGIIO_DIR(port_number);

    DIGIIO_IN(port_number) = keep | forced | renval;
    MCU.digiIO.in_updated[port_number] = oldval ^ DIGIIO_IN(port_number);
    HW_DMSG_DIGI_IO("msp430:dio: pullup/pulldown resistor enable old=0x%02x val=0x%02x \n",
                    oldval, MCU.digiIO.in_updated[port_number]);
  }
  else // normal behaviour
#endif  
  {
    oldval                 = DIGIIO_IN(port_number);
    DIGIIO_IN(port_number) = (oldval & ~bitmask) | ((val & bitmask) & ~DIGIIO_DIR(port_number));
    MCU.digiIO.in_updated[port_number] = oldval ^ DIGIIO_IN(port_number);
  }


  HW_DMSG_DIGI_IO("msp430:dio: write from devices on port %d val 0x%02x -> 0x%02x\n",
                  port_number + 1, oldval & 0xff, DIGIIO_IN(port_number) & 0xff);

  /* interrupt on ports 1 & 2 */
  if (port_number < 2)
    {
      /*
       * ies = 1 high to low 
       * ies = 0 low to high
       *        
       * high to low : oldval=1 val=0 bitmask=1 DIGIIO_IES=1
       * low to high : oldval=0 val=1 bitmask=1 DIGIIO_IES=0
       */
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
/* ************************************************** */
/* ************************************************** */

void msp430_digiIO_update_done(void)
{ 
  int i;
  for(i=0; i<9; i++)
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


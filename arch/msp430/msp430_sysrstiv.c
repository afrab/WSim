/**
 *  \file   msp430_sysrstiv.h
 *  \brief  MSP430 Reset reason
 *  \author David Graeff
 *  \date   2013
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

 /* Reset vector reason */
#define SYSRSTIV_NONE      (0x0000)    /* No Interrupt pending */
#define SYSRSTIV_BOR       (0x0002)    /* SYSRSTIV : BOR */
#define SYSRSTIV_RSTNMI    (0x0004)    /* SYSRSTIV : RST/NMI */
#define SYSRSTIV_DOBOR     (0x0006)    /* SYSRSTIV : Do BOR */
#define SYSRSTIV_LPM5WU    (0x0008)    /* SYSRSTIV : Port LPM5 Wake Up */
#define SYSRSTIV_SECYV     (0x000A)    /* SYSRSTIV : Security violation */
#define SYSRSTIV_SVSL      (0x000C)    /* SYSRSTIV : SVSL */
#define SYSRSTIV_SVSH      (0x000E)    /* SYSRSTIV : SVSH */
#define SYSRSTIV_SVML_OVP  (0x0010)    /* SYSRSTIV : SVML_OVP */
#define SYSRSTIV_SVMH_OVP  (0x0012)    /* SYSRSTIV : SVMH_OVP */
#define SYSRSTIV_DOPOR     (0x0014)    /* SYSRSTIV : Do POR */
#define SYSRSTIV_WDTTO     (0x0016)    /* SYSRSTIV : WDT Time out */
#define SYSRSTIV_WDTKEY    (0x0018)    /* SYSRSTIV : WDTKEY violation */
#define SYSRSTIV_KEYV      (0x001A)    /* SYSRSTIV : Flash Key violation */
#define SYSRSTIV_PLLUL     (0x001C)    /* SYSRSTIV : PLL unlock */
#define SYSRSTIV_PERF      (0x001E)    /* SYSRSTIV : peripheral/config area fetch */
#define SYSRSTIV_PMMKEY    (0x0020)    /* SYSRSTIV : PMMKEY violation */

#if !defined(SYSRSTIV_BASE)
#define SYSRSTIV_BASE  0x0180
#endif

#define SYSRSTIV_START SYSRSTIV_BASE
#define SYSRSTIV_END   (SYSRSTIV_BASE + 0x20)

//System control SYSCTL 00h
//Bootstrap loader configuration area SYSBSLC 02h
//JTAG mailbox control SYSJMBC 06h
//JTAG mailbox input 0 SYSJMBI0 08h
//JTAG mailbox input 1 SYSJMBI1 0Ah
//JTAG mailbox output 0 SYSJMBO0 0Ch
//JTAG mailbox output 1 SYSJMBO1 0Eh
//Bus Error vector generator SYSBERRIV 18h
//User NMI vector generator SYSUNIV 1Ah
//System NMI vector generator SYSSNIV 1Ch
//Reset vector generator SYSRSTIV 1Eh

#define SYSCTL 0x00
#define SYSBSLC 0x02
#define SYSJMBC 0x06
#define SYSJMBI0 0x08
#define SYSJMBI1 0x0A
#define SYSJMBO0 0x0C
#define SYSJMBO1 0x0E
#define SYSBERRIV 0x18
#define SYSUNIV 0x1A
#define SYSSNIV 0x1C
#define SYSRSTIV 0x1E

void msp430_sysrstiv_create()
{
  msp430_io_register_range16(SYSRSTIV_START,SYSRSTIV_START+SYSRSTIV,msp430_sysrstiv_read,msp430_sysrstiv_write);
}

void msp430_sysrstiv_reset() {}

int16_t msp430_sysrstiv_read(uint16_t addr)
{
  switch (addr) {
    case  SYSRSTIV_BASE+SYSCTL:
    case  SYSRSTIV_BASE+SYSBSLC:
    case  SYSRSTIV_BASE+SYSJMBC:      
    case  SYSRSTIV_BASE+SYSJMBI0:      
    case  SYSRSTIV_BASE+SYSJMBI1:      
    case  SYSRSTIV_BASE+SYSJMBO0:      
    case  SYSRSTIV_BASE+SYSJMBO1:      
    case  SYSRSTIV_BASE+SYSBERRIV:      
    case  SYSRSTIV_BASE+SYSUNIV:      
    case  SYSRSTIV_BASE+SYSSNIV: 
      return 0; // we do not know better, dummy implementation
    case  SYSRSTIV_BASE+SYSRSTIV:      
      return SYSRSTIV_NONE; // reset reason always none
    default:
      break;
  }
  
  ERROR("msp430:sysrstiv: read not implemented for 0x%04x\n",addr);
  return 0;
}

void msp430_sysrstiv_write(uint16_t addr, int16_t val)
{
  static uint8_t already_message = 0;
  if (already_message)
    return;
  already_message = 1;
  ERROR("msp430:sysrstiv: write not implemented (0x%04x = 0x%02x)\n", addr, val);
}

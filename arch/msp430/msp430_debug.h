
/**
 *  \file   msp430_debug.h
 *  \brief  MSP430 MCU emulation debug
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_DEBUG_H
#define MSP430_DEBUG_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* msp430_debug_opcode    (unsigned short opcode, int b);
char* msp430_debug_regname   (unsigned short r);
char* msp430_debug_portname  (uint16_t addr);

void  msp430_print_registers (int columns);
void  msp430_print_stack     (int lines);

extern char* msp430_lpm_names[];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define _DEBUG_MSP430
#define _DEBUG_FETCH_DECODE
#define _DEBUG_DISASSEMBLE
#define _DEBUG_SIGNAL
#define _DEBUG_REGISTERS
#define _DEBUG_INTERRUPT
#define _DEBUG_GIE
#define _DEBUG_LPM            /* low power mode */
#define _DEBUG_SYSTEM_TIME    /* nano time      */
#define _DEBUG_SYSTEM_CLOCK   /* basic | fll    */
#define _DEBUG_IO             /* mcu bus io     */
#define _DEBUG_SFR            /* sfr registers  */

#define _DEBUG_TIMER          /* timer Ax Bx    */
#define _DEBUG_HWMUL          /* hardware mult. */
#define _DEBUG_BASIC_TIMER    /* basic timer    */
#define _DEBUG_WATCHDOG       /* watchdog       */
#define _DEBUG_USART          /* usart 0/1      */
#define _DEBUG_DIGI_IO        /* GPIO           */
#define _DEBUG_LCD            /* internal LCD   */
#endif

#ifdef DEBUG_MSP430
#    define HW_DMSG_MSP(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_MSP(x...) do { } while (0)
#endif

#ifdef DEBUG_FETCH_DECODE
#    define HW_DMSG_FD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_FD(x...) do { } while (0)
#endif

#ifdef DEBUG_DISASSEMBLE
#    define HW_DMSG_DIS(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_DIS(x...) do { } while (0)
#endif

#ifdef DEBUG_SIGNAL
#    define HW_DMSG_SIGNAL(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_SIGNAL(x...) do { } while (0)
#endif

#ifdef DEBUG_INTERRUPT
#    define HW_DMSG_INTR(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_INTR(x...) do { } while (0)
#endif

#ifdef DEBUG_GIE
#    define HW_DMSG_GIE(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_GIE(x...) do { } while (0)
#endif

#ifdef DEBUG_LPM
#    define HW_DMSG_LPM(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_LPM(x...) do { } while (0)
#endif

#ifdef DEBUG_SYSTEM_TIME
#    define HW_DMSG_TIME(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_TIME(x...) do { } while (0)
#endif

#ifdef DEBUG_IO
#    define HW_DMSG_IO(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_IO(x...) do { } while (0)
#endif

#ifdef DEBUG_SFR
#    define HW_DMSG_SFR(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_SFR(x...) do { } while (0)
#endif

#ifdef DEBUG_SYSTEM_CLOCK
#    define HW_DMSG_CLOCK(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_CLOCK(x...) do { } while (0)
#endif

#ifdef DEBUG_TIMER
#    define HW_DMSG_TIMER(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_TIMER(x...) do { } while (0)
#endif

#ifdef DEBUG_BASIC_TIMER
#    define HW_DMSG_BT(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_BT(x...) do { } while (0)
#endif

#ifdef DEBUG_WATCHDOG
#    define HW_DMSG_WD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_WD(x...) do { } while (0)
#endif

#ifdef DEBUG_DIGI_IO
#    define HW_DMSG_DIGI_IO(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_DIGI_IO(x...) do { } while (0)
#endif

#ifdef DEBUG_LCD
#    define HW_DMSG_LCD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_LCD(x...) do { } while (0)
#endif

#ifdef DEBUG_USART
#    define HW_DMSG_USART(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_USART(x...) do { } while (0)
#endif

#ifdef DEBUG_HWMUL
#    define HW_DMSG_HWMUL(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_HWMUL(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

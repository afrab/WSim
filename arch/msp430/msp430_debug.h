
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
#define DEBUG_MSP430        0
#define DEBUG_FETCH_DECODE  0
#define DEBUG_DISASSEMBLE   0
#define DEBUG_SIGNAL        0
#define DEBUG_REGISTERS     0
#define DEBUG_INTERRUPT     0
#define DEBUG_GIE           0
#define DEBUG_LPM           0 /* low power mode */
#define DEBUG_SYSTEM_TIME   0 /* nano time      */
#define DEBUG_SYSTEM_CLOCK  0 /* basic | fll    */
#define DEBUG_IO            0 /* mcu bus io     */
#define DEBUG_SFR           0 /* sfr registers  */


#define DEBUG_TIMER         0 /* timer Ax Bx    */
#define DEBUG_HWMUL         0 /* hardware mult. */
#define DEBUG_BASIC_TIMER   0 /* basic timer    */
#define DEBUG_WATCHDOG      0 /* watchdog       */
#define DEBUG_FLASH         0 /* mcu flash      */
#define DEBUG_USCIB         0 /* uscib 0/1      */
#define DEBUG_USART         0 /* usart 0/1      */
#define DEBUG_USART_INFO    0 /* usart 0/1      */
#define DEBUG_DIGI_IO       0 /* GPIO           */
#define DEBUG_LCD           0 /* internal LCD   */
#endif

#if DEBUG_MSP430
#    define HW_DMSG_MSP(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_MSP(x...) do { } while (0)
#endif

#if DEBUG_FETCH_DECODE
#    define HW_DMSG_FD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_FD(x...) do { } while (0)
#endif

#if DEBUG_DISASSEMBLE
#    define HW_DMSG_DIS(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_DIS(x...) do { } while (0)
#endif

#if DEBUG_SIGNAL
#    define HW_DMSG_SIGNAL(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_SIGNAL(x...) do { } while (0)
#endif

#if DEBUG_INTERRUPT
#    define HW_DMSG_INTR(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_INTR(x...) do { } while (0)
#endif

#if DEBUG_GIE
#    define HW_DMSG_GIE(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_GIE(x...) do { } while (0)
#endif

#if DEBUG_LPM
#    define HW_DMSG_LPM(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_LPM(x...) do { } while (0)
#endif

#if DEBUG_SYSTEM_TIME
#    define HW_DMSG_TIME(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_TIME(x...) do { } while (0)
#endif

#if DEBUG_IO
#    define HW_DMSG_IO(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_IO(x...) do { } while (0)
#endif

#if DEBUG_SFR
#    define HW_DMSG_SFR(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_SFR(x...) do { } while (0)
#endif

#if DEBUG_SYSTEM_CLOCK
#    define HW_DMSG_CLOCK(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_CLOCK(x...) do { } while (0)
#endif

#if DEBUG_TIMER
#    define HW_DMSG_TIMER(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_TIMER(x...) do { } while (0)
#endif

#if DEBUG_BASIC_TIMER
#    define HW_DMSG_BT(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_BT(x...) do { } while (0)
#endif

#if DEBUG_WATCHDOG
#    define HW_DMSG_WD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_WD(x...) do { } while (0)
#endif

#if DEBUG_DIGI_IO
#    define HW_DMSG_DIGI_IO(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_DIGI_IO(x...) do { } while (0)
#endif

#if DEBUG_LCD
#    define HW_DMSG_LCD(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_LCD(x...) do { } while (0)
#endif

#if DEBUG_FLASH
#    define HW_DMSG_FLASH(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_FLASH(x...) do { } while (0)
#endif

#if DEBUG_USCIB
#    define HW_DMSG_USCIB(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_USCIB(x...) do { } while (0)
#endif

#if DEBUG_USART
#    define HW_DMSG_USART(x...) HW_DMSG_MCUDEV(x)
#else
#    define HW_DMSG_USART(x...) do { } while (0)
#endif

#if DEBUG_USART_INFO
#    define HW_DMSG_USART_INFO(x...) HW_DMSG_MCU_INFO(x)
#else
#    define HW_DMSG_USART_INFO(x...) do { } while (0)
#endif

#if DEBUG_HWMUL
#    define HW_DMSG_HWMUL(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_HWMUL(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

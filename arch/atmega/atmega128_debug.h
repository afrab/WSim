
/**
 *  \file   atmega128_debug.h
 *  \brief  Atmega128 MCU emulation debug
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef ATMEGA128_DEBUG_H
#define ATMEGA128_DEBUG_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void  atmega128_print_SR       (void);
void  atmega128_print_registers(void);
char* atmega128_debug_portname(uint16_t addr);

extern char* atmega128_lpm_names[];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define DEBUG_ATMEGA
#define FETCH_DECODE
#define DISASSEMBLE
#define DEBUG_REGISTERS
#define DEBUG_INTERRUPT
#define _DEBUG_LPM            /* low power mode */
#define DEBUG_SYSTEM_CLOCK   /* basic | fll    */
#define DEBUG_IO             /* mcu bus io     */

#define _DEBUG_TIMER          /* timer Ax Bx    */
#define _DEBUG_HWMUL          /* hardware mult. */
#define _DEBUG_BASIC_TIMER    /* basic timer    */
#define _DEBUG_WATCHDOG       /* watchdog       */
#define _DEBUG_USART          /* usart 0/1      */
#define DEBUG_DIGI_IO        /* GPIO           */
#define _DEBUG_LCD            /* internal LCD   */
#endif

#ifdef DEBUG_ATMEGA
#    define HW_DMSG_ATM(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_ATM(x...) do { } while (0)
#endif

#ifdef FETCH_DECODE
#    define HW_DMSG_FD(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_FD(x...) do { } while (0)
#endif

#ifdef DISASSEMBLE
#    define HW_DMSG_DIS(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_DIS(x...) do { } while (0)
#endif

#ifdef DEBUG_LPM
#    define HW_DMSG_LPM(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_LPM(x...) do { } while (0)
#endif

#ifdef DEBUG_IO
#    define HW_DMSG_IO(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_IO(x...) do { } while (0)
#endif

#ifdef DEBUG_DIGI_IO
#    define HW_DMSG_DIGI_IO(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_DIGI_IO(x...) do { } while (0)
#endif

#ifdef DEBUG_IO_RESERVED
#    define HW_DMSG_IO_RESERVED(x...) HW_DMSG_MCU(x)
#else
#    define HW_DMSG_IO_RESERVED(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

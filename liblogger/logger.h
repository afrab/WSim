
/**
 *  \file   logger.h
 *  \brief  Wsim logger module
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef _LOGGER_H_
#define _LOGGER_H_

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void logger_init(const char *output, int level);
void logger_close();
extern int logger_verbose_level;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void REAL_STDOUT(char* fmt, ...);
void REAL_STDERR(char* fmp, ...);

#define STDOUT(x...) REAL_STDOUT(x)
#define STDERR(x...) REAL_STDERR(x)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * Verbose level
 * 
 * 0 - ERROR
 * 1 - WARNING
 * 2 - HW_DMSG + MAIN UTILS
 * 3 -          UART bytes
 * 4 -          ELF loader
 *   -          GDB engine
 * 5 -          GDB remote protocol dump
 *   -          ELF + system calls
 *   -          WSNet protocol dump
 * 6 -          ELF + struct dumps
 * 7 - RADIO State       
 */

/* ERROR is equal to 0 in mingw32 headers (Windows) */
#if defined(ERROR)
#undef ERROR        
#endif

void OUTPUT(char* fmt, ...);  // verbose 0
void ERROR (char* fmt, ...);  // verbose 0

#if defined(VERBOSE_IS_A_FUNC)
#else
#define VERBOSE(level, x...)                 \
do {                                         \
  if (logger_verbose_level >= level)         \
    {                                        \
      OUTPUT(x);                             \
    }                                        \
} while (0)
#endif

#define WARNING(x...)     VERBOSE(1,x)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(XCODE_DEBUG)
#define SW_DMSG(x...)     VERBOSE(2,x) 
#define DEBUG_SW_MEM
#define DEBUG_SW_BRK
#else
#define SW_DMSG(x...)     do {} while(0)
#endif


#ifdef DEBUG_SW_MEM
#    define SW_DMSG_MEM(x...) SW_DMSG(x)
#else
#    define SW_DMSG_MEM(x...) do { } while (0)
#endif

#ifdef DEBUG_SW_BRK
#    define SW_DMSG_BRK(x...) SW_DMSG(x)
#else
#    define SW_DMSG_BRK(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define HW_DMSG(x...)     VERBOSE(2,x) 
#define DEBUG_MCU
#define DEBUG_MACH
#define DEBUG_DEV        
#define DEBUG_UI
#define DEBUG_GDB
#else
#define HW_DMSG(x...)     do {} while(0)
#endif


#ifdef DEBUG_MCU
#    define HW_DMSG_MCU(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_MCU(x...)  do { } while (0)
#endif

#ifdef DEBUG_MACH
#    define HW_DMSG_MACH(x...) HW_DMSG(x)
#else
#    define HW_DMSG_MACH(x...) do { } while (0)
#endif

#ifdef DEBUG_DEV
#    define HW_DMSG_DEV(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_DEV(x...)  do { } while (0)
#endif

#ifdef DEBUG_UI
#    define HW_DMSG_UI(x...)   HW_DMSG(x)
#else
#    define HW_DMSG_UI(x...)   do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif


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
 * 0 - ERROR / MESSAGES
 * 1 - WARNING
 * 2 - INFO
 * 3 - 
 * 4 -
 * 5 - HW_DMSG - hardware related messages
 *   -          mcu
 *   -          mcu devices
 *   -          platform
 *   -          platform devices 
 *   -          machine
 * 6 - LIB_DMSG -
 *   -          ELF   loader
 *   -          WSNet protocol dump
 *   -          GDB   engine
 *   -          GUI   backend
 *   -          tracer
 *   -          logpkt
 *   -          etracer
 *   -          
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

#define MESSAGE(x...)     VERBOSE(0,x)
#define WARNING(x...)     VERBOSE(1,x)
#define INFO(x...)        VERBOSE(2,x)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(XCODE_DEBUG)
#define SW_DMSG(x...)     VERBOSE(2,x) 
#define DEBUG_SW_MEM 0
#define DEBUG_SW_BRK 0
#else
#define SW_DMSG(x...)     do {} while(0)
#endif


#if DEBUG_SW_MEM != 0
#    define SW_DMSG_MEM(x...) SW_DMSG(x)
#else
#    define SW_DMSG_MEM(x...) do { } while (0)
#endif

#if DEBUG_SW_BRK != 0
#    define SW_DMSG_BRK(x...) SW_DMSG(x)
#else
#    define SW_DMSG_BRK(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define HW_DMSG(x...)     VERBOSE(5,x) 
#define DEBUG_MCU      0
#define DEBUG_MCUDEV   1
#define DEBUG_MACH     0
#define DEBUG_DEV      0
#define DEBUG_PLATFORM 0
#else
#define HW_DMSG(x...)     do {} while(0)
#endif


#if DEBUG_MCU != 0
#    define HW_DMSG_MCU(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_MCU(x...)  do { } while (0)
#endif


#if DEBUG_MCUDEV != 0
#    define HW_DMSG_MCUDEV(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_MCUDEV(x...)  do { } while (0)
#endif


#if DEBUG_MACH != 0
#    define HW_DMSG_MACH(x...) HW_DMSG(x)
#else
#    define HW_DMSG_MACH(x...) do { } while (0)
#endif


#if DEBUG_DEV != 0
#    define HW_DMSG_DEV(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_DEV(x...)  do { } while (0)
#endif


#if DEBUG_PLATFORM != 0
#    define HW_DMSG_PLATFORM(x...)  HW_DMSG(x)
#else
#    define HW_DMSG_PLATFORM(x...)  do { } while (0)
#endif



/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define DMSG_LIB(x...)      VERBOSE(6,x) 
#define DEBUG_UI            0
#define DEBUG_SELECT        0
#define DEBUG_GDB_SRP_PROTO 0        /* Serial remote protocol commands / replies  */
#define DEBUG_GDB_CMD       0        /* GDB detailed debug of commands and actions */
#define DEBUG_iHEX          0      
#define DEBUG_LIBWSNET      0
#define DEBUG_LIB_ELF       0
#define DEBUG_LIB_ELF_DMP   0
#else
#define DMSG_LIB(x...)      do {} while(0)
#endif


#if DEBUG_UI != 0
#    define DMSG_LIB_UI(x...)       DMSG_LIB(x)
#else
#    define DMSG_LIB_UI(x...)       do { } while (0)
#endif

#if DEBUG_SELECT != 0
#    define DMSG_LIB_SELECT(x...)   DMSG_LIB(x)
#else
#    define DMSG_LIB_SELECT(x...)   do { } while (0)
#endif


#if DEBUG_GDB_CMD != 0
#    define DMSG_LIB_GDB_CMD(x...)  DMSG_LIB(x)
#else
#    define DMSG_LIB_GDB_CMD(x...)  do { } while (0)
#endif

#if DEBUG_GDB_SRP_PROTO != 0
#    define DMSG_LIB_GDB(x...)      DMSG_LIB(x)
#else
#    define DMSG_LIB_GDB(x...)      do { } while (0)
#endif


#if DEBUG_iHEX != 0
#    define DMSG_LIB_iHEX(x...)     DMSG_LIB(x)
#else
#    define DMSG_LIB_iHEX(x...)     do { } while (0)
#endif


#if DEBUG_LIBWSNET != 0
#    define DMSG_LIB_WSNET(x...)    DMSG_LIB(x)
#else
#    define DMSG_LIB_WSNET(x...)    do { } while (0)
#endif


#if DEBUG_LIB_ELF != 0
#    define DMSG_LIB_ELF(x...)      DMSG_LIB(x)
#else
#    define DMSG_LIB_ELF(x...)      do { } while (0)
#endif

#if DEBUG_LIB_ELF_DMP != 0
#    define DMSG_LIB_ELF_DMP(x...)  DMSG_LIB(x)
#else
#    define DMSG_LIB_ELF_DMP(x...)  do { } while (0)
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

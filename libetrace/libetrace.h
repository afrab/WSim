
/**
 *  \file   libetrace.h
 *  \brief  WSim energy tracer for eSimu
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef _WSIM_LIBETRACE_H_
#define _WSIM_LIBETRACE_H_

#include <stdint.h>
#include "config.h"

#if defined(ETRACE)
/* ************************************************** */
/* ************************************************** */

#define ETRACE_OK                         0
#define ETRACE_ERROR                      1
#define ETRACE_QUIT                       2

/* instruction constant definitions */
/* ******************************** */
#define ETRACER_INSN_CLASS_IDLE        0x00
#define ETRACER_INSN_CLASS_1           0x01
#define ETRACER_INSN_CLASS_2           0x02
#define ETRACER_INSN_CLASS_3           0x03

#define ETRACER_BRANCH_NONE            0x00
#define ETRACER_BRANCH_JUMP            0x01
#define ETRACER_BRANCH_CALL            0x02
#define ETRACER_BRANCH_XCALL           0x04
#define ETRACER_BRANCH_UNKN            0x0A

/* access and bus activation constant definitions */
/* ********************************************** */
#define ETRACER_ACCESS_READ            0x00
#define ETRACER_ACCESS_WRITE           0x01

#define ETRACER_ACCESS_WIDTH           0x0F
#define ETRACER_ACCESS_BYTE            0x01
#define ETRACER_ACCESS_HWORD           0x02
#define ETRACER_ACCESS_WORD            0x03
#define ETRACER_ACCESS_BIT             0x04

#define ETRACER_ACCESS_LVL_BUS         0x00 /* main bus  */
#define ETRACER_ACCESS_LVL_GPIO        0x01 /* gpio bits */
#define ETRACER_ACCESS_LVL_SPI         0x02 /* spi base  */
#define ETRACER_ACCESS_LVL_SPI0        0x02 /* spi 0     */
#define ETRACER_ACCESS_LVL_SPI1        0x03 /* spi 1     */
#define ETRACER_ACCESS_LVL_OUT         0x04 /* outside   */

/* events constant definiction 5 bits = 32 ids */
/* ******************************************* */

/* periph_id peripheral Id */
#define ETRACER_PER_ID_MCU_CPU         0x00
#define ETRACER_PER_ID_MCU_GPIO        0x01

#define ETRACER_PER_ID_MCU_USART       0x02
#define ETRACER_PER_ID_MCU_USART0      0x02
#define ETRACER_PER_ID_MCU_USART1      0x03

#define ETRACER_PER_ID_MCU_SPI         0x04
#define ETRACER_PER_ID_MCU_SPI0        0x04
#define ETRACER_PER_ID_MCU_SPI1        0x05

#define ETRACER_PER_ID_CC1100          0x06
#define ETRACER_PER_ID_M25P            0x07
#define ETRACER_PER_ID_DS2411          0x08
#define ETRACER_PER_ID_CONSOLE         0x09
#define ETRACER_PER_ID_AT45DB          0x07 /* 0x0a */ /* same as M25P until eSimu is updated   */
#define ETRACER_PER_ID_CC2420          0x06 /* 0x0b */ /* same as CC1100 until eSimu is updated */

/* event_id:5 */
#define ETRACER_PER_EVT_WRITE_COMMAND  0x00
#define ETRACER_PER_EVT_READ_COMMAND   0x01
#define ETRACER_PER_EVT_MODE_CHANGED   0x02

/* arg:6 values for write and read commands */
#define ETRACER_PER_ARG_WR_LVL         0x0F /* level mask          */
#define ETRACER_PER_ARG_WR_HDL         0xF0 /* value mask          */
#define ETRACER_PER_ARG_WR_SRC         0x00 /* src            0000 */
#define ETRACER_PER_ARG_WR_DST         0x10 /* dst            0001 */
#define ETRACER_PER_ARG_WR_SRC_FIFO    0x20 /* src data       0010 */
#define ETRACER_PER_ARG_WR_DST_FIFO    0x30 /* dst fifo       0011 */
#define ETRACER_PER_ARG_WR_SRC_EXT     0x40 /* out            0100 */
#define ETRACER_PER_ARG_WR_DST_EXT     0x50 /* in             0101 */
#define ETRACER_PER_ARG_WR_FLUSH       0x80 /* flush          1xxx */

/* arg:6 event peripherals specific definitions */
#define ETRACER_M25P_POWER_DEEP_DOWN   0x01
#define ETRACER_M25P_POWER_STANDBY     0x02
#define ETRACER_M25P_POWER_ACTIVE      0x03
#define ETRACER_M25P_POWER_READ        0x04
#define ETRACER_M25P_POWER_WRITE       0x05

#define ETRACER_AT45DB_POWER_DEEP_DOWN 0x01
#define ETRACER_AT45DB_POWER_STANDBY   0x02
#define ETRACER_AT45DB_POWER_ACTIVE    0x03
#define ETRACER_AT45DB_POWER_READ      0x04
#define ETRACER_AT45DB_POWER_WRITE     0x05

#define ETRACER_CC1100_IDLE            0x01
#define ETRACER_CC1100_SLEEP           0x02
#define ETRACER_CC1100_XOFF            0x03
#define ETRACER_CC1100_STARTUP         0x04
#define ETRACER_CC1100_RX              0x05
#define ETRACER_CC1100_TX              0x06

#define ETRACER_CC2420_IDLE            0x01
#define ETRACER_CC2420_SLEEP           0x02
#define ETRACER_CC2420_VREG_OFF        0x03
#define ETRACER_CC2420_STARTUP         0x04
#define ETRACER_CC2420_RX              0x05
#define ETRACER_CC2420_TX              0x06

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * etracer creation
 */
void etracer_init          (char* filename, int ws_mode);
void etracer_close         (void);

/**
 *
 */
void etracer_start         (void);
void etracer_stop          (void);


/*
 * pc         : current pc
 * class      : insn class 
 * timing     : insn cycle count 
 * non_seq    : 0=seq 1=brach
 * ret_addr   : next pc 
 * b_type     : branch type (BRANCH_JUMP / BRANCH_CALL)
 */
extern void (*etracer_slot_insn_ptr)(uint32_t pc, uint8_t class, uint16_t cycles, 
				     uint8_t non_seq, uint16_t ret_addr, 
				     uint8_t b_type, uint8_t except, uint8_t reti);
#define etracer_slot_insn(pc,class,cycles,non_seq,ret_addr,b_type,except,reti)  \
do {                                                                            \
  if (etracer_slot_insn_ptr != NULL)                                            \
    etracer_slot_insn_ptr(pc,class,cycles,non_seq,ret_addr,b_type,except,reti); \
} while(0)


/*
 * base_addr  : addr
 * burst_size : word size burst (1)
 * type       : RW_ACCESS | ACCESS_WIDTH
 * level      : 1
 * timing     : 0 for Flash & RAM
 */
extern void (*etracer_slot_access_ptr)(uint32_t addr, uint8_t burst_size, uint8_t type, 
				       uint8_t width, uint8_t level, uint16_t timing);
#define etracer_slot_access(addr,burst_size,type,width,level,timing)  \
do {                                                                  \
  if (etracer_slot_access_ptr != NULL)                                \
    etracer_slot_access_ptr(addr,burst_size,type,width,level,timing); \
} while(0)


/* 
 * periph_id  : 
 * event_id   : 
 * skew       : time shift from beginning of slot
 */
extern void (*etracer_slot_event_ptr)(uint8_t periph_id, uint8_t event_id, uint8_t arg, uint8_t skew);
#define etracer_slot_event(periph_id,event_id,arg,skew)               \
do {                                                                  \
  if (etracer_slot_event_ptr != NULL)                                 \
    etracer_slot_event_ptr(periph_id,event_id,arg,skew);              \
} while(0)


/* 
 * periph_id  : 
 * event_id   : 
 * skew       : time shift from beginning of slot
 */
extern void (*etracer_slot_set_pc_ptr)(uint32_t pc);
#define etracer_slot_set_pc(pc)                                       \
do {                                                                  \
  if (etracer_slot_set_pc_ptr != NULL)                                \
    etracer_slot_set_pc_ptr(pc);                                      \
} while(0)

extern void (*etracer_slot_set_ns_ptr)();
#define etracer_slot_set_ns()                                         \
do {                                                                  \
  if (etracer_slot_set_ns_ptr != NULL)                                \
    etracer_slot_set_ns_ptr();                                        \
} while(0)


/*
 * end slot 
 */
extern void (*etracer_slot_end_ptr)(int timing);
#define etracer_slot_end(timing)                                      \
do {                                                                  \
  if (etracer_slot_end_ptr != NULL)                                   \
    etracer_slot_end_ptr(timing);                                     \
} while(0)


/* 
 * state save/restore for machine backtracks
 */
void etracer_state_save    (void);
void etracer_state_restore (void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#else
#define NOOP do { } while (0)

#define etracer_init(x...)        NOOP
#define etracer_close()           NOOP

#define etracer_slot_insn(x...)   NOOP
#define etracer_slot_access(x...) NOOP
#define etracer_slot_event(x...)  NOOP
#define etracer_slot_set_pc(x...) NOOP
#define etracer_slot_set_ns()     NOOP
#define etracer_slot_end(x...)    NOOP

#define etracer_start()           NOOP
#define etracer_stop()            NOOP

#define etracer_state_save()      NOOP
#define etracer_state_restore()   NOOP

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif /* ETRACE */
#endif /* _WSIM_LIBETRACE_H */


/**
 *  \file   atmega128.h
 *  \brief  Atmega128 MCU definition and macros
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef ATMEGA128_H
#define ATMEGA128_H

#define TARGET_BYTE_ORDER_DEFAULT LITTLE_ENDIAN

#include "atmega128_models.h"
#include "atmega128_debug.h"
#include "atmega128_alu.h"
#include "atmega128_digiIO.h"
#include "atmega128_io.h"

#define MCU_T atmega128_mcu_t

#define MAX_EEPROM_SIZE       0x01000   /*   4kB */
#define MAX_FLASH_SIZE        0x20000   /* 128kB */
#define MAX_IOREGION_SIZE     0x00100   /* 256B  */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct atmega128_mcu_t {
  uint8_t flash       [MAX_FLASH_SIZE];
  uint8_t eeprom      [MAX_EEPROM_SIZE];

  struct atmega128_alu_t          alu;
  struct atmega128_digiIO_t       digiIO;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

extern tracer_id_t ATMEGA_TRACER_ACLK;
extern tracer_id_t ATMEGA_TRACER_MCLK;
extern tracer_id_t ATMEGA_TRACER_SMCLK;
extern tracer_id_t ATMEGA_TRACER_GIE;
extern tracer_id_t ATMEGA_TRACER_PC;
extern tracer_id_t ATMEGA_TRACER_SP;
extern tracer_id_t ATMEGA_TRACER_INTR;
extern tracer_id_t ATMEGA_TRACER_LPM;
extern tracer_id_t ATMEGA_TRACER_PORT1;
extern tracer_id_t ATMEGA_TRACER_PORT2;
extern tracer_id_t ATMEGA_TRACER_PORT3;
extern tracer_id_t ATMEGA_TRACER_PORT4;
extern tracer_id_t ATMEGA_TRACER_PORT5;
extern tracer_id_t ATMEGA_TRACER_PORT6;
extern tracer_id_t ATMEGA_TRACER_USART0;
extern tracer_id_t ATMEGA_TRACER_USART1;

#define TRACER_TRACE_PC(v)      tracer_event_record(ATMEGA_TRACER_PC,v)
#define TRACER_TRACE_SP(v)      tracer_event_record(ATMEGA_TRACER_SP,v)
#define TRACER_TRACE_GIE(v)     tracer_event_record(ATMEGA_TRACER_GIE,v)
#define TRACER_TRACE_INTR(v)    tracer_event_record(ATMEGA_TRACER_INTR,v)
#define TRACER_TRACE_LPM(v)     tracer_event_record(ATMEGA_TRACER_LPM,v)

#define TRACER_TRACE_ACLK(v)    tracer_event_record(ATMEGA_TRACER_ACLK,v)
#define TRACER_TRACE_MCLK(v)    tracer_event_record(ATMEGA_TRACER_MCLK,v)
#define TRACER_TRACE_SMCLK(v)   tracer_event_record(ATMEGA_TRACER_SMCLK,v)

#define TRACER_TRACE_PORT1(v)   tracer_event_record(ATMEGA_TRACER_PORT1,v)
#define TRACER_TRACE_PORT2(v)   tracer_event_record(ATMEGA_TRACER_PORT2,v)
#define TRACER_TRACE_PORT3(v)   tracer_event_record(ATMEGA_TRACER_PORT3,v)
#define TRACER_TRACE_PORT4(v)   tracer_event_record(ATMEGA_TRACER_PORT4,v)
#define TRACER_TRACE_PORT5(v)   tracer_event_record(ATMEGA_TRACER_PORT5,v)
#define TRACER_TRACE_PORT6(v)   tracer_event_record(ATMEGA_TRACER_PORT6,v)

#define TRACER_TRACE_USART0(v)  tracer_event_record(ATMEGA_TRACER_USART0,v)
#define TRACER_TRACE_USART1(v)  tracer_event_record(ATMEGA_TRACER_USART1,v)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

extern struct atmega128_mcu_t mcu;
extern struct atmega128_mcu_t mcu_backup;

#define MCU                mcu
#define MCU_ALU            MCU.alu
#define MCU_SIGNAL         MCU_ALU.signal
#define MCU_INSN_CPT       MCU_ALU.insn_counter
#define MCU_CYCLE_CPT      MCU_ALU.cycle_counter
#define MCU_HWMUL          MCU.hwmul
#define MCU_FLASH          MCU.flash
#define MCU_RAM            MCU_ALU.ram

#define MCU_BOOT_ADDRESS   0x0

#define CYCLE_INCREMENT    MCU_CLOCK.MCLK_increment

int atmega128_mcu_create(int xtal, int xosc);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif  /* ifndef ATMEGA128_H */

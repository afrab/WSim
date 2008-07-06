
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
#define MCU_RAMCTL         MCU_ALU.ramctl

#define MCU_BOOT_ADDRESS   0x0

#define CYCLE_INCREMENT    MCU_CLOCK.MCLK_increment

int atmega128_mcu_create(int xtal, int xosc);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif  /* ifndef ATMEGA128_H */

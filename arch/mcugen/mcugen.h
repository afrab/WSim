
/**
 *  \file   mcugen.h
 *  \brief  Generic MCU ALU emulation
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef MCUGEN_H
#define MCUGEN_H

#define MAX_RAM_SIZE  0x1000
#define MCU_REGISTERS   16

typedef int16_t mcu_register_t;

struct mcugen_mcu_t {
  mcu_register_t   regs[MCU_REGISTERS];
  uint8_t          ram [MAX_RAM_SIZE];

  uint64_t         clock_counter;
  int              clock_increment;
  uint32_t         clock_freq;
  uint32_t         clock_cycle_nanotime;
  uint32_t         signal;

};

extern struct mcugen_mcu_t mcu;
extern struct mcugen_mcu_t mcu_backup;

#define MCU              mcu
#define MCU_REGS         mcu.regs
#define MCU_RAM          mcu.ram

int mcugen_mcu_create(int freq);

#define MCU_NAME        "mcugen"
#define MCU_MODEL_NAME  "mv3"
#define MCU_ARCH_ID     0 /* libelf arch ID */
#define MCU_MACH_ID     0

#endif

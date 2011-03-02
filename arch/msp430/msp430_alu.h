
/**
 *  \file   msp430_alu.h
 *  \brief  MSP430 ALU emulation 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_ALU_H
#define MSP430_ALU_H

#define MCU_REGISTERS   	16

typedef int16_t mcu_register_t; 

struct msp430_alu_t {
  mcu_register_t   regs[MCU_REGISTERS];
  uint32_t         interrupt_vector;
  uint32_t         signal;

  uint64_t         insn_counter;
  uint64_t         cycle_counter;
  uint64_t         irq_counter;

  // etrace + gdb utils
  mcu_register_t   curr_pc;
  mcu_register_t   sequ_pc;
  mcu_register_t   next_pc;

  // next sequential address
  int      etracer_seq_address;
  uint8_t  etracer_nseq_flag;
  uint8_t  etracer_branch_type;
  uint8_t  etracer_except;
  uint8_t  etracer_reti;
};

#define PC_REG_IDX  0
#define SP_REG_IDX  1
#define SR_REG_IDX  2
#define CG1_REG_IDX 2
#define CG2_REG_IDX 3

// #define PC_Current MCU_ALU.current_pc
// #define PC_Next    MCU_ALU.next_pc
// #define PC	      MCU_ALU.regs[PC_REG_IDX]
#define	SP	   MCU_ALU.regs[SP_REG_IDX]
#define	SR	   MCU_ALU.regs[SR_REG_IDX]
#define	CG1	   MCU_ALU.regs[CG1_REG_IDX]
#define	CG2	   MCU_ALU.regs[CG2_REG_IDX]

/** 
 * status register (register 2)
 * 
 *         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * SR (2) [  reserved   |v|x|x|x|x|x|n|z|c]
 *
 *  8 : v       overflow bit
 *  7 : scg1    system clock generator 1
 *  6 : scg0    system clock generator 0
 *  5 : oscoff  oscillator off
 *  4 : cpuoff  cpu off
 *  3 : gie     general interrupt enable
 *  2 : n       negative bit
 *  1 : z       zero bit
 *  0 : c       carry bit
 **/

#define SHIFT_SCG1   7
#define SHIFT_SCG0   6
#define SHIFT_OSCOFF 5
#define SHIFT_CPUOFF 4
#define SHIFT_GIE    3

#define MASK_SCG1    0x0080
#define MASK_SCG0    0x0040
#define MASK_OSCOFF  0x0020
#define MASK_CPUOFF  0x0010
#define MASK_GIE     0x0008

#define SET_SCG1     SR |= MASK_SCG1
#define SET_SCG0     SR |= MASK_SCG0
#define SET_OSCOFF   SR |= MASK_OSOFF
#define SET_CPUOFF   SR |= MASK_CPUOFF
#define SET_GIE      SR |= MASK_GIE

#define CLR_SCG1     SR &= ~MASK_SCG1
#define CLR_SCG0     SR &= ~MASK_SCG0
#define CLR_OSCOFF   SR &= ~MASK_OSOFF
#define CLR_CPUOFF   SR &= ~MASK_CPUOFF
#define CLR_GIE      SR &= ~MASK_GIE

#define MCU_READ_GIE     ((uint16_t)(SR & MASK_GIE   ) >> 3)
#define MCU_READ_CPUOFF  ((uint16_t)(SR & MASK_CPUOFF) >> 4) 
#define MCU_READ_OSCOFF  ((uint16_t)(SR & MASK_OSCOFF) >> 5)
#define MCU_READ_SCG0    ((uint16_t)(SR & MASK_SCG0  ) >> 6)
#define MCU_READ_SCG1    ((uint16_t)(SR & MASK_SCG1  ) >> 7)

/************************/
/* Low power modes      */
/************************/

/**
 * internal MCU Signals used by the simulation engine
 * enum mcu_signal_t {    is defined un arch/common/mcu.h 
 */

#define RUNNING_MODE_FROM_REG(R) ((R >> 4) & 0x0fu)
#define RUNNING_MODE() RUNNING_MODE_FROM_REG(SR)
#define RUNNING_AM    0x0
#define RUNNING_LPM0  0x1
#define RUNNING_LPM1  0x5
#define RUNNING_LPM2  0x9
#define RUNNING_LPM3  0xd
#define RUNNING_LPM4  0xf

/************************/
/*                      */
/************************/

void     msp430_alu_reset (void);

#endif

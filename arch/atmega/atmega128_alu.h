
/**
 *  \file   atmega128_alu.h
 *  \brief  Atmega128 MCU ALU emulation 
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef ATMEGA128_ALU_H
#define ATMEGA128_ALU_H


/* 
 * Atmega 128 data memory map
 *
 * Data Memory
 * 32 Registers     $0000 - $001F
 * 64 I/O Registers $0020 - $005F
 * 160 Ext I/O Reg. $0060 - $00FF
 *                  $0100
 * Internal SRAM
 *   (4096 x 8)
 *                  $10FF
 *                  $1100
 * External SRAM
 *   (0 - 64K x 8)
 *                  $FFFF
 */

#define MCU_REGISTERS              32

#if defined(DEBUG)
#define MAX_RAM_SIZE          0x10100   /*  64kB + regs & i/o */
#else
#define MAX_RAM_SIZE          0x01100   /*   4kB + regs & i/o */
#endif

typedef int8_t   mcu_register_t;
typedef uint32_t mcu_register16_t;

// /* Endian problem */
struct reg16_HL_t {
  uint8_t high;
  uint8_t low;
};

union  u_ureg16_t {
  uint16_t          val;
  struct reg16_HL_t bytes;
};

typedef union u_ureg16_t ureg16hl_t;

struct atmega128_alu_t {
  uint8_t          ram[MAX_RAM_SIZE];

  uint16_t         pc;
  uint16_t         next_pc;
  uint16_t         skip_execute;

  int              interrupts;
  uint16_t         interrupt_vector;
  uint64_t         insn_counter;
  uint64_t         cycle_counter;
  uint32_t         signal;

  int              etracer_nseq_address;
  int              etracer_next_address;

  uint8_t          etracer_nseq_flag;
  uint8_t          etracer_branch_type;
  uint8_t          etracer_except;
  uint8_t          etracer_reti;
};

/************************/
/* Low power modes      */
/************************/

#define MCU_REGS      MCU_ALU.ram
#define MCU_RAM       MCU_ALU.ram
#define MCU_IOREGS    ((int8_t*) (& MCU_ALU.ram[32]))

#define SREG          MCU_IOREGS[IO_REG_SREG]
#define SR            SREG
#define SPH           MCU_IOREGS[IO_REG_SPH]
#define SPL           MCU_IOREGS[IO_REG_SPL]
#define RAMPZ         MCU_IOREGS[IO_REG_RAMPZ]

/* ************************************************** */

#define HW_DMSG_W16(x...) do { } while (0)

#define REG_16_READ(lh,ll) ((MCU_REGS[lh] << 8) | (MCU_REGS[ll] & 0xff))
#define REG_16_WRITE(val,lh,ll)                                              \
do {                                                                         \
  HW_DMSG_W16("write16 high byte [0x%04x] = 0x%02x\n",lh,(val >> 8) & 0xff); \
  HW_DMSG_W16("write16 low byte  [0x%04x] = 0x%02x\n",ll,(val >> 0) & 0xff); \
  MCU_REGS[lh] = (val >> 8) & 0xff;                                          \
  MCU_REGS[ll] = (val >> 0) & 0xff;                                          \
} while (0)

#define REG_X_READ()       REG_16_READ(27,26)
#define REG_X_WRITE(val)   REG_16_WRITE(val,27,26)

#define REG_Y_READ()       REG_16_READ(29,28)
#define REG_Y_WRITE(val)   REG_16_WRITE(val,29,28)

#define REG_Z_READ()       REG_16_READ(31,30)
#define REG_Z_WRITE(val)   REG_16_WRITE(val,31,30)

/* ************************************************** */

#define IOREG_16_READ(lh,ll) ((MCU_IOREGS[lh] << 8) | (MCU_IOREGS[ll] & 0xff))
#define IOREG_16_WRITE(val,lh,ll)     \
do {                                \
  MCU_IOREGS[lh] = (val >> 8) & 0xff; \
  MCU_IOREGS[ll] =  val       & 0xff; \
} while (0)

#define SP_READ()          IOREG_16_READ(IO_REG_SPH,IO_REG_SPL)
#define SP_WRITE(val)      IOREG_16_WRITE(val,IO_REG_SPH,IO_REG_SPL)

/* ************************************************** */

#define RUNNING_MODE_FROM_REG(R) (0) /* FIXME */
#define RUNNING_MODE() RUNNING_MODE_FROM_REG(SR)

/************************/
/*                      */
/************************/

void         atmega128_alu_reset (void);
void         atmega128_set_pc    (uint16_t addr);
uint16_t     atmega128_get_pc    (void); 

#endif

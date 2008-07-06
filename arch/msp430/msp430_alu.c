
/**
 *  \file   msp430_alu.c
 *  \brief  MSP430 ALU emulation 
 *  \author Antoine Fraboulet
 *  \date   2005
 *
 *  All rights reserved 
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * global variables used in this file
 *
 * static struct msp430_op_type1 opt1;
 * static struct msp430_op_type2 opt2;
 *
 */ 

#define __DEBUG_ME_HARDER

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

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

#define SET_V        SR |= 0x0100
#define SET_N        SR |= 0x0004
#define SET_Z        SR |= 0x0002
#define SET_C        SR |= 0x0001

#define CLR_V        SR &= ~0x0100
#define CLR_N        SR &= ~0x0004
#define CLR_Z        SR &= ~0x0002
#define CLR_C        SR &= ~0x0001

#define WRITE_V(b)   if (b) { SET_V; } else { CLR_V; }
#define WRITE_N(b)   if (b) { SET_N; } else { CLR_N; }
#define WRITE_Z(b)   if (b) { SET_Z; } else { CLR_Z; }
#define	WRITE_C(b)   if (b) { SET_C; } else { CLR_C; }

#define READ_V       ((uint16_t)(SR & 0x0100) >> 8)
#define READ_N       ((uint16_t)(SR & 0x0004) >> 2)
#define READ_Z       ((uint16_t)(SR & 0x0002) >> 1)
#define	READ_C       ((uint16_t)(SR & 0x0001) >> 0)

/************************/
/* MSP430 MACHINE       */
/************************/

inline uint16_t mcu_get_pc(void)
{
  return MCU_ALU.regs[PC_REG_IDX];
}

inline uint16_t mcu_get_pc_next(void)
{
  return MCU_ALU.next_pc;
}

inline void mcu_set_pc_next(uint16_t x)
{
  /* next instruction */
  MCU_ALU.next_pc = x;
}

inline void msp430_alu_reset(void)
{
  int i;
  for(i=0; i < MCU_REGISTERS; i++) 
    {
      MCU_ALU.regs[i] = 0;
    }

  MCU_ALU.regs[PC_REG_IDX] = msp430_read_short(0xFFFEu);
  MCU_ALU.next_pc          = MCU_ALU.regs[PC_REG_IDX]; 
  SR = 0;

  MCU_ALU.interrupts = 0;
  MCU_ALU.signal     = 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

/**
 *   MSP340 Instruction set architecture (ISA)   [slau056e.pdf, 3-19]
 *
 * Type 1 : double operands
 * Type 2 : single operand
 * Type 3 : jumps
 *   
 *         1 1 1 1 1 1 
 *         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *
 * T1     [  op   | sreg  |x|b| as| dreg  ] x=ad
 *
 * T2     [  op             |b| ad| d/sreg]
 *
 * T3     [  op |  cc |       offset      ]
 *
 *        [       |       |        |      ]
 *
 * T1   opcode on 4 bits   = b15=x & b14=x & b15 | b14 true
 * T3   opcode on 3 bits   = b15=0 & b14=0 & b13 = 1
 * T2   opcode on          = b15=0 & b14=0 & b13 = 0
 * 
 **/ 

/** 
 * type 1 : double operand
 **/
#define OP_MOV	    0x4 // 0100
#define OP_ADD	    0x5 // 0101
#define OP_ADDC	    0x6 // 0110
#define OP_SUBC	    0x7 // 0111
#define OP_SUB	    0x8 // 1000
#define OP_CMP	    0x9 // 1001
#define OP_DADD	    0xa // 1010
#define OP_BIT	    0xb // 1011
#define OP_BIC	    0xc // 1100
#define OP_BIS	    0xd // 1101
#define OP_XOR	    0xe // 1110
#define OP_AND	    0xf // 1111

/**
 * type 2 : single operand
 * not right shifted to avoid collisions with type 3
 **/
#define OP_SXT	  0x118 // 0001 0001 1000
#define OP_CALL   0x128 // 0001 0010 1000
#define OP_RETI   0x130 // 0001 0011 0000
#define OP_PUSH   0x120 // 0001 0010 0000
#define OP_SWPB   0x108 // 0001 0000 1000
#define OP_RRA	  0x110 // 0001 0001 0000
#define OP_RRC	  0x100 // 0001 0000 0000

/**
 * type 3 : 2 bits + 2 bits 
 *
 * 0011 11 0 ... 0
 *        | 10 bits
 **/
#define OP_JNZ	   0x20 // 0010 0000
#define OP_JZ	   0x24 // 0010 0100
#define OP_JNC	   0x28 // 0010 1000
#define OP_JC	   0x2c // 0010 1100
#define OP_JN	   0x30 // 0011 0000
#define OP_JGE	   0x34 // 0011 0100
#define OP_JL	   0x38 // 0011 1000
#define OP_JMP	   0x3c // 0011 1100

static inline unsigned int extract_opcode(uint16_t insn)
{
  unsigned int tmp;

  tmp = (insn >> 12) & 0x0f; // 4 bits

  HW_DMSG_DIS("PC:0x%04x ins:0x%04x  ",mcu_get_pc() & 0xffff,insn & 0xffff);
  
  /* type 1 = double operands : opcode 4 bits */
  if ((tmp & 0xc) != 0x0) 
    {
      return tmp;
    }
  /* type 3 = jumps : opcode 3 bits */
  if ((tmp & 0x2) == 0x2) 
    {
      return ((unsigned int)insn >> 8) & ~3;
    }
  /* type 2 = single operands or SIGTRAP x */
  return (insn >> 4) & ~7;
}

/***********************/
/*                     */
/***********************/

#if defined(ETRACE)
#define ETRACER_SET_JUMP_TYPE(x)  MCU_ALU.etracer_branch_type    = x
#define ETRACER_BRANCH(x)         MCU_ALU.etracer_branch_type    = x


static inline int msp430_insn_class(int opcode)
{
  switch (opcode)
    {
      /** 
       * type 1 : double operand
       **/
    case OP_MOV	    : return ETRACER_INSN_CLASS_1;
    case OP_ADD	    : return ETRACER_INSN_CLASS_1;
    case OP_ADDC    : return ETRACER_INSN_CLASS_1;
    case OP_SUBC    : return ETRACER_INSN_CLASS_1;
    case OP_SUB	    : return ETRACER_INSN_CLASS_1;
    case OP_CMP	    : return ETRACER_INSN_CLASS_1;
    case OP_DADD    : return ETRACER_INSN_CLASS_1;
    case OP_BIT	    : return ETRACER_INSN_CLASS_1;
    case OP_BIC	    : return ETRACER_INSN_CLASS_1;
    case OP_BIS	    : return ETRACER_INSN_CLASS_1;
    case OP_XOR	    : return ETRACER_INSN_CLASS_1;
    case OP_AND	    : return ETRACER_INSN_CLASS_1;

      /**
       * type 2 : single operand
       * not right shifted to avoid collisions with type 3
       **/
    case OP_SXT     : return ETRACER_INSN_CLASS_1;
    case OP_CALL    : return ETRACER_INSN_CLASS_1;
    case OP_RETI    : return ETRACER_INSN_CLASS_1;
    case OP_PUSH    : return ETRACER_INSN_CLASS_1;
    case OP_SWPB    : return ETRACER_INSN_CLASS_1;
    case OP_RRA	    : return ETRACER_INSN_CLASS_1;
    case OP_RRC	    : return ETRACER_INSN_CLASS_1;

      /**
       * type 3 : 2 bits + 2 bits 
       *
       * 0011 11 0 ... 0
       *        | 10 bits
       **/
    case OP_JNZ	    : return ETRACER_INSN_CLASS_1;
    case OP_JZ	    : return ETRACER_INSN_CLASS_1;
    case OP_JNC	    : return ETRACER_INSN_CLASS_1;
    case OP_JC	    : return ETRACER_INSN_CLASS_1;
    case OP_JN	    : return ETRACER_INSN_CLASS_1;
    case OP_JGE	    : return ETRACER_INSN_CLASS_1;
    case OP_JL	    : return ETRACER_INSN_CLASS_1;
    case OP_JMP	    : return ETRACER_INSN_CLASS_1;
    }

  return 0;
}

#else
#define ETRACER_SET_JUMP()       do { } while (0)
#define ETRACER_SET_JUMP_TYPE(x) do { } while (0)
#define ETRACER_BRANCH(x)        do { } while (0)
#define msp430_insn_class(x)     do { } while (0)
#endif

/******************************************************************************************/
/** TYPE 1 DOUBLE OPERANDS ****************************************************************/
/******************************************************************************************/

/**
 * Information used during a type 1 operation (double operands)
 **/
struct msp430_op_type1
{
  uint8_t  as;         /* source addressing bits  */
  uint8_t  ad;         /* dest addressing bits    */

  uint8_t  src_reg;    /* source register         */
  uint8_t  dst_reg;    /* dest register           */

  int16_t  src_val;    /* source value            */
  uint16_t dst_addr;   /* dest address (mode = 1) */

#define CST_MODE 0
#define MEM_MODE 1 
#define REG_MODE 2

  uint8_t  byte;       /* operand size            */
  uint8_t  dst_mode;   /* dest mode 0:reg 1:mem   */


#define SRC_TIMING_0 0
#define SRC_TIMING_1 1
#define SRC_TIMING_2 2
#define SRC_TIMING_3 3

#define DST_TIMING_0 0
#define DST_TIMING_1 1
#define DST_TIMING_2 2

  uint8_t  src_t_mode; /* src cycle timing mode   */
  uint8_t  dst_t_mode; /* dst cycle timing mode   */

};

/**
 * global variable used for type1 operations
 **/
static struct msp430_op_type1 opt1;

#define READ_SRC(byte,offset) ((byte) ? msp430_read_byte( offset ) & 0xff : msp430_read_short( offset ))

/** 
 * opt1 structure decode from instruction 
 **/
static void msp430_type1_double_operands(uint16_t insns)
{
  uint16_t decode_next_pc;
  uint16_t s_offset;

  /*         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0       */
  /* T1     [  op   | sreg  |x|b| as| dreg  ] x=ad */
  
  opt1.dst_reg = (insns >> 0) & 0xf;
  opt1.as      = (insns >> 4) & 0x3;
  opt1.byte    = (insns >> 6) & 0x1;
  opt1.ad      = (insns >> 7) & 0x1;
  opt1.src_reg = (insns >> 8) & 0xf;

#if defined(DEBUG) && defined(DEBUG_TIMING)
  opt1.src_t_mode = 0xffu;
  opt1.dst_t_mode = 0xffu;
#endif

  HW_DMSG_DIS("%s ",msp430_debug_opcode(insns >> 12,opt1.byte));

  decode_next_pc = mcu_get_pc() + 2;

  /*****************/
  /** source      **/
  /*****************/

  switch (opt1.as)
    {
    case 0x0: /* sreg register */
      switch (opt1.src_reg)
	{
	case CG2_REG_IDX:
	  opt1.src_val    = 0x0000;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#0,");
	  break;
	default:
	  opt1.src_val    = MCU_ALU.regs[opt1.src_reg];
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("%s,",mcu_regname_str(opt1.src_reg));
	  break;
	}
      break;

    case 0x1: /* source is indexed on sreg */
      switch (opt1.src_reg)
	{
	case PC_REG_IDX: /* symbolic : [ PC+2 ] + PC */ 
	  s_offset = msp430_read_short( decode_next_pc ) + mcu_get_pc(); // MCU_ALU.regs[opt1.src_reg];
	  opt1.src_val    = msp430_read_short( s_offset );
	  opt1.src_t_mode = SRC_TIMING_3;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("0x%04x,",s_offset & 0xffff);
	  break;
	case CG1_REG_IDX: /* absolute */
	  s_offset = msp430_read_short( decode_next_pc );
 	  opt1.src_val    = READ_SRC( opt1.byte, s_offset );
	  opt1.src_t_mode = SRC_TIMING_3;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("&0x%04x,",s_offset & 0xffff);
	  break;
	case CG2_REG_IDX: /* CG2 == +1 */
	  opt1.src_val    = 0x0001;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#1,");
	  break;
	default:          /* index */
	  {
	    uint16_t off = msp430_read_short( decode_next_pc );
	    s_offset = off + MCU_ALU.regs[opt1.src_reg];
	    HW_DMSG_DIS("%d(%s),", off & 0xffff, mcu_regname_str(opt1.src_reg));
	    opt1.src_val    = READ_SRC(opt1.byte,s_offset);
	    opt1.src_t_mode = SRC_TIMING_3;
	    decode_next_pc += 2;
	  }
	  break;
	}
      break;

    case 0x2: /* source is indirect @Rn */
      switch (opt1.src_reg)
	{
	case CG1_REG_IDX: /* CG1 == +4 */
	  opt1.src_val    = 0x0004;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#4,");
	  break;
	case CG2_REG_IDX: /* CG2 == +2 */
	  opt1.src_val    = 0x0002;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#2,");
	  break;
	default:
	  s_offset = MCU_ALU.regs[opt1.src_reg];
	  opt1.src_val    = READ_SRC(opt1.byte,s_offset);
	  opt1.src_t_mode = SRC_TIMING_1;
	  HW_DMSG_DIS("@%s,",mcu_regname_str(opt1.src_reg));
	  break;
	}
      break;

    case 0x3: /* source is indirect increment */
      switch (opt1.src_reg)
	{
	case PC_REG_IDX: /* [PC + 2] */
	  s_offset        = decode_next_pc; 
	  opt1.src_val    = READ_SRC(opt1.byte,s_offset);
	  opt1.src_t_mode = SRC_TIMING_2;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("#%d,",opt1.src_val);
	  break;

	case CG1_REG_IDX:
	  opt1.src_val    = 0x0008;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#8,");
	  break;

	case CG2_REG_IDX:
	  opt1.src_val    = 0xffff;
	  opt1.src_t_mode = SRC_TIMING_0;
	  HW_DMSG_DIS("#-1,");
	  break;

	default:
	  s_offset        = MCU_ALU.regs[opt1.src_reg];
	  opt1.src_val    = READ_SRC(opt1.byte,s_offset);
	  opt1.src_t_mode = SRC_TIMING_2;
	  MCU_ALU.regs[opt1.src_reg] += opt1.byte ? 1 : 2;
	  HW_DMSG_DIS("@%s+,",mcu_regname_str(opt1.src_reg));
	  break;
	}
      break;
    }
  
  /*****************/
  /** destination **/
  /*****************/

  switch (opt1.ad)
    {
    case 0x0: /* dreg register   */
      opt1.dst_mode   = REG_MODE;
      opt1.dst_t_mode = (opt1.dst_reg == PC_REG_IDX) ? DST_TIMING_0 : DST_TIMING_1;
      /* opt1.dst_reg is already set */
      HW_DMSG_DIS("%s",mcu_regname_str(opt1.dst_reg));
      break;
    case 0x1: /* indexed on dreg */
      opt1.dst_mode   = MEM_MODE;
      switch (opt1.dst_reg)
	{
	case PC_REG_IDX: /* symbolic */ 
	  opt1.dst_addr   = msp430_read_short( decode_next_pc ) + decode_next_pc ; // MCU_ALU.regs[opt1.dst_reg];
	  opt1.dst_t_mode = DST_TIMING_2;
	  HW_DMSG_DIS("0x%04x",opt1.dst_addr & 0xffff);
	  break;
	case SR_REG_IDX: /* absolute */
	  opt1.dst_addr   = msp430_read_short( decode_next_pc );
	  opt1.dst_t_mode = DST_TIMING_2;
	  HW_DMSG_DIS("&0x%04x",opt1.dst_addr & 0xffff);
	  break;
	default:         /* index */
	  opt1.dst_addr   = msp430_read_short( decode_next_pc ) + MCU_ALU.regs[opt1.dst_reg];
	  opt1.dst_t_mode = DST_TIMING_2;
	  HW_DMSG_DIS("%d(%s)",msp430_read_short( decode_next_pc ) & 0xffff,mcu_regname_str(opt1.dst_reg));
	  break;
	}
      decode_next_pc += 2;
      break;
    }

  /***********************/
  /** operands complete **/
  /***********************/

  HW_DMSG_DIS("\n");
  mcu_set_pc_next(decode_next_pc);
#if defined(ETRACE)
  MCU_ALU.sequ_pc = decode_next_pc;
#endif
}

/******************************************************************************************/
/** TYPE 2 SINGLE OPERAND *****************************************************************/
/******************************************************************************************/

/**
 * Information used during a type 2 operation (single operands)
 **/
struct msp430_op_type2
{
  uint8_t  byte;  /* operand size                           */
  uint8_t  ad;    /* adressing bits                         */
  uint8_t  mode;  
  uint8_t  reg;   /* destination/source register            */

  uint16_t addr;  /* destination/source addr if mode == MEM */
  int16_t  val;   /* destination/source value               */

#define OPT2_TIMING_0 0
#define OPT2_TIMING_1 1
#define OPT2_TIMING_2 2
#define OPT2_TIMING_3 3
#define OPT2_TIMING_4 4

  uint8_t  t_mode;/* timing mode                            */

};

/**
 * global variable used for type2 operations
 **/
static struct msp430_op_type2 opt2;

/** 
 * opt2 structure decode from instruction 
 **/
static void msp430_type2_single_operand(uint16_t insns)
{
  uint16_t decode_next_pc;
  /*         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  */
  /* T2     [  op             |b| ad| dsreg ] */

  opt2.reg  = (insns >> 0) & 0xf;
  opt2.ad   = (insns >> 4) & 0x3;
  opt2.byte = (insns >> 6) & 0x1;

#if defined(DEBUG) && defined(DEBUG_TIMING)
  opt2.t_mode = 0xffu;
#endif
  
  HW_DMSG_DIS("%s ",msp430_debug_opcode((insns >> 4) & ~7,opt2.byte));

  decode_next_pc = mcu_get_pc() + 2;

  switch (opt2.ad)
    {
    case 0x0: /* sreg register */
      switch (opt2.reg)
	{
	case CG2_REG_IDX:
	  opt2.mode   = CST_MODE;
	  opt2.val    = 0x0000;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#0");
	  break;
	default:
	  opt2.mode   = REG_MODE;
	  opt2.val    = MCU_ALU.regs[opt2.reg];
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("%s",mcu_regname_str(opt2.reg));
	  break;
	}
      break;

    case 0x1:
      switch (opt2.reg)
	{
	case PC_REG_IDX: /* symbolic */ 
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = msp430_read_short( decode_next_pc ) + decode_next_pc ; // MCU_ALU.regs[opt2.reg];
	  opt2.val    = msp430_read_short( opt2.addr );
	  opt2.t_mode = OPT2_TIMING_4;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("0x%04x",opt2.addr & 0xffff);
	  break;
	case CG1_REG_IDX: /* absolute */
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = msp430_read_short( decode_next_pc );
 	  opt2.val    = READ_SRC(opt2.byte,opt2.addr);
	  opt2.t_mode = OPT2_TIMING_4;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("&0x%04x",opt2.addr & 0xffff);
	  break;
	case CG2_REG_IDX: /* CG2 == +1 */
	  opt2.mode   = CST_MODE;
	  opt2.val    = 0x0001;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#1");
	  break;
	default:          /* index */
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = msp430_read_short( decode_next_pc ) + MCU_ALU.regs[opt2.reg];
 	  opt2.val    = READ_SRC(opt2.byte,opt2.addr);
	  opt2.t_mode = OPT2_TIMING_4;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("%d(%s)",msp430_read_short( decode_next_pc ) & 0xffff,mcu_regname_str(opt2.reg));
	  break;
	}
      break;

    case 0x2:
      switch (opt2.reg)
	{
	case CG1_REG_IDX: /* CG1 == +4 */
	  opt2.mode   = CST_MODE;
	  opt2.val    = 0x0004;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#4");
	  break;
	case CG2_REG_IDX: /* CG2 == +2 */
	  opt2.mode   = CST_MODE;
	  opt2.val    = 0x0002;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#2");
	  break;
	default:  /* indirect */
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = MCU_ALU.regs[opt2.reg];
	  opt2.val    = READ_SRC(opt2.byte,opt2.addr);
	  opt2.t_mode = OPT2_TIMING_1;
	  HW_DMSG_DIS("@%s",mcu_regname_str(opt2.reg));
	  break;
	}
      break;

    case 0x3:
      switch (opt2.reg)
	{
	case PC_REG_IDX: /* immediate */
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = decode_next_pc; // MCU_ALU.regs[opt2.reg];
	  opt2.val    = msp430_read_short( opt2.addr );
	  opt2.t_mode = OPT2_TIMING_3;
	  decode_next_pc += 2;
	  HW_DMSG_DIS("#%d",opt2.val & 0xffff);
	  break;
	case CG1_REG_IDX:
	  opt2.mode   = CST_MODE;
	  opt2.val    = 0x0008;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#8");
	  break;
	case CG2_REG_IDX:
	  opt2.mode   = CST_MODE; 
	  opt2.val    = 0xffff;
	  opt2.t_mode = OPT2_TIMING_0;
	  HW_DMSG_DIS("#-1");
	  break;
	default:  /* indirect autoincrement */
	  opt2.mode   = MEM_MODE;
	  opt2.addr   = MCU_ALU.regs[opt2.reg];
	  opt2.val    = READ_SRC(opt2.byte,opt2.addr);
	  opt2.t_mode = OPT2_TIMING_2;
	  MCU_ALU.regs[opt2.reg] += opt2.byte ? 1 : 2;
	  HW_DMSG_DIS("@%s+",mcu_regname_str(opt2.reg));
	  break;
	}
      break;
    }

  HW_DMSG_DIS("\n");
  mcu_set_pc_next(decode_next_pc);
#if defined(ETRACE)
  MCU_ALU.sequ_pc = decode_next_pc;
#endif
}

/******************************************************************************************/
/** TYPE 3 OFFSET *************************************************************************/
/******************************************************************************************/

/**
 * Information used during a type 3 operation (jump)
 **/

static inline int16_t msp430_type3_offset(short insns)
{
  /*         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  */
  /* T3     [  op |  cc |       offset      ] */
  uint16_t decode_next_pc;
  int16_t  res;

  /* value          */
  res = insns & 0x3ff;
  /* sign extension */
  if (res & 0x200) 
    { 
      res |= ~0x3ff;
    }
  /* shift by one */
  res <<= 1;

  HW_DMSG_DIS("%s $%s%d\n", 
	      msp430_debug_opcode((insns >> 8) & ~3,0), 
	      ((res + 2) > 0) ? "+" : "", res + 2);

  decode_next_pc = mcu_get_pc() + 2;
  mcu_set_pc_next(decode_next_pc);
#if defined(ETRACE)
  MCU_ALU.sequ_pc = decode_next_pc;
#endif
  return res;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

/**
 * Addressing modes
 *
 * 00 Rn    Register mode
 * 01 X(Rn) Indexed mode
 * 01 EDE   Symbolic mode
 * 01 &EDE  Absolute mode
 * 10 @Rn   Indirect mode
 * 11 @Rn+  Indirect autoincrement
 * 11 #N    Immediate mode
 **/

#define SET_CYCLES(n)  msp430_instruction_cycles = n

/**
 * Type 1 opcode timing
 *
 * Destination
 *    Rn PC X(Rn) EDE &EDE 
 *
 * static int opt1_cycles_class[7][5] = 
 *  {                    // source          
 *    { 1, 2, 4, 4, 4 }, // Rn     SRC_TIMING_0 
 *    { 2, 2, 5, 5, 5 }, // @Rn    SRC_TIMING_1 
 *    { 2, 3, 5, 5, 5 }, // @Rn+   SRC_TIMING_2 
 *    { 2, 3, 5, 5, 5 }, // #N     SRC_TIMING_2 
 *    { 3, 3, 6, 6, 6 }, // x(Rn)  SRC_TIMING_3 
 *    { 3, 3, 6, 6, 6 }, // EDE    SRC_TIMING_3 
 *    { 3, 3, 6, 6, 6 }, // &EDE   SRC_TIMING_3 
 *  }; 
 *
 *  source mode timing 
 *  Rn                                   --> SRC_TIMING_0
 *  @Rn                                  --> SRC_TIMING_1
 *  @Rn+, #N are always the same         --> SRC_TIMING_2 
 *  X(Rn), EDE, &EDE are always the same --> SRC_TIMING_3 
 *
 *  destination mode timing
 *  Rn                                   --> DST_TIMING_0 
 *  PC                                   --> DST_TIMING_1 
 *  X(Rn), EDE,&EDE are always the same  --> DST_TIMING_2 
 **/

static int opt1_cycles_class[4][3] = 
  {              // source          
    { 1, 2, 4 }, // SRC_TIMING_0 
    { 2, 2, 5 }, // SRC_TIMING_1 
    { 2, 3, 5 }, // SRC_TIMING_2 
    { 3, 3, 6 }, // SRC_TIMING_3 
  }; 


/**
 * Type 2 opcode timing 
 *
 * 00 Rn    Register mode           --> OPT2_TIMING_0
 * 10 @Rn   Indirect mode           --> OPT2_TIMING_1
 * 11 @Rn+  Indirect autoincrement  --> OPT2_TIMING_2
 * 11 #N    Immediate mode          --> OPT2_TIMING_3
 * 01 X(Rn) Indexed mode            --> OPT2_TINING_4
 * 01 EDE   Symbolic mode
 * 01 &EDE  Absolute mode
 *
 * X(Rn), EDE, &EDE are always the same --> OPT2_TIMING_
 *                                     Rn @Rn @Rn+ #N X(Rn) EDE &EDE 
 * static int opt2_cycles_class1[7] = {  1, 3,  3,  0,  4,   4,   4 }; // OP_RRA, OP_RRC, OP_SWPB, OP_SXT 
 * static int opt2_cycles_class2[7] = {  3, 4,  5,  4,  5,   5,   5 }; // OP_PUSH 
 * static int opt2_cycles_class3[7] = {  4, 4,  5,  5,  5,   5,   5 }; // OP_CALL 
 */

static int opt2_cycles_class1[5] = {  1, 3,  3,  0,  4 }; // OP_RRA, OP_RRC, OP_SWPB, OP_SXT 
static int opt2_cycles_class2[5] = {  3, 4,  5,  4,  5 }; // OP_PUSH 
static int opt2_cycles_class3[5] = {  4, 4,  5,  5,  5 }; // OP_CALL 


/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
#define READ_OPT1_DST ((opt1.dst_mode == REG_MODE) ?                               \
		     (MCU_REGS[opt1.dst_reg]) :                                    \
		     ((opt1.byte == 1) ? msp430_read_byte(opt1.dst_addr) & 0xff : msp430_read_short(opt1.dst_addr)))

static inline void WRITE(int m, int b, int r, uint16_t a, int16_t res)
{
  if (m == REG_MODE)
    {
      if (r == PC_REG_IDX) /* PC */
	{
	  mcu_set_pc_next(res);
	  ETRACER_BRANCH(ETRACER_BRANCH_NONE);
	}
      else if (r == SR_REG_IDX) /* SR */
	{
	  if (RUNNING_MODE() != RUNNING_MODE_FROM_REG(res))
	    {
	      HW_DMSG_LPM("msp430:lpm: writing SR register, LPM changed 0x%x at PC 0x%04x\n",
			  RUNNING_MODE_FROM_REG(res),mcu_get_pc());
	      HW_DMSG_LPM("msp430:lpm:    new mode is (SCG1:%d,SCG0:%d,OSCOFF:%d,CPUOFF:%d)\n",
			  (res>>SHIFT_SCG1)&1,(res>>SHIFT_SCG0)&1,(res>>SHIFT_OSCOFF)&1,(res>>SHIFT_CPUOFF)&1);
	      MCU_CLOCK_SYSTEM_SPEED_TRACER();
	      // scg1:%d scg0:%d oscoff:%d cpuoff:%d\n", MCU_READ_SCG1,MCU_READ_SCG0,MCU_READ_OSCOFF,MCU_READ_CPUOFF);
	      mcu_signal_add(SIG_MCU_LPM_CHANGE);
	    }
	  if ((MCU_ALU.regs[r] & MASK_GIE) != (res & MASK_GIE))
	    {
	      HW_DMSG_GIE("msp430:intr: GIE bit %d -> %d at PC 0x%04x [%"PRIu64"]\n",
			   (MCU_ALU.regs[r] >> SHIFT_GIE)&1,
			   (res >> SHIFT_GIE)&1, mcu_get_pc(),
			   MACHINE_TIME_GET_NANO());
	      TRACER_TRACE_GIE((res >> SHIFT_GIE)&1);
	    }
	}
      MCU_ALU.regs[r] = res;
    }
  else
    {
      if (b==1)
	{
	  msp430_write_byte(a,res);
	}
      else
	{
	  msp430_write_short(a,res);
	}
    }
}

#define WRITE_OPT1 WRITE(opt1.dst_mode,opt1.byte,opt1.dst_reg,opt1.dst_addr,result)
#define WRITE_OPT2 WRITE(opt2.mode,opt2.byte,opt2.reg,opt2.addr,result)

#define BIT_MSB_MASK(b)   (b ? 0x80u : 0x8000u)
#define BIT_CARRY_MASK(b) (b ? 0xffffff00u : 0xffff0000u)
#define BIT_MASK(b)       (b ? 0x000000ffu : 0x0000ffffu)

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static inline void msp430_mcu_update(unsigned int msp430_instruction_cycles)
{
  int time;
#if defined(DEBUG_ME_HARDER)
  uint16_t debug_SR = SR;
  uint16_t debug_IV = MCU_IV;
#endif
  
  /*****************************/
  /* basic clock must be first */
  /*****************************/
  time = MCU_CLOCK_SYSTEM_UPDATE(msp430_instruction_cycles);
  
  MACHINE_TIME_SET_INCR(time);
  //  HW_DMSG_INTR("-");

  
  /********************/
  /* internal devices */
  /********************/
  msp430_watchdog_update();
#if defined(__msp430_have_basic_timer)
  msp430_basic_timer_update();
#endif
#if defined(__msp430_have_timera3)
  msp430_timerA3_update();
#endif
#if defined(__msp430_have_timera5)
  msp430_timerA5_update();
#endif
#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)
  msp430_timerB_update();
#endif

#if defined(__msp430_have_dma)
  msp430_dma_update();
#endif
#if defined(__msp430_have_flash)
  msp430_flash_update();
#endif

  msp430_usart0_update();   
#if defined(__msp430_have_usart1)
  msp430_usart1_update();
#endif
  
#if defined(__msp430_have_lcd)
  msp430_lcd_update();
#endif
  
#if defined(__msp430_have_cmpa)
  msp430_cmpa_update();
#endif
#if defined(__msp430_have_adc12)
  msp430_adc12_update();
#endif
#if defined(__msp430_have_adc10)
  msp430_adc10_update();
#endif
#if defined(__msp430_have_dac12)
  msp430_dac12_update();
#endif

  /***************************/
  /* external devices update */
  /***************************/
  devices_update(); /* update digi IO ports */
  
  /******************************************/
  /* clear update flags on internal devices */
  /******************************************/
  msp430_digiIO_update_done();
  
  MCU_CLOCK_SYSTEM_UPDATE_DONE();
  
  /*********************************/
  /* etrace for eSimu              */
  /*********************************/
  etracer_slot_end(time); /* record current slot, start a new one */

#if defined(DEBUG_ME_HARDER)
  if (((debug_SR & MASK_GIE) == 0) && ((SR & MASK_GIE) == MASK_GIE))
    {
      HW_DMSG_INTR("msp430:intr:debug:update: GIE set back to 1 at PC=0x%04x\n",mcu_get_pc());
      TRACER_TRACE_GIE(1); 
    }
  if (debug_IV != MCU_IV)
    {
      HW_DMSG_INTR("msp430:intr:debug:update: IV set to 0x%04x at PC=0x%04x\n",MCU_IV,mcu_get_pc());
    }
#endif

  MCU_ALU.etracer_except = msp430_interrupt_start_if_any();

  /*********************************/
  /* check for a pending interrupt */
  /*********************************/
  
  MACHINE_TIME_CLR_INCR();
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void msp430_mcu_run_insn()
{
  do 
    {
      uint16_t insn;
      unsigned int opcode;
      unsigned int msp430_instruction_cycles;

#if defined(DEBUG_ME_HARDER)
      uint16_t debug_SR = SR;
#endif

#if defined(ETRACE)
      MCU_ALU.etracer_nseq_flag        = MCU_ALU.next_pc != MCU_ALU.sequ_pc;
      MCU_ALU.etracer_seq_address      = MCU_ALU.sequ_pc;
      MCU_ALU.etracer_branch_type      = 0;
      MCU_ALU.etracer_reti             = 0;
#endif

      /* pc_current is used through the function */
      /* regs[PC] = PC_next, must be done at end of execution to keep Debug correct */
      MCU_ALU.curr_pc = MCU_ALU.regs[PC_REG_IDX] = MCU_ALU.next_pc;

      HW_DMSG_FD("msp430: -- Fetch start - 0x%04x ---------------------------------\n",MCU_ALU.curr_pc);

      /* fetch + decode + execute */
      insn = msp430_fetch_short(MCU_ALU.curr_pc);
      TRACER_TRACE_PC(MCU_ALU.curr_pc);
      if ((mcu_signal_get() & SIG_MAC) != 0)
	{
	  insn = 0x0000;
	  SET_CYCLES(0);
	  HW_DMSG_FD("msp430:alu:  Memory Access Control on fetch at 0x%04x\n",MCU_ALU.curr_pc);
	  HW_DMSG_FD("msp430:alu:  MCU_SIGTRAP 0\n");
	  return;
	}

      opcode = extract_opcode(insn);

      if ((mcu_signal_get() & SIG_MAC) != 0)
	{
	  HW_DMSG_FD("msp430:alu:  Memory Access Control on operand at 0x%04x\n",MCU_ALU.curr_pc);
	}

      switch(opcode)
        {
	  /* ********************************************************************** */
	  /* *** SIGTRAP 0 and 1 ************************************************** */
	  /* ********************************************************************** */

	case 0x0:
	  mcu_signal_add(SIG_MCU | SIG_MCU_TRAP);
	  SET_CYCLES(0);
          switch (insn)
            {
	    case 0:
	      HW_DMSG_FD("msp430:alu:  MCU_SIGTRAP 0\n");
	      break;
	    case 1:
	      HW_DMSG_FD("msp430:alu:  MCU_SIGTRAP 1\n");
	      break;
	    default:
	      ERROR("msp430:alu: MCU_SIGTRAP %d (0x%04x) unknown\n",insn,insn);
	    }
	  return;

	  	  
	  /* ********************************************************************** */
          /* *** SINLE OPERAND OPERATIONS ***************************************** */
	  /* ********************************************************************** */

	  /* flags : VNZC */
	  /* modes : OSCOFF CPUOFF GIE */

        case OP_RRA: 
	  {
	    int16_t result;
	    msp430_type2_single_operand( insn );
	    result = opt2.val;
	    result = (result & BIT_MSB_MASK(opt2.byte)) | ((result>>1) & (opt2.byte ? 0x7f : 0x7fff) );
	    WRITE_OPT2;
	    /* flags : 0*** */
	    CLR_V;
	    WRITE_N(result & BIT_MSB_MASK(opt2.byte));
	    WRITE_Z(result == 0);
	    WRITE_C(opt2.val & 1);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class1[opt2.t_mode]);
	  }
          break;

        case OP_RRC:
	  {
	    int16_t result;
	    msp430_type2_single_operand( insn );
	    if (opt2.byte) /* 8 bits */
	      {
		result  = (((uint8_t)opt2.val) >> 1) | (READ_C << 7);
	      }
	    else
	      {
		result  = (((uint16_t)opt2.val) >> 1) | (READ_C << 15);
	      }
	    WRITE_OPT2;
	    /* flags : **** */
	    WRITE_V(!(opt2.val & BIT_MSB_MASK(opt2.byte)) && (READ_C));
	    WRITE_N(result & BIT_MSB_MASK(opt2.byte));
	    WRITE_Z(result == 0);
	    WRITE_C(opt2.val & 1);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class1[opt2.t_mode]);
	  }
          break;

        case OP_SWPB:
	  {
	    int16_t result;
	    msp430_type2_single_operand( insn );
	    result = ((opt2.val << 8) & 0xff00) | ((opt2.val >> 8) & 0x00ff);
	    WRITE_OPT2;
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class1[opt2.t_mode]);
	  }
          break;

        case OP_SXT: 
	  {
	    int16_t result;
	    msp430_type2_single_operand( insn );
	    result = opt2.val & 0xff;
	    if (result & 0x80) 
	      result |= 0xff00;
	    WRITE_OPT2;
	    /* flags : 0*** */
	    CLR_V;
	    WRITE_N(result & BIT_MSB_MASK(opt2.byte));
	    WRITE_Z(result == 0);
	    WRITE_C(result != 0);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class1[opt2.t_mode]);
	  }
          break;

        case OP_PUSH:
	  {
	    int16_t result;
	    msp430_type2_single_operand( insn );
	    SP -= 2;
	    result = opt2.val;
	    msp430_write_short(SP,result);
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class2[opt2.t_mode]);
	  }
          break;

        case OP_CALL:
	  {
	    uint16_t next_pc;
	    msp430_type2_single_operand( insn );
	    SP -= 2;
	    msp430_write_short(SP,mcu_get_pc_next());
	    next_pc = opt2.val;
	    mcu_set_pc_next(next_pc);
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt2_cycles_class3[opt2.t_mode]);
	    ETRACER_BRANCH(ETRACER_BRANCH_CALL);
	  }
          break;

        case OP_RETI:
	  {
	    // no need msp430_type2_single_operand( insn );
	    uint16_t oldSR = SR;
	    uint16_t next_pc;
	    SR = msp430_read_short(SP);
	    if (RUNNING_MODE() != RUNNING_MODE_FROM_REG(oldSR))
	    {
	      HW_DMSG_LPM("msp430:lpm: changed during stacked SR pop on RETI\n");
	      HW_DMSG_LPM("msp430:lpm:     from (SCG1:%d,SCG0:%d,OSCOFF:%d,CPUOFF:%d)\n",
			  (oldSR>>SHIFT_SCG1)&1,(oldSR>>SHIFT_SCG0)&1,(oldSR>>SHIFT_OSCOFF)&1,(oldSR>>SHIFT_CPUOFF)&1);
	      HW_DMSG_LPM("msp430:lpm:       to (SCG1:%d,SCG0:%d,OSCOFF:%d,CPUOFF:%d)\n",
			  (SR>>SHIFT_SCG1)&1,(SR>>SHIFT_SCG0)&1,(SR>>SHIFT_OSCOFF)&1,(SR>>SHIFT_CPUOFF)&1);
	      mcu_signal_add(SIG_MCU_LPM_CHANGE);
	    }
	    SP += 2;
	    next_pc = msp430_read_short(SP);
	    mcu_set_pc_next(next_pc);
	    SP += 2;
	    HW_DMSG_INTR("msp430:intr: return from interrupt at pc = 0x%04x new pc is 0x%04x [%"PRIu64"]\n",
			 MCU_ALU.curr_pc,next_pc, MACHINE_TIME_GET_NANO());
	    tracer_event_record(TRACER_MCU_INTR,0);
	    /* flags : restored from the stack */
	    /* modes : restored from the stack */
	    /* cycle */
	    SET_CYCLES(5);

	    /* check interrupt that have been raised during the IRQ routine */
	    if (msp430_interrupt_checkifg())
	      {
		HW_DMSG_INTR("msp430:intr: == Return from interrupt with uncleared IFG, setting new interrupt\n");
	      }
	    ETRACER_BRANCH(ETRACER_BRANCH_NONE);
	    MCU_ALU.etracer_reti = 1;
	  }
	  break;

	  /* ********************************************************************** */
          /* *** DOUBLE OPERAND OPERATIONS **************************************** */
	  /* ********************************************************************** */

	case OP_DADD:
	  {
	    int i, c, s, d, b;
	    int16_t result;
	    msp430_type1_double_operands( insn );

	    b = opt1.byte ? 2 : 4;
	    c = READ_C;
	    s = opt1.src_val;
	    d = READ_OPT1_DST;

	    result = 0;
	    for(i=0; i < b; i++ )
	    {
	      short tmp = (s & 0xf) + (d & 0xf) + c;
	      
	      if (tmp >= 10) 
		{
		  tmp -= 10;
		  c = 1;
		}
	      else
		{
		  c = 0;
		}
	      
	      s >>= 4;
	      d >>= 4;
	      result |= tmp << (i*4);
	    }

	    WRITE_OPT1;
	    /* flags : **** */
	    //WRITE_V(); undefined
	    WRITE_N(result & BIT_MSB_MASK(opt1.byte));
	    WRITE_Z(result == 0);
	    WRITE_C(result > (opt1.byte ? 99 : 9999));
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
	  break;

	case OP_MOV: 
	  {
	    int16_t result;
	    msp430_type1_double_operands( insn );
	    result = opt1.src_val;
	    WRITE_OPT1;
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
#if defined(ETRACE)
	    if (opt1.src_reg == opt1.dst_reg)
	      {
		switch (insn)
		  {
		  case 0x4404: /* mov r4,r4 */
		    etracer_start();
		    WARNING("msp430: PC:0x%04x software internal mov 4 : etracer_start()\n",MCU_ALU.curr_pc & 0xffff); 
		    break;
		  case 0x4505: /* mov r5,r5 */
		    etracer_stop();
		    WARNING("msp430: PC:0x%04x software internal mov 5 : etracer_stop()\n",MCU_ALU.curr_pc & 0xffff); 
		    break;
		  case 0x4606: WARNING("msp430: PC:0x%04x software internal mov 6\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4707: WARNING("msp430: PC:0x%04x software internal mov 7\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4808: WARNING("msp430: PC:0x%04x software internal mov 8\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4909: WARNING("msp430: PC:0x%04x software internal mov 9\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4a0a: WARNING("msp430: PC:0x%04x software internal mov A\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4b0b: WARNING("msp430: PC:0x%04x software internal mov B\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4c0c: WARNING("msp430: PC:0x%04x software internal mov C\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4d0d: WARNING("msp430: PC:0x%04x software internal mov D\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4e0e: WARNING("msp430: PC:0x%04x software internal mov E\n",MCU_ALU.curr_pc & 0xffff); break;
		  case 0x4f0f: WARNING("msp430: PC:0x%04x software internal mov F\n",MCU_ALU.curr_pc & 0xffff); break;
		  }
	      }
#endif
	  }
          break;

        case OP_ADD: 
        case OP_ADDC:
	  {
	    int mask,msb,src,dst,result,C,addc,t1,t2;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    addc = ((opcode==OP_ADDC) ? READ_C : 0);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = src + dst + addc ;
	    C = (result & BIT_CARRY_MASK(opt1.byte)) != 0;
	    result = result & mask;

	    WRITE_OPT1;

	    /* flags : **** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    t1  = ((src & msb) == 0) && ((dst & msb) == 0) &&  (result & msb);       /* pos + pos = neg */
	    t2  =  (src & msb)       &&  (dst & msb)       && ((result & msb) == 0); /* neg + neg = pos */
	      
	    WRITE_V(t1 || t2);
	    WRITE_N(result & msb);
	    WRITE_Z(result == 0);
	    WRITE_C(C);

	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_SUB:
        case OP_SUBC:
	  {
	    int mask,msb,src,dst,result,C,subc,t1,t2;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    subc = ((opcode==OP_SUBC) ? READ_C : 1);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = ((~src) & mask) + dst + subc;
	    C = (result & BIT_CARRY_MASK(opt1.byte)) != 0;
	    result  = result & mask;

	    WRITE_OPT1;

	    /* flags : **** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    t1  = ((src & msb) == 0) &&  (dst & msb)       && ((result & msb) == 0); /* neg - pos = pos */
	    t2  =  (src & msb)       && ((dst & msb) == 0) &&  (result & msb);       /* pos - neg = neg */
	      
	    WRITE_V(t1 || t2);
	    WRITE_N(result & msb);
	    WRITE_Z(result == 0);
	    WRITE_C(C);

	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_CMP: /* OP_CMP is the same as SUB except for result storage */
	  {
	    int mask,msb,src,dst,result,C,t1,t2;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = ((~src) & mask) + dst + 1;
	    C = (result & BIT_CARRY_MASK(opt1.byte)) != 0;
	    result = result & mask;
	    
	    /* flags : **** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    t1  = ((src & msb) == 0) &&  (dst & msb)       && ((result & msb) == 0); /* neg - pos = pos */
	    t2  =  (src & msb)       && ((dst & msb) == 0) &&  (result & msb);       /* pos - neg = neg */

	    WRITE_V(t1 || t2);
	    WRITE_N(result &  msb);
	    WRITE_Z(result == 0);
	    WRITE_C(C);

	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_BIC:
	  {
	    int mask,src,dst,result;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = (~src & dst) & mask;

	    WRITE_OPT1;
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_BIS:
	  {
	    int mask,src,dst,result;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = (src | dst) & mask;

	    WRITE_OPT1;
	    /* flags : ---- */
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
	  break;

        case OP_BIT:
	  { 
	    int mask,msb,src,dst,result;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = (src & dst) & mask;

	    /* flags : 0*** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    CLR_V;
	    WRITE_N(result & msb);
	    WRITE_Z(result == 0);
	    WRITE_C(result != 0);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_XOR:
	  {
	    int mask,msb,src,dst,result;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = (src ^ dst) & mask;

	    WRITE_OPT1;
	    /* flags : **** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    WRITE_V((src & msb) && (dst & msb));
	    WRITE_N(result & msb);
	    WRITE_Z(result == 0);
	    WRITE_C(result != 0);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
          break;

        case OP_AND:
	  {
	    int mask,msb,src,dst,result;
	    msp430_type1_double_operands( insn );
	    mask = BIT_MASK(opt1.byte);
	    src  = opt1.src_val  & mask;
	    dst  = READ_OPT1_DST & mask;

	    result = (src & dst) & mask;

	    WRITE_OPT1;
	    /* flags : 0*** */
	    msb = BIT_MSB_MASK(opt1.byte);
	    CLR_V;
	    WRITE_N(result & msb);
	    WRITE_Z(result == 0);
	    WRITE_C(result != 0);
	    /* modes : ---  */
	    /* cycles       */
	    SET_CYCLES(opt1_cycles_class[opt1.src_t_mode][opt1.dst_t_mode]);
	  }
	  break;
	    
	  /* ********************************************************************** */
          /* *** JUMP OPERATIONS ************************************************** */
	  /* ********************************************************************** */

        case OP_JNZ: /* OP_JNE */
	  {
	    int16_t offset = msp430_type3_offset(insn);
	    if (! READ_Z)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JZ: /* OP_JEQ */
	  {
	    int16_t offset = msp430_type3_offset(insn);
	    if (READ_Z)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JC: /* OP_JHS */
          {
	    int16_t offset = msp430_type3_offset(insn);
	    if (READ_C)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JNC: /* OP_JLO */
          {
	    int16_t offset = msp430_type3_offset(insn);
	    if (! READ_C)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
	  break;

        case OP_JN:
          {
	    int16_t offset = msp430_type3_offset(insn);
	    if (READ_N)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JGE:
          {
	    int16_t offset = msp430_type3_offset(insn);
	    if (!(READ_N^READ_V))
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JL:
          {
	    int16_t offset = msp430_type3_offset(insn);
	    if (READ_N^READ_V)
	      {
		uint16_t next_pc = mcu_get_pc_next() + offset;
		mcu_set_pc_next(next_pc);
		ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	      }
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

        case OP_JMP:
	  {
	    /* 
	     * do NOT remove temporary offset variable unless you really
	     * know what you are doing 
	     */
	    int16_t offset   = msp430_type3_offset(insn);
	    uint16_t next_pc = mcu_get_pc_next() + offset;
	    mcu_set_pc_next(next_pc);
	    ETRACER_BRANCH(ETRACER_BRANCH_JUMP);
	    /* flags : ---- */
	    SET_CYCLES(2);
	  }
          break;

	  /* ********************************************************************** */
	  /* ********************************************************************** */
	  /* ********************************************************************** */

        default:
          ERROR("msp430:alu: Unknown opcode 0x%04x at 0x%04x\n", insn, MCU_ALU.curr_pc);
	  SET_CYCLES(0);
	  mcu_signal_add(SIG_MCU_ILL);
	  return;

        } /* case OP_*/

      /* PC = PC_next, must be done at end of execution to keep Debug correct */
      MCU_ALU.regs[PC_REG_IDX] = MCU_ALU.next_pc;

#if defined(DEBUG) && defined(DEBUG_REGISTERS)
      msp430_print_registers();
#endif
      HW_DMSG_FD("msp430: -- Fetch end --------------------------------------------\n");

      /* statistics */
      MCU_INSN_CPT  += 1;
      MCU_CYCLE_CPT += msp430_instruction_cycles;

#if defined(DEBUG_ME_HARDER)
      if (((debug_SR & MASK_GIE) == 0) && ((SR & MASK_GIE) == MASK_GIE))
	{
	  HW_DMSG_INTR("msp430:intr:debug: GIE set back to 1 at PC=0x%04x\n",MCU_ALU.curr_pc);
	  /* GIE */ tracer_event_record(TRACER_MCU_GIE,1);
	}
#endif

#if defined(ETRACE)
      etracer_slot_insn(MCU_ALU.curr_pc, /* WARNING : MSB problem when uin16_t -> uint32_t */
			msp430_insn_class(opcode),
			msp430_instruction_cycles,
			MCU_ALU.etracer_nseq_flag,         /* previous instruction was a jump      */
			MCU_ALU.etracer_seq_address,       /* sequential exec would have been here */
			MCU_ALU.etracer_branch_type,
			MCU_ALU.etracer_except,
			MCU_ALU.etracer_reti);
#endif

      /* device updates */
      msp430_mcu_update(msp430_instruction_cycles);

    }
  while (MCU_ALU.signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static inline void msp430_mcu_run_lpm(void)
{
#define LPM_UPDATE_CYCLES 4
  do { 
    etracer_slot_set_pc( MCU_ALU.curr_pc );
    msp430_mcu_update(LPM_UPDATE_CYCLES);
  } while (MCU_ALU.signal == 0); 
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

/**
 * msp430_mcu_run
 * low power state machine handling
 */
void mcu_run()
{
  uint32_t signal;
  int curr_run_mode;
  int prev_run_mode;

  curr_run_mode = RUNNING_MODE(); 
  MCU_CLOCK_SYSTEM_SPEED_TRACER();

  do {
    /*
     * if (runmode & 1 == 1) then the cpuoff bit is set and MCLK is disabled 
     */
    if ((curr_run_mode & 1) == 0)
      {
	msp430_mcu_run_insn();
      }
    else
      {
	/* VERBOSE(3,"msp430: run LPM mode\n"); */
	msp430_mcu_run_lpm();
      }

    prev_run_mode = curr_run_mode; 
    curr_run_mode = RUNNING_MODE();
    signal        = mcu_signal_get();

    /*
     * update rmode after a couple of instructions
     */
    if ((signal & SIG_MCU_LPM_CHANGE) != 0)
      {
	HW_DMSG_LPM("msp430:lpm: Low power mode [%s] changed to [%s] at [%" PRId64 "]\n",
		    msp430_lpm_names[prev_run_mode],          
		    msp430_lpm_names[curr_run_mode], 
		    MACHINE_TIME_GET_NANO());
	tracer_event_record(TRACER_MCU_POWER, curr_run_mode);
	MCU_CLOCK_SYSTEM_SPEED_TRACER();
	mcu_signal_remove(SIG_MCU_LPM_CHANGE); 
	signal = mcu_signal_get();

	/* we were AM, we are going at least LPM0 */
	if (((prev_run_mode & 1) == 0) && ((curr_run_mode & 1) != 0))
	  {
	    etracer_slot_set_ns();
	  }

      }

  } while (signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

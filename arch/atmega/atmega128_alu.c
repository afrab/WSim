
/**
 *  \file   atmega128_alu.c
 *  \brief  Atmega128 MCU ALU emulation
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "atmega128.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * global variables used in this file
 *
 *
 */ 

#define __DEBUG_ME_HARDER

#define BITS2(w,n) ((w >> n) & 3)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define BIT0_(v)   ((v >>  0) & 1)
#define BIT1_(v)   ((v >>  1) & 1)
#define BIT2_(v)   ((v >>  2) & 1)
#define BIT3_(v)   ((v >>  3) & 1)
#define BIT4_(v)   ((v >>  4) & 1)
#define BIT5_(v)   ((v >>  5) & 1)
#define BIT6_(v)   ((v >>  6) & 1)
#define BIT7_(v)   ((v >>  7) & 1)
#define BIT8_(v)   ((v >>  8) & 1)
#define BIT9_(v)   ((v >>  9) & 1)
#define BIT10_(v)  ((v >> 10) & 1)
#define BIT11_(v)  ((v >> 11) & 1)
#define BIT12_(v)  ((v >> 12) & 1)
#define BIT13_(v)  ((v >> 13) & 1)
#define BIT14_(v)  ((v >> 14) & 1)
#define BIT15_(v)  ((v >> 15) & 1)

/*
#define BIT0n(v)  !((v >> 0) & 1)
#define BIT1n(v)  !((v >> 1) & 1)
#define BIT2n(v)  !((v >> 2) & 1)
#define BIT3n(v)  !((v >> 3) & 1)
#define BIT4n(v)  !((v >> 4) & 1)
#define BIT5n(v)  !((v >> 5) & 1)
#define BIT6n(v)  !((v >> 6) & 1)
#define BIT7n(v)  !((v >> 7) & 1)
*/

#define BIT0n(v)   !BIT0_(v)
#define BIT1n(v)   !BIT1_(v)
#define BIT2n(v)   !BIT2_(v)
#define BIT3n(v)   !BIT3_(v)
#define BIT4n(v)   !BIT4_(v)
#define BIT5n(v)   !BIT5_(v)
#define BIT6n(v)   !BIT6_(v)
#define BIT7n(v)   !BIT7_(v)
#define BIT8n(v)   !BIT8_(v)
#define BIT9n(v)   !BIT9_(v)
#define BIT10n(v)  !BIT10_(v)
#define BIT11n(v)  !BIT11_(v)
#define BIT12n(v)  !BIT12_(v)
#define BIT13n(v)  !BIT13_(v)
#define BIT14n(v)  !BIT14_(v)
#define BIT15n(v)  !BIT15_(v)

/*
 *  SREG: Status Register
 *  C:    Carry Flag
 *  Z:    Zero Flag
 *  N:    Negative Flag
 *  V:    Two's complement overflow indicator
 *  S:    N xor V, For signed tests
 *  H:    Half Carry Flag
 *  T:    Transfer bit used by BLD and BST instructions
 *  I:    Global Interrupt Enable/Disable Flag
 */

#define SREG_I_BIT (1 << 7)
#define SREG_T_BIT (1 << 6)
#define SREG_H_BIT (1 << 5)
#define SREG_S_BIT (1 << 4)
#define SREG_V_BIT (1 << 3)
#define SREG_N_BIT (1 << 2)
#define SREG_Z_BIT (1 << 1)
#define SREG_C_BIT (1 << 0)

#define SET_I  SR |= SREG_I_BIT
#define SET_T  SR |= SREG_T_BIT
#define SET_H  SR |= SREG_H_BIT
#define SET_S  SR |= SREG_S_BIT
#define SET_V  SR |= SREG_V_BIT
#define SET_N  SR |= SREG_N_BIT
#define SET_Z  SR |= SREG_Z_BIT
#define SET_C  SR |= SREG_C_BIT

#define CLR_I  SR &= ~SREG_I_BIT
#define CLR_T  SR &= ~SREG_T_BIT
#define CLR_H  SR &= ~SREG_H_BIT
#define CLR_S  SR &= ~SREG_S_BIT
#define CLR_V  SR &= ~SREG_V_BIT
#define CLR_N  SR &= ~SREG_N_BIT
#define CLR_Z  SR &= ~SREG_Z_BIT
#define CLR_C  SR &= ~SREG_C_BIT

#define WRITE_I(b)   if (b) { SET_I; } else { CLR_I; }
#define WRITE_T(b)   if (b) { SET_T; } else { CLR_T; }
#define WRITE_H(b)   if (b) { SET_H; } else { CLR_H; }
#define WRITE_S(b)   if (b) { SET_S; } else { CLR_S; }
#define WRITE_V(b)   if (b) { SET_V; } else { CLR_V; }
#define WRITE_N(b)   if (b) { SET_N; } else { CLR_N; }
#define WRITE_Z(b)   if (b) { SET_Z; } else { CLR_Z; }
#define WRITE_C(b)   if (b) { SET_C; } else { CLR_C; }

#define READ_I ((uint16_t)((SR >> 7) & 1))
#define READ_T ((uint16_t)((SR >> 6) & 1))
#define READ_H ((uint16_t)((SR >> 5) & 1))
#define READ_S ((uint16_t)((SR >> 4) & 1))
#define READ_V ((uint16_t)((SR >> 3) & 1))
#define READ_N ((uint16_t)((SR >> 2) & 1))
#define READ_Z ((uint16_t)((SR >> 1) & 1))
#define READ_C ((uint16_t)((SR >> 0) & 1))


enum atmega_opcode {
  OP_ADD = 0,
  OP_ADC,
  OP_ADIW,
  OP_AND,
  OP_ANDI,
  OP_ASR,
  OP_BREAK,
  OP_SUB,
  OP_SUBI,
  OP_SBC,
  OP_SBCI,
  OP_SBIW,
  OP_OR,
  OP_ORI,
  OP_EOR,
  OP_COM,
  OP_NEG,
  OP_SBR,
  OP_CBR,
  OP_CLC,
  OP_CLH,
  OP_CLT,
  OP_CLI,
  OP_CLS,
  OP_CLV,
  OP_CLZ,
  OP_IN,
  OP_INC,
  OP_DEC,
  OP_TST,
  OP_CLR, /* == OP_EOR */
  OP_CLN,
  OP_SPM,
  OP_SEH,
  OP_SEN,
  OP_SER,
  OP_SEV,
  OP_SEZ,
  OP_SLEEP,
  OP_MUL,
  OP_MULS,
  OP_MULSU,
  OP_FMUL,
  OP_FMULS,
  OP_FMULSU,
  OP_RJMP,
  OP_IJMP,
  OP_EIJMP,
  OP_ELPM,
  OP_JMP,
  OP_RCALL,
  OP_ICALL,
  OP_EICALL,
  OP_CALL,
  OP_RET,
  OP_RETI,
  OP_CPSE,
  OP_CBI,
  OP_CP,
  OP_CPC,
  OP_CPI,
  OP_ROR,
  OP_SBRC,
  OP_SBRS,
  OP_SBIC,
  OP_SBIS,
  OP_BRBS,
  OP_BRBC,
  OP_BLD,
  OP_BST,
  OP_BREQ,
  OP_BRNE,
  OP_BRCS,
  OP_BRCC,
  OP_BRSH,
  OP_BRLO,
  OP_BRMI,
  OP_BRPL,
  OP_BRGE,
  OP_BRLT,
  OP_BRHS,
  OP_BRHC,
  OP_BRTS,
  OP_BRTC,
  OP_BRVS,
  OP_BRVC,
  OP_BRIE,
  OP_BRID,
  OP_MOV,
  OP_MOVW,
  OP_NOP,
  OP_LDI,
  OP_LDS,
  OP_LD,
  OP_LDD,
  OP_LPM,
  OP_LSR,
  OP_OUT,
  OP_POP,
  OP_PUSH,
  OP_SBI,
  OP_SEC,
  OP_SES,
  OP_SET,
  OP_SEI,
  OP_SR,
  OP_ST,
  OP_STD,
  OP_STS,
  OP_SWAP,
  OP_WDR
};

typedef int (*opcode_fun_t)(uint16_t opcode, uint16_t insn);
static int opcode_default(uint16_t opcode, uint16_t insn);

static int opcode_sleep  (uint16_t opcode, uint16_t insn);

static int opcode_or     (uint16_t opcode, uint16_t insn);
static int opcode_eor    (uint16_t opcode, uint16_t insn);
static int opcode_ori    (uint16_t opcode, uint16_t insn);
static int opcode_sbr    (uint16_t opcode, uint16_t insn);
static int opcode_and    (uint16_t opcode, uint16_t insn);
static int opcode_andi   (uint16_t opcode, uint16_t insn);
static int opcode_cbr    (uint16_t opcode, uint16_t insn);
static int opcode_com    (uint16_t opcode, uint16_t insn);
static int opcode_neg    (uint16_t opcode, uint16_t insn);

static int opcode_add    (uint16_t opcode, uint16_t insn);
static int opcode_adc    (uint16_t opcode, uint16_t insn);
static int opcode_adiw   (uint16_t opcode, uint16_t insn);

static int opcode_sub    (uint16_t opcode, uint16_t insn);
static int opcode_subi   (uint16_t opcode, uint16_t insn);
static int opcode_sbiw   (uint16_t opcode, uint16_t insn);
static int opcode_sbc    (uint16_t opcode, uint16_t insn);
static int opcode_sbci   (uint16_t opcode, uint16_t insn);

static int opcode_inc    (uint16_t opcode, uint16_t insn);
static int opcode_dec    (uint16_t opcode, uint16_t insn);

static int opcode_mul    (uint16_t opcode, uint16_t insn);
static int opcode_muls   (uint16_t opcode, uint16_t insn);
static int opcode_mulsu  (uint16_t opcode, uint16_t insn);

static int opcode_fmul    (uint16_t opcode, uint16_t insn);
static int opcode_fmuls   (uint16_t opcode, uint16_t insn);
static int opcode_fmulsu  (uint16_t opcode, uint16_t insn);

static int opcode_asr   (uint16_t opcode, uint16_t insn);

static int opcode_in     (uint16_t opcode, uint16_t insn);
static int opcode_out    (uint16_t opcode, uint16_t insn);
static int opcode_sbi    (uint16_t opcode, uint16_t insn);

static int opcode_cp     (uint16_t opcode, uint16_t insn);
static int opcode_cpi    (uint16_t opcode, uint16_t insn);
static int opcode_cpc    (uint16_t opcode, uint16_t insn);

static int opcode_sec    (uint16_t opcode, uint16_t insn);
static int opcode_seh    (uint16_t opcode, uint16_t insn);
static int opcode_set    (uint16_t opcode, uint16_t insn);
static int opcode_sei    (uint16_t opcode, uint16_t insn);
static int opcode_ses    (uint16_t opcode, uint16_t insn);
static int opcode_sev    (uint16_t opcode, uint16_t insn);
static int opcode_sez    (uint16_t opcode, uint16_t insn);
static int opcode_sen    (uint16_t opcode, uint16_t insn);

static int opcode_clc    (uint16_t opcode, uint16_t insn);
static int opcode_clh    (uint16_t opcode, uint16_t insn);
static int opcode_clt    (uint16_t opcode, uint16_t insn);
static int opcode_cli    (uint16_t opcode, uint16_t insn);
static int opcode_cls    (uint16_t opcode, uint16_t insn);
static int opcode_clv    (uint16_t opcode, uint16_t insn);
static int opcode_clz    (uint16_t opcode, uint16_t insn);
static int opcode_cln    (uint16_t opcode, uint16_t insn);

static int opcode_ser    (uint16_t opcode, uint16_t insn);

static int opcode_jmp    (uint16_t opcode, uint16_t insn);
static int opcode_rjmp   (uint16_t opcode, uint16_t insn);

static int opcode_call   (uint16_t opcode, uint16_t insn);
static int opcode_ret    (uint16_t opcode, uint16_t insn);

static int opcode_brne   (uint16_t opcode, uint16_t insn);
static int opcode_brcc   (uint16_t opcode, uint16_t insn);
static int opcode_breq   (uint16_t opcode, uint16_t insn);

static int opcode_mov    (uint16_t opcode, uint16_t insn);
static int opcode_movw   (uint16_t opcode, uint16_t insn);

static int opcode_st     (uint16_t opcode, uint16_t insn);
static int opcode_sts    (uint16_t opcode, uint16_t insn);
static int opcode_std    (uint16_t opcode, uint16_t insn);

static int opcode_elpm   (uint16_t opcode, uint16_t insn);
static int opcode_ld     (uint16_t opcode, uint16_t insn);
static int opcode_ldd    (uint16_t opcode, uint16_t insn);
static int opcode_ldi    (uint16_t opcode, uint16_t insn);

static int opcode_pop    (uint16_t opcode, uint16_t insn);
static int opcode_push   (uint16_t opcode, uint16_t insn);

struct atmega_opcode_info_t {
  opcode_fun_t   fun;
  char          *name;
};

struct atmega_opcode_info_t OPCODES[] = {
  { .fun = opcode_add,     .name = "ADD"    }, /* done: needs reviewing */
  { .fun = opcode_adc,     .name = "ADC"    }, /* done: needs reviewing */
  { .fun = opcode_adiw,    .name = "ADIW"   }, /* done: needs reviewing */
  { .fun = opcode_and,     .name = "AND"    },
  { .fun = opcode_andi,    .name = "ANDI"   }, /* done: needs reviewing */
  { .fun = opcode_asr,     .name = "ASR"    }, /* done: needs reviewing */
  { .fun = opcode_default, .name = "BREAK"  },
  { .fun = opcode_sub,     .name = "SUB"    },  /* done: needs reviewing */
  { .fun = opcode_subi,    .name = "SUBI"   },
  { .fun = opcode_sbc,     .name = "SBC"    }, /* done: needs reviewing & check flag Z*/
  { .fun = opcode_sbci,    .name = "SBCI"   }, /* done: needs reviewing */
  { .fun = opcode_sbiw,    .name = "SBIW"   }, /* done: needs reviewing */
  { .fun = opcode_or,      .name = "OR"     }, /* done: needs reviewing */
  { .fun = opcode_ori,     .name = "ORI"    },
  { .fun = opcode_eor,     .name = "EOR"    },
  { .fun = opcode_com,     .name = "COM"    }, /* done: needs reviewing */
  { .fun = opcode_neg,     .name = "NEG"    }, /* done: needs reviewing + check C & V flags */
  { .fun = opcode_sbr,     .name = "SBR"    }, /* Code review & What's the difference between SBR & ORI */
  { .fun = opcode_cbr,     .name = "CBR"    }, /* Check opcode & code review */
  { .fun = opcode_clc,     .name = "CLC"    }, /* done: needs reviewing */
  { .fun = opcode_clh,     .name = "CLH"    }, /* done: needs reviewing */
  { .fun = opcode_clt,     .name = "CLT"    }, /* done: needs reviewing */
  { .fun = opcode_cli,     .name = "CLI"    },
  { .fun = opcode_cls,     .name = "CLS"    }, /* done: needs reviewing */
  { .fun = opcode_clv,     .name = "CLV"    }, /* done: needs reviewing */
  { .fun = opcode_clz,     .name = "CLZ"    }, /* done: needs reviewing */
  { .fun = opcode_in,      .name = "IN"     },
  { .fun = opcode_inc,     .name = "INC"    }, /* done: needs reviewing */
  { .fun = opcode_dec,     .name = "DEC"    }, /* done: needs reviewing */
  { .fun = opcode_default, .name = "TST"    },
  { .fun = opcode_default, .name = "CLR"    }, /* == OP_EOR */
  { .fun = opcode_cln,     .name = "CLN"    }, /* done: needs reviewing */
  { .fun = opcode_default, .name = "SPM"    },
  { .fun = opcode_seh,     .name = "SEH"    }, /* done: needs reviewing */
  { .fun = opcode_sen,     .name = "SEN"    }, /* done: needs reviewing */
  { .fun = opcode_ser,     .name = "SER"    }, /* done: needs reviewing */
  { .fun = opcode_sev,     .name = "SEV"    }, /* done: needs reviewing */
  { .fun = opcode_sez,     .name = "SEZ"    }, /* done: needs reviewing */
  { .fun = opcode_sleep,   .name = "SLEEP"  },
  { .fun = opcode_mul,     .name = "MUL"    }, /* done: needs reviewing */
  { .fun = opcode_muls,    .name = "MULS"   }, /* done: needs reviewing */
  { .fun = opcode_mulsu,   .name = "MULSU"  }, /* done: needs reviewing */
  { .fun = opcode_fmul,    .name = "FMUL"   }, /* done: needs reviewing */
  { .fun = opcode_fmuls,   .name = "FMULS"  }, /* done: needs reviewing */
  { .fun = opcode_fmulsu,  .name = "FMULSU" }, /* done: needs reviewing */
  { .fun = opcode_rjmp,    .name = "RJMP"   },
  { .fun = opcode_default, .name = "IJMP"   },
  { .fun = opcode_default, .name = "EIJMP"  },
  { .fun = opcode_elpm,    .name = "ELPM"   },
  { .fun = opcode_jmp,     .name = "JMP"    },
  { .fun = opcode_default, .name = "RCALL"  },
  { .fun = opcode_default, .name = "ICALL"  },
  { .fun = opcode_default, .name = "EICALL" },
  { .fun = opcode_call,    .name = "CALL"   },
  { .fun = opcode_ret,     .name = "RET"    },
  { .fun = opcode_default, .name = "RETI"   },
  { .fun = opcode_default, .name = "CPSE"   },
  { .fun = opcode_default, .name = "CBI"    },
  { .fun = opcode_cp,      .name = "CP"     },
  { .fun = opcode_cpc,     .name = "CPC"    },
  { .fun = opcode_cpi,     .name = "CPI"    },
  { .fun = opcode_default, .name = "ROR"    },
  { .fun = opcode_default, .name = "SBRC"   },
  { .fun = opcode_default, .name = "SBRS"   },
  { .fun = opcode_default, .name = "SBIC"   },
  { .fun = opcode_default, .name = "SBIS"   },
  { .fun = opcode_default, .name = "BRBS"   },
  { .fun = opcode_default, .name = "BRBC"   },
  { .fun = opcode_default, .name = "BLD"    },
  { .fun = opcode_default, .name = "BST"    },
  { .fun = opcode_breq,    .name = "BREQ"   },
  { .fun = opcode_brne,    .name = "BRNE"   },
  { .fun = opcode_default, .name = "BRCS"   },
  { .fun = opcode_brcc,    .name = "BRCC"   },
  { .fun = opcode_default, .name = "BRSH"   },
  { .fun = opcode_default, .name = "BRLO"   },
  { .fun = opcode_default, .name = "BRMI"   },
  { .fun = opcode_default, .name = "BRPL"   },
  { .fun = opcode_default, .name = "BRGE"   },
  { .fun = opcode_default, .name = "BRLT"   },
  { .fun = opcode_default, .name = "BRHS"   },
  { .fun = opcode_default, .name = "BRHC"   },
  { .fun = opcode_default, .name = "BRTS"   },
  { .fun = opcode_default, .name = "BRTC"   },
  { .fun = opcode_default, .name = "BRVS"   },
  { .fun = opcode_default, .name = "BRVC"   },
  { .fun = opcode_default, .name = "BRIE"   },
  { .fun = opcode_default, .name = "BRID"   },
  { .fun = opcode_mov,     .name = "MOV"    },
  { .fun = opcode_movw,    .name = "MOVW"   },
  { .fun = opcode_default, .name = "NOP"    },
  { .fun = opcode_ldi,     .name = "LDI"    },
  { .fun = opcode_default, .name = "LDS"    },
  { .fun = opcode_ld,      .name = "LD"     }, /* LD X */
  { .fun = opcode_ldd,     .name = "LD"     }, /* LD Y / Z */
  { .fun = opcode_default, .name = "LPM"    },
  { .fun = opcode_default, .name = "LSR"    },
  { .fun = opcode_out,     .name = "OUT"    },
  { .fun = opcode_pop,     .name = "POP"    },
  { .fun = opcode_push,    .name = "PUSH"   },
  { .fun = opcode_sbi,     .name = "SBI"    }, /* done: needs reviewing */
  { .fun = opcode_sec,     .name = "SEC"    }, /* done: needs reviewing */
  { .fun = opcode_ses,     .name = "SES"    }, /* done: needs reviewing */
  { .fun = opcode_set,     .name = "SET"    }, /* done: needs reviewing */
  { .fun = opcode_sei,     .name = "SEI"    },
  { .fun = opcode_default, .name = "SR"     },
  { .fun = opcode_st,      .name = "ST"     },
  { .fun = opcode_std,     .name = "STD"    },
  { .fun = opcode_sts,     .name = "STS"    },
  { .fun = opcode_default, .name = "SWAP"   },
  { .fun = opcode_default, .name = "WDR"    }
};


#define SET_CYCLES(n) do { } while (0) 
#define ADD_TO_PC(k)							\
  do {									\
    uint16_t next_pc;                                                   \
    next_pc = mcu_get_pc() + k;                                         \
    mcu_set_pc_next( next_pc );                                         \
  } while (0) 

static int opcode_default(uint16_t opcode, uint16_t UNUSED insn)
{
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  mcu_signal_set(SIG_MCU_ILL);
  HW_DMSG_DIS("Illegal / unknown opcode\n");
  ERROR("Illegal / unknown opcode\n");
  // PC unchanged
  SET_CYCLES(0);
  return opcode;
}

/* TODO: check flags management */
/**
 * \brief 
 * \param opcode
 * \param insn
 * \return opcode
 */
static int opcode_add(uint16_t opcode, uint16_t insn)
{
  uint8_t  dd, rr;
  int8_t  R, Rd, Rr;
  
  // 0000 11rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  Rd = MCU_REGS[dd];
  Rr = MCU_REGS[rr];
  R = Rd + Rr;
  MCU_REGS[dd] = R;

  WRITE_H(((BIT3_(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3n(R))) | ((BIT3n(R)) & (BIT3_(Rd))));
  WRITE_N(BIT7_(R));
  WRITE_V(((BIT7_(Rd)) & (BIT7_(Rr)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7n(Rr)) & (BIT7_(R))));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);
  WRITE_C(((BIT7_(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7n(R))) | ((BIT7n(R)) & (BIT7_(Rd))));
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: check flags management */
/**
 * \brief 
 * \param opcode
 * \param insn
 * \return opcode
 */
static int opcode_adc(uint16_t opcode, uint16_t insn)
{
  uint8_t  dd, rr;
  int8_t  R, Rd, Rr;
  
  // 0001 11rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  Rd = MCU_REGS[dd];
  Rr = MCU_REGS[rr];
  R = Rd + Rr + READ_C;
  MCU_REGS[dd] = R;
  
  WRITE_H(((BIT3_(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3n(R))) | ((BIT3n(R)) & (BIT3_(Rd))));
  WRITE_N(BIT7_(R));
  WRITE_V((BIT7_(Rd) & BIT7_(Rr) & BIT7n(R)) | (BIT7n(Rd) & BIT7n(Rr) & BIT7_(R)));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);
  WRITE_C(((BIT7_(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7n(R))) | ((BIT7n(R)) & (BIT7_(Rd))));
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_sleep(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0101 1000 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  /* #define IO_REG_MCUCR         0x35 / 0x55 */

  mcu_signal_set(SIG_MCU_ILL);
  HW_DMSG_DIS("Illegal / unknown opcode\n");

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_sei(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0111 1000  
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  // set global interrupt flag
  SET_I;
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_jmp(uint16_t opcode, uint16_t insn)
{
  uint32_t addr;
  uint16_t offset;
  //                A1   A2   B1   B2   C1   C2   D1   D2
  // 0c 94 66 00 : 0000 1100 1001 0100 0110 0110 0000 0000
  //                
  //                B1   B2   A1   A2   D1   D2   C1   C2
  //               1001 0100 0000 1100 0000 0000 0110 0110
  //
  // reference     1001 010k kkkk 110k kkkk kkkk kkkk kkkk
  // jmp 0xcc                                    1100 1100
 
  offset = atmega128_flash_read_short((mcu_get_pc() + 1) << 1);
  addr   = ((insn >> 3) & 0x3e) | (insn & 0x01);
  addr   = (addr << 16) | offset;
  HW_DMSG_DIS("%s 0x%06x [0x%08x]\n",OPCODES[opcode].name, addr << 1, insn << 16 | offset);

  mcu_set_pc_next( addr ); // PC is aligned on words
  SET_CYCLES(3);
  return opcode;
}

static int opcode_eor(uint16_t opcode, uint16_t insn)
{
  uint8_t rd;
  uint8_t rr;
  uint8_t res;
  // 0010 01rd dddd rrrr
  rr = ((insn >> 5) & 0x10) | ((insn >> 0) & 0x0f);
  rd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, rd, rr);

  res = MCU_REGS[rr] ^ MCU_REGS[rd];
  MCU_REGS[rd] = res;

  CLR_V;
  WRITE_N(BIT7_(res));
  WRITE_Z(res == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review */
static int opcode_com(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  int8_t R;

  // 1001 010d dddd 0000
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);

  R = 0xFF - MCU_REGS[dd];
  MCU_REGS[dd] = R;
  
  SET_C;
  CLR_V;
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: check C, V flags & code review */
static int opcode_neg(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t R, Rd;

  // 1001 010d dddd 0001
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);
  
  Rd = MCU_REGS[dd];
  R = 0x00 - Rd;
  MCU_REGS[dd] = R;
  
  WRITE_H((BIT3_(R)) | (BIT3_(Rd)));
  WRITE_C(R != 0);
  WRITE_V(R == 0x80);
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: What's the difference sbr &S ori? */
static int opcode_sbr(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, kk;
  int8_t R;

  // 0110 KKKK dddd KKKK
  kk = ((insn >> 4) & 0xf0) | ((insn >> 0) & 0xf);
  dd = ((insn >> 4) & 0x0f) + 16;
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, kk);
  
  R = MCU_REGS[dd] | kk;
  MCU_REGS[dd] = R;
  
  CLR_V;
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Check opcode */
static int opcode_cbr(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, kk;
  int8_t R;

  // 0111 KKKK dddd KKKK
  dd = ((insn >> 4) & 0x0f) + 16;
  kk  = ((insn >> 4) & 0xf0) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%x\n",OPCODES[opcode].name, dd, kk);
  
  R = MCU_REGS[dd] & (0xff - kk);
  MCU_REGS[dd] = R;
  
  CLR_V;
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_clc(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1000 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_C;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_clh(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1101 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_H;

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_clt(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1110 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_T;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_cli(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1111 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_I;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_cls(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1100 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_S;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_clv(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1011 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_V;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_clz(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1001 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_Z;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_cln(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 1010 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // clear global interrupt flag
  CLR_N;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_ori(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t kk;
  uint8_t res;
  // 0110 KKKK dddd KKKK
  kk = ((insn >> 4) & 0xf0) | ((insn >> 0) & 0xf);
  dd = ((insn >> 4) & 0x0f) + 16;
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, kk);

  res = MCU_REGS[dd] | kk;
  MCU_REGS[dd] = res;
  CLR_V;
  WRITE_N(BIT7_(res));
  WRITE_Z(res == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review */
static int opcode_adiw(uint16_t opcode, uint16_t insn)
{
  uint8_t  K;
  uint8_t  dd;
  uint8_t  Rdh, Rdl;
  uint16_t  R;
 
  // 1001 0110 KKdd KKKK
  /* we need to the immediate value kkkkkk from 00000000 kk00kkkk const 0-63  */
  K = ((insn&0x00C0)>>2)|(insn&0x000F);
  /* we need the register pair's lowest address from 00000000 00dd0000 in r24,r26,r28,r30 */
  dd = ((insn&0x0030)>>3)+24;
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, K);
  
  Rdl = MCU_REGS[dd];
  Rdh = MCU_REGS[dd+1];
  R = (Rdh<<8) + Rdl;
  R+=K;
  MCU_REGS[dd] = (uint8_t)R;
  MCU_REGS[dd+1] = (uint8_t)(R>>8);
  
  WRITE_V((BIT7n(Rdh)) & (BIT15_(R)));
  WRITE_S(READ_N ^ READ_V);
  WRITE_C((BIT15n(R)) & (BIT7_(Rdh)));
  WRITE_N(BIT15_(R));
  WRITE_Z(R == 0);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_and(uint16_t opcode, uint16_t insn)
{
  uint8_t rd;
  uint8_t rr;
  uint8_t res;
  // 0010 00rd dddd rrrr
  rr = ((insn >> 5) & 0x10) | ((insn >> 0) & 0xf);
  rd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, rd, rr);

  res = MCU_REGS[rr] & MCU_REGS[rd];
  MCU_REGS[rd] = res;
  CLR_V;
  WRITE_N(BIT7_(res));
  WRITE_Z(res == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review */
static int opcode_andi(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t K;
  int8_t  R;
  
  // 0111 KKKK dddd KKKK
  dd = ((insn >> 4) & 0x0f) + 16;
  K  = ((insn >> 4) & 0xf0) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%x\n",OPCODES[opcode].name, dd, K);
  
  R = K & MCU_REGS[dd];
  MCU_REGS[dd] = R;
  
  CLR_V;
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_S(READ_N ^ READ_V);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: check flag management */
static int opcode_asr(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  int8_t R;
  
  // 1001 010d dddd 0101
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);
  
    /* set flag C */
  WRITE_C(MCU_REGS[dd]&0x01);
  
  R = MCU_REGS[dd]>>1;
  MCU_REGS[dd] = R;

  WRITE_V(READ_N ^ READ_C);
  WRITE_S(READ_N ^ READ_V);
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_out(uint16_t opcode, uint16_t insn)
{
  uint8_t  rr;
  uint8_t  aa;
  // 1011 1AAr rrrr AAAA
  rr = ((insn >> 4) & 0x1f);
  aa = ((insn >> 5) & 0x30) | ((insn >> 0) & 0xf);
  HW_DMSG_DIS("%s 0x%02x,r%d\n",OPCODES[opcode].name, aa, rr);

  atmega128_io_write_byte(aa,MCU_REGS[rr]);
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_rjmp(uint16_t opcode, uint16_t insn)
{
  int16_t kk;
  // 1100 kkkk kkkk kkkk
  kk = insn & 0x0fff;
  if (kk & 0x0800) /* sign extension */
    kk |= 0xf000;
  HW_DMSG_DIS("%s %s%d\n",OPCODES[opcode].name, (kk < 0) ? ".":".+", kk * 2);

  ADD_TO_PC( kk + 1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_cpi(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t K;
  int8_t  Rd;
  int8_t  R;
  // 0011 KKKK dddd KKKK
  dd = ((insn >> 4) & 0x0f) + 16;
  K  = ((insn >> 4) & 0xf0) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%x\n",OPCODES[opcode].name, dd, K);
  
  Rd = MCU_REGS[dd];
  R  = Rd - K;
  
  WRITE_H(((BIT3n(Rd)) & (BIT3_(K)))  |  ((BIT3_(K)) & (BIT3_(R)))  |  ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_V(((BIT7_(Rd)) & (BIT7n(K)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(K)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_C(((BIT7n(Rd)) & (BIT7_(K))) | ((BIT7_(K)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  WRITE_S(READ_N ^ READ_V);

  atmega128_print_SR();

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_cpc(uint16_t opcode, uint16_t insn)
{
  uint8_t rd, rr;
  int8_t  Rd, Rr;
  int8_t  R, C;
  // 0000 01rd dddd rrrr
  rr = ((insn >> 5) & 0x10) | ((insn >> 0) & 0x0f);
  rd = ((insn >> 4) & 0x10) | ((insn >> 4) & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, rd, rr);
  
  Rd = MCU_REGS[rd];
  Rr = MCU_REGS[rr];
  C  = READ_C;
  R  = Rd - Rr - C;
  // same tests as CP
  WRITE_H(((BIT3n(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3_(R))) | ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_V(((BIT7_(Rd)) & (BIT7n(Rr)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(Rr)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_C(((BIT7n(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  WRITE_S(READ_N ^ READ_V);

  atmega128_print_SR();
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_brne(uint16_t opcode, uint16_t insn)
{
  int8_t K;
  // 1111 01kk kkkk k001
  K  = ((insn >> 3) & 0x7f);
  if (K & 0x40) /* sign extension */
    K |= 0x80;
  HW_DMSG_DIS("%s %s%d\n",OPCODES[opcode].name, (K<0)?".":".+", K * 2);
  if (! READ_Z)
    {
      ADD_TO_PC( K + 1 );
      SET_CYCLES(2);
    }
  else
    {
      ADD_TO_PC(1); // PC is aligned on words
      SET_CYCLES(1);
    }
  return opcode;
}

static int opcode_brcc(uint16_t opcode, uint16_t insn)
{
  int8_t K;
  // 1111 01kk kkkk k000
  K  = ((insn >> 3) & 0x7f);
  if (K & 0x40) /* sign extension */
    K |= 0x80;
  HW_DMSG_DIS("%s %s%d\n",OPCODES[opcode].name, (K<0)?".":".+", K * 2);
  if (READ_C)
    {
      ADD_TO_PC( K + 1 );
      SET_CYCLES(2);
    }
  else
    {
      ADD_TO_PC(1); // PC is aligned on words
      SET_CYCLES(1);
    }
  return opcode;
}

static int opcode_breq(uint16_t opcode, uint16_t insn)
{
  int8_t K;
  // 1111 00kk kkkk k001
  K  = ((insn >> 3) & 0x7f);
  if (K & 0x40) /* sign extension */
    K |= 0x80;
  HW_DMSG_DIS("%s %s%d\n",OPCODES[opcode].name, (K<0)?".":".+", K * 2);
  if (READ_Z)
    {
      ADD_TO_PC(K + 1);
      SET_CYCLES(2);
    }
  else
    {
      ADD_TO_PC(1); // PC is aligned on words
      SET_CYCLES(1);
    }
  return opcode;
}

static int opcode_elpm(uint16_t opcode, uint16_t insn)
{
  uint8_t  dd;
  int8_t   zplus;
  uint32_t addr;
  /* OPT:POSSIBLE_SPLIT_OPCODE */
  // (i)   1001 0101 1101 1000
  // (ii)  1001 000d dddd 0110
  // (iii) 1001 000d dddd 0111
  if (BIT4_(insn))
    {
      dd = 0;
    }
  else
    {
      dd = (insn >> 4) & 0x1f;
    }
  zplus = BIT0_(insn);
  HW_DMSG_DIS("%s r%d,Z%s\n",OPCODES[opcode].name, dd, (zplus)?"+":"");

  addr  = ((RAMPZ & 1) << 16) | ((MCU_REGS[31] << 8) & 0xff00) | (MCU_REGS[30] & 0xff);
  HW_DMSG_DIS("     r30 = 0x%02x | r30 = 0x%02x | addr = 0x%0x\n",MCU_REGS[31], MCU_REGS[30] & 0xff, addr);
  MCU_REGS[dd] = atmega128_flash_read_byte(addr);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(3);
  return opcode;
}

static int opcode_call(uint16_t opcode, uint16_t UNUSED insn)
{
  uint32_t SP;
  uint32_t addr;
  uint16_t offset;
  // 1001 010k kkkk 111k
  // kkkk kkkk kkkk kkkk
  offset = atmega128_flash_read_short((mcu_get_pc() + 1) << 1);
  addr   = ((insn >> 3) & 0x3e) | (insn & 1);
  addr   = (addr << 16) | offset;
  HW_DMSG_DIS("%s 0x%06x [0x%08x]\n",OPCODES[opcode].name, addr << 1, insn << 16 | offset);

  mcu_set_pc_next(addr);
  SP = SP_READ();
  HW_DMSG_DIS("atmega128: PC = 0x%04x\n",mcu_get_pc());
  HW_DMSG_DIS("atmega128: SP = 0x%04x\n",SP);
  atmega128_ram_write_short(SP, mcu_get_pc() + 1); // STACK = PC + 1;

#if defined(ATMEGA_PC_16BITS)
  SP = SP - 2;
  SP_WRITE(SP);
  SET_CYCLES(4); 
#elif defined(ATMEGA_PC_22BITS)
  SP = SP - 3;
  SP_WRITE(SP);
  SET_CYCLES(5);
#else
#error "must define PC size to 16 or 22 bits"
#endif

  return opcode;
}

static int opcode_ret(uint16_t opcode, uint16_t UNUSED insn)
{
  uint32_t SP;
  // 1001 0101 0000 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  SP = SP_READ(); // pre-increment scheme
#if defined(ATMEGA_PC_16BITS)
  SP = SP + 2;
  SP_WRITE(SP);
  SET_CYCLES(4); 
#elif defined(ATMEGA_PC_22BITS)
  SP = SP + 3;
  SP_WRITE(SP);
  SET_CYCLES(5);
#else
#error "must define PC size to 16 or 22 bits"
#endif
  mcu_set_pc_next( atmega128_ram_read_short(SP) );
  return opcode;
}

static int opcode_in(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t aa;
  // 1011 0AAd dddd AAAA
  dd = ((insn >> 4) & 0x1f);
  aa = ((insn >> 5) & 0x30) | ((insn >> 0) & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, aa);

  MCU_REGS[dd] = atmega128_io_read_byte(aa);
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review & check V flag */
static int opcode_inc(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  int8_t R, Rd;
  
  // 1001 010d dddd 0011
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);

  Rd = MCU_REGS[dd];
  R = Rd + 1;
  MCU_REGS[dd] = R;

  WRITE_S(READ_N ^ READ_V);
  WRITE_V(Rd == 0x7f);
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review & check V flag */
static int opcode_dec(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t R, Rd;
  
  // 1001 010d dddd 1010
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);
  
  Rd = MCU_REGS[dd];
  R = Rd - 1;
  MCU_REGS[dd] = R;
  
  WRITE_S(READ_N ^ READ_V);
  WRITE_V(Rd == 0x80);
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_set(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0110 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_T;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_seh(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0101 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_H;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_sen(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0010 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_N;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_ser(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  
  // 1110 1111 dddd 1111
  dd = ((insn >> 4) & 0x0f);
  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, dd);
  
  // set all bits in register Rd
  MCU_REGS[dd] = 0xff;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_sev(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0011 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_V;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_sez(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0001 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_Z;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_mul(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  uint16_t R;
  
  // 1001 11rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  R = MCU_REGS[dd] * MCU_REGS[rr];
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_muls(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int16_t R;
  
  // 0000 0010 dddd rrrr
  dd = ((insn >> 4) & 0x0f);
  rr = (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  R = ((int8_t)MCU_REGS[dd]) * ((int8_t)MCU_REGS[rr]);
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_mulsu(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int16_t R;
  
  // 0000 0011 0ddd 0rrr
  dd = ((insn >> 4) & 0x07) + 16;
  rr = (insn & 0x07) + 16;
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  R = ((int8_t)MCU_REGS[dd]) * MCU_REGS[rr];
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_fmul(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  uint16_t preR, R;
  
  // 0000 0011 0ddd 1rrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  preR = MCU_REGS[dd] * MCU_REGS[rr];
  R = preR << 1;
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(preR));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_fmuls(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int16_t preR, R;
  
  // 0000 0011 0ddd 1rrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  preR = ((int8_t)MCU_REGS[dd]) * ((int8_t)MCU_REGS[rr]);
  R = preR << 1;
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(preR));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode
}

/* TODO: Code review */
static int opcode_fmulsu(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int16_t preR, R;
  
  // 0000 0011 1ddd 1rrr
  dd = ((insn >> 4) & 0x07) + 16;
  rr = (insn & 0x07) + 16;
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  preR = ((int8_t)MCU_REGS[dd]) * MCU_REGS[rr];
  R = preR << 1;
  MCU_REGS[0]=(uint8_t) R; // low byte
  MCU_REGS[1]=(uint8_t) (R>>8); // high byte
  
  WRITE_C(BIT15_(preR));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_mov(uint16_t opcode, uint16_t insn)
{
  uint8_t rr;
  uint8_t dd;
  // 0010 11rd dddd rrrr
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f); 
  dd = ((insn >> 4) & 0x1f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name,dd,rr);
  MCU_REGS[dd] = MCU_REGS[rr];
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_movw(uint16_t opcode, uint16_t insn)
{
  uint8_t rr;
  uint8_t dd;
  // 0000 0001 dddd rrrr
  rr = ((insn >> 0) & 0x0f) << 1;
  dd = ((insn >> 4) & 0x0f) << 1;
  HW_DMSG_DIS("%s r%d:r%d,r%d:r%d\n",OPCODES[opcode].name,dd+1,dd,rr+1,rr);

  MCU_REGS[dd + 1] = MCU_REGS[rr + 1];
  MCU_REGS[dd    ] = MCU_REGS[rr    ];

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_sts(uint16_t opcode, uint16_t insn)
{
  uint8_t  dd;
  uint16_t kk;
  // 1001 001d dddd 0000
  // kkkk kkkk kkkk kkkk
  dd = ((insn >> 4) & 0x1f);
  kk = atmega128_flash_read_short((mcu_get_pc()+1) << 1);
  // check RAMPD if anything goes wrong
  HW_DMSG_DIS("%s 0x%04x,r%d\n",OPCODES[opcode].name,kk,dd);
  atmega128_ram_write_byte(kk,MCU_REGS[dd]);
  ADD_TO_PC(2); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_st(uint16_t opcode, uint16_t insn)
{
  uint16_t X;
  uint8_t  rr;
  int predec, postinc;
  /* OPT:POSSIBLE_SPLIT_OPCODE */
  // (i)   1001 001r rrrr 1100
  // (ii)  1001 001r rrrr 1101
  // (iii) 1001 001r rrrr 1110
  X  = REG_X_READ();
  rr = (insn >> 4) & 0x1f;
  postinc = BIT0_(insn);
  predec  = BIT1_(insn);
  HW_DMSG_DIS("%s %sX%s,r%d\n",OPCODES[opcode].name, predec ? "-":"", postinc ? "+":"", rr);

  if (predec)
    {
      X--;
    }
  atmega128_ram_write_byte(X, MCU_REGS[rr]);
  if (postinc)
    {
      X++;
    }
  REG_X_WRITE(X);
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_std(uint16_t opcode, uint16_t UNUSED insn)
{
  int depl, y;
  uint8_t  rr;
  char chrindex;
  uint16_t Index;

  /* OPT:POSSIBLE_SPLIT_OPCODE */
  // Y register
  // ==========
  // (i)    1000 001r rrrr 1000
  // (ii)   1001 001r rrrr 1001  post increment
  // (iii)  1001 001r rrrr 1010  pre decrement
  // (iiii) 10q0 qq1r rrrr 1qqq  q displacement
  //
  // Z register
  // ==========
  // (i)    1000 001r rrrr 0000
  // (ii)   1001 001r rrrr 0001  post increment
  // (iii)  1001 001r rrrr 0010  pre decrement
  // (iiii) 10q0 qq1r rrrr 0qqq  q displacement
  y    = BIT3_(insn); /* index on Y or Z */
  if (y)
    {
      chrindex = 'Y';
      Index = REG_Y_READ();
    }
  else
    {
      chrindex = 'Z';
      Index = REG_Y_READ();
    }
  rr   = (insn >> 4) & 0x1f;
  depl = BIT12n(insn); /* 0: displacement, 1:pre/post */
  if (depl)
    {
      uint8_t qq = ((insn >> 8) & 0x10) | ((insn >> 7) & 0x18) | (insn & 0x07);
      if (qq == 0) /* STD (i)  */
	{
	  HW_DMSG_DIS("%s %c,r%d\n",OPCODES[opcode].name, chrindex, rr);
	}
      else         /* STD (iv) */
	{
	  HW_DMSG_DIS("%s %c+%d,r%d\n",OPCODES[opcode].name, qq, chrindex, rr);
	}
      atmega128_ram_write_byte(Index + qq, MCU_REGS[rr]);
    }
  else
    {
      int predec, postinc;
      postinc = BIT0_(insn); /* bit12 is set on increment */
      predec  = BIT1_(insn); 
      HW_DMSG_DIS("%s %s%c%s,r%d\n",OPCODES[opcode].name, predec ? "-":"", chrindex, postinc ? "+":"", rr);
      if (predec) 
	{
	  Index --;
	}
      atmega128_ram_write_byte(Index, MCU_REGS[rr]);
      if (postinc) 
	{
	  Index ++;
	}
      if (y) 
	{
	  REG_Y_WRITE(Index);
	} 
      else 
	{
	  REG_Z_WRITE(Index);
	}
    }
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_cp(uint16_t opcode, uint16_t insn)
{
  uint8_t rr, dd;
  int8_t  Rd, Rr;
  int8_t  R;
  // 0001 01rd dddd rrrr
  rr = ((insn >> 5) & 0x10) | ((insn >> 0) & 0x0f);
  dd = ((insn >> 4) & 0x10) | ((insn >> 4) & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);

  Rd = MCU_REGS[dd];
  Rr = MCU_REGS[rr];
  R  = Rd - Rr;
  // same tests as CPC
  WRITE_H(((BIT3n(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3_(R))) | ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_V(((BIT7_(Rd)) & (BIT7n(Rr)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(Rr)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_C(((BIT7n(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  WRITE_S(READ_N ^ READ_V);
  
  atmega128_print_SR();
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review */
static int opcode_sub(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int8_t  R, Rd, Rr;
  
  // 0001 10rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  Rd=MCU_REGS[dd];
  Rr=MCU_REGS[rr];
  R = Rd - Rr;
  MCU_REGS[dd] = R;
  
  WRITE_H(((BIT3n(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3_(R))) | ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_V(((BIT7_(Rd)) & (BIT7n(Rr)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(Rr)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_C(((BIT7n(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  WRITE_S(READ_N ^ READ_V);

  atmega128_print_SR();

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_subi(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  uint8_t K;
  int8_t  Rd;
  int8_t  R;
  // 0101 KKKK dddd KKKK
  dd = ((insn >> 4) & 0x0f) + 16;
  K  = ((insn >> 4) & 0xf0) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, K);
  
  Rd = MCU_REGS[dd];
  R  = Rd - K;
  MCU_REGS[dd] = R;

  WRITE_H(((BIT3n(Rd)) & (BIT3_(K)))  |  ((BIT3_(K)) & (BIT3_(R)))  |  ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_V(((BIT7_(Rd)) & (BIT7n(K)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(K)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  WRITE_C(((BIT7n(Rd)) & (BIT7_(K))) | ((BIT7_(K)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  WRITE_S(READ_N ^ READ_V);

  atmega128_print_SR();

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review & check flag */
static int opcode_sbc(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int8_t Rd, Rr;
  int8_t R;
  
  // 0000 10rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
	
  Rd=MCU_REGS[dd];
  Rr=MCU_REGS[rr];
  
  R = Rd - Rr - READ_C;
  MCU_REGS[dd] = R;
  
  WRITE_H(((BIT3n(Rd)) & (BIT3_(Rr))) | ((BIT3_(Rr)) & (BIT3_(R))) | ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_S(READ_N ^ READ_V);
  WRITE_V(((BIT7_(Rd)) & (BIT7n(Rr)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(Rr)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
  WRITE_C(((BIT7n(Rd)) & (BIT7_(Rr))) | ((BIT7_(Rr)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  if (R) CLR_Z;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: code review & check flags */
static int opcode_sbci(uint16_t opcode, uint16_t insn)
{
  uint8_t dd;
  int8_t  R, Rd;
  uint8_t K;
  
  // 0100 KKKK dddd KKKK
  dd = ((insn >> 4) & 0x0f) + 16;
  K  = ((insn >> 4) & 0xf0) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%x\n",OPCODES[opcode].name, dd, K);
  
  Rd = MCU_REGS[dd];
  R = Rd - K - READ_C;
  MCU_REGS[dd] = R;
  
  WRITE_H(((BIT3n(Rd)) & (BIT3_(K))) | ((BIT3_(K)) & (BIT3_(R))) | ((BIT3_(R)) & (BIT3n(Rd))));
  WRITE_S(READ_N ^ READ_V);
  WRITE_V(((BIT7_(Rd)) & (BIT7n(K)) & (BIT7n(R))) | ((BIT7n(Rd)) & (BIT7_(K)) & (BIT7_(R))));
  WRITE_N(BIT7_(R));
	WRITE_C(((BIT7n(Rd)) & (BIT7_(K))) | ((BIT7_(K)) & (BIT7_(R))) | ((BIT7_(R)) & (BIT7n(Rd))));
  if (R) CLR_Z;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: check flags management */
static int opcode_sbiw(uint16_t opcode, uint16_t insn)
{
  uint8_t  K;
  uint8_t  dd;
  uint8_t   Rdh, Rdl;
  uint16_t  R;
  
  // 1001 0111 KKdd KKKK
  K = ((insn&0x00C0)>>2)|(insn&0x000F);
  dd = ((insn&0x0030)>>3)+24;
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, dd, K);
  
  Rdl = MCU_REGS[dd];
  Rdh = MCU_REGS[dd+1];
  R = (Rdh<<8) + Rdl;
  R-=K;
  MCU_REGS[dd] = (uint8_t)R;
  MCU_REGS[dd+1] = (uint8_t)(R>>8);
  
  WRITE_V(BIT7_(Rdh) & BIT15n(R));
  WRITE_S(READ_N ^ READ_V);
  WRITE_C(BIT15_(R) & BIT7n(Rdh));
  WRITE_N(BIT15_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_or(uint16_t opcode, uint16_t insn)
{
  uint8_t dd, rr;
  int8_t R;
  
  // 0010 10rd dddd rrrr
  dd = ((insn >> 4) & 0x1f);
  rr = ((insn >> 5) & 0x10) | (insn & 0x0f);
  HW_DMSG_DIS("%s r%d,r%d\n",OPCODES[opcode].name, dd, rr);
  
  R = MCU_REGS[dd] | MCU_REGS[rr];
  MCU_REGS[dd] = R;

  WRITE_S(READ_N ^ READ_V);
  CLR_V;
  WRITE_N(BIT7_(R));
  WRITE_Z(R == 0);
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_ldi(uint16_t opcode, uint16_t insn)
{
  uint8_t  rd;
  int8_t   kk;
  // 1110 KKKK dddd KKKK
  rd = ((insn >> 4) & 0x0f) + 16;
  kk = ((insn >> 4) & 0xf0) | ((insn >> 0) & 0x0f);
  HW_DMSG_DIS("%s r%d,0x%02x\n",OPCODES[opcode].name, rd, kk & 0x00ff);

  MCU_REGS[rd] = kk;
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

static int opcode_ld(uint16_t opcode, uint16_t insn)
{
  uint8_t  dd;
  uint16_t Index;
  int predec, postinc;
  // X register
  // ==========
  // (i)   1001 000d dddd 1100 
  // (ii)  1001 000d dddd 1101  postinc
  // (iii) 1001 000d dddd 1110  predec
  Index = REG_X_READ();
  dd   = ((insn >> 4) & 0x1f);
  postinc = BIT0_(insn);
  predec  = BIT1_(insn); 
  HW_DMSG_DIS("%s r%d,%sX%s\n",OPCODES[opcode].name, dd, predec ? "-":"", postinc ? "+":"");

  if (predec) 
    {
      Index --;
    }
  MCU_REGS[dd] = atmega128_ram_read_byte(Index);
  if (postinc) 
    {
      Index ++;
    }
  REG_X_WRITE(Index);
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

static int opcode_ldd(uint16_t opcode, uint16_t insn)
{
  int depl, y;
  uint8_t dd;
  char chrindex;
  uint16_t Index;

  // Y register
  // ==========
  // (i)    1000 000d dddd 1000 
  // (ii)   1001 000d dddd 1001    post increment
  // (iii)  1001 000d dddd 1010    pre decrement
  // (iiii) 10q0 qq0d dddd 1qqq    q displacement
  //
  // Z register
  // ==========
  // (i)    1000 000d dddd 0000
  // (ii)   1001 000d dddd 0001    post increment
  // (iii)  1001 000d dddd 0010    pre decrement
  // (iiii) 10q0 qq0d dddd 0qqq    q displacement
  y    = BIT3_(insn); /* index on Y or Z */
  if (y)
    {
      chrindex = 'Y';
      Index = REG_Y_READ();
    }
  else
    {
      chrindex = 'Z';
      Index = REG_Y_READ();
    }
  dd   = ((insn >> 4) & 0x1f);
  depl = BIT12n(insn);
  if (depl)
    {
        uint8_t qq = ((insn >> 8) & 0x10) | ((insn >> 7) & 0x18) | (insn & 0x07);
      if (qq == 0) /* LD (i)  */
	{
	  HW_DMSG_DIS("%s r%d,%c\n",OPCODES[opcode].name, dd, chrindex);
	}
      else         /* LD (iv) */
	{
	  HW_DMSG_DIS("%s r%d,%c+%d\n",OPCODES[opcode].name, dd, qq, chrindex);
	}
      MCU_REGS[dd] = atmega128_ram_read_byte(Index + qq);
    }
  else
    {
      int predec, postinc;
      postinc = BIT0_(insn); /* bit12 is set on increment */
      predec  = BIT1_(insn); 
      HW_DMSG_DIS("%s r%d,%s%c%s\n",OPCODES[opcode].name, dd, predec ? "-":"", chrindex, postinc ? "+":"");
      if (predec) 
	{
	  Index --;
	}
      MCU_REGS[dd] = atmega128_ram_read_byte(Index);
      if (postinc) 
	{
	  Index ++;
	}
      if (y) 
	{
	  REG_Y_WRITE(Index);
	} 
      else 
	{
	  REG_Z_WRITE(Index);
	}
    }
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}


static int opcode_push(uint16_t opcode, uint16_t insn)
{
  uint8_t  rd;
  uint32_t SP;
  // 1001 001d dddd 1111
  rd = ((insn >> 4) & 0x1f);

  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, rd);

  SP=SP_READ();
  MCU_RAM[ SP ] = MCU_REGS[rd];
  SP=SP - 1;
  SP_WRITE(SP);
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_sbi(uint16_t opcode, uint16_t insn)
{
  uint8_t  aa, bb;
  int8_t   AA;
  
  // 1001 1010 AAAA Abbb
  aa = ((insn >> 3) & 0x1f);
  bb = (insn & 0x07);
  HW_DMSG_DIS("%s 0x%02x,b%d\n",OPCODES[opcode].name, aa, bb);

  AA = atmega128_io_read_byte(aa);
  atmega128_io_write_byte(aa, AA | (1<<bb));

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* TODO: Code review */
static int opcode_sec(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0000 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_C;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}

/* TODO: Code review */
static int opcode_ses(uint16_t opcode, uint16_t UNUSED insn)
{
  // 1001 0100 0100 1000
  HW_DMSG_DIS("%s\n",OPCODES[opcode].name);
  
  // set global interrupt flag
  SET_S;
  
  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(1);
  return opcode;
}
  
static int opcode_pop(uint16_t opcode, uint16_t insn)
{
  uint8_t  rd;
  uint32_t SP;
  // 1001 000d dddd 1111
  rd = ((insn >> 4) & 0x1f);

  HW_DMSG_DIS("%s r%d\n",OPCODES[opcode].name, rd);

  SP = SP_READ();
  MCU_REGS[rd] = MCU_RAM[ SP ] ;
  SP = SP + 1;
  SP_WRITE(SP);

  ADD_TO_PC(1); // PC is aligned on words
  SET_CYCLES(2);
  return opcode;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if 0

LD [page 83]  Load Indirect from Data Space to Register using Index X
(i)    1001 000d dddd 1100    v0+v1
(ii)   1001 000d dddd 1101    v0+v1
(iii)  1001 000d dddd 1110    v0+v1

LDD [page 86]  Load Indirect from Data Space to Register using Index Y
(i)    1000 000d dddd 1000    v0+v1
(ii)   1001 000d dddd 1001    v0+v1
(iii)  1001 000d dddd 1010    v0+v1
(iiii) 10q0 qq0d dddd 1qqq    v0+v1

LDD [page 88]   Load Indirect From Data Space to Register using Index Z
(i)    1000 000d dddd 0000    v0+v1
(ii)   1001 000d dddd 0001    v0+v1
(iii)  1001 000d dddd 0010    v0+v1
(iiii) 10q0 qq0d dddd 0qqq    v0+v1

  -------------------------------------

ST [page 137] Store Indirect From Register to Data Space using Index X
(i)   1001 001r rrrr 1100   - -
(ii)  1001 001r rrrr 1101   - -
(iii) 1001 001r rrrr 1110   - -

STD [page 139]  Store Indirect From Register to Data Space using Index Y
(i)    1000 001r rrrr 1000  - -
(ii)   1001 001r rrrr 1001  - -
(iii)  1001 001r rrrr 1010  - -
(iiii) 10q0 qq1r rrrr 1qqq  - -

STD [page 142]  Store Indirect From Register to Data Space using Index Z
(i)    1000 001r rrrr 0000  - -
(ii)   1001 001r rrrr 0001  - -
(iii)  1001 001r rrrr 0010  - -
(iiii) 10q0 qq1r rrrr 0qqq  - -

#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_set_pc_next(uint16_t x)
{
  MCU_ALU.next_pc = x;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint16_t mcu_get_pc(void)
{
  return MCU_ALU.pc;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint16_t mcu_get_pc_next(void)
{
  return MCU_ALU.next_pc;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_alu_reset(void)
{
  memset(MCU_RAM,0,MAX_RAM_SIZE);

  mcu_set_pc_next( MCU_BOOT_ADDRESS );
  SR = 0;

  MCU_ALU.interrupts = 0;
  MCU_ALU.signal     = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define UNKNOWN_OPCODE(insn)						\
  do {									\
    ERROR("atmega:error: unknown opcode 0x%04x at 0x%04x\n",insn,mcu_get_pc()); \
    mcu_signal_add(SIG_MCU | SIG_MCU_ILL);				\
  } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define RETURN(opcode,insn)                                  \
  do {							     \
    return OPCODES[opcode].fun(opcode,insn);		     \
  } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline unsigned int extract_opcode(uint16_t insn)
{
  unsigned int tmp;

  tmp = (insn >> 12) & 0x0f; // 4 bits
  HW_DMSG_DIS("PC:0x%04x ins:0x%04x  ",(mcu_get_pc() & 0xffff) * 2,insn & 0xffff);

  switch (tmp)
    {
      /* ============================================================ */
      /* == 0000 BCD ================================================ */
      /* ============================================================ */
    case 0x0: 
      switch ((insn >> 10 ) & 0x3)
	{
	case 0x0: /* 0000 00xx CD */
	  switch ((insn >> 8) & 0x3)
	    {
	    case 0x0: /* 0000 0000 CD */
	      if (insn == 0) RETURN(OP_NOP,insn);
	      break;
	    case 0x1: /* 0000 0001 CD */ RETURN(OP_MOVW,insn);
	    case 0x2: /* 0000 0010 CD */ RETURN(OP_MULS,insn);
	    case 0x3: /* 0000 0011 0xxx 1xxx */
	      if      ((((insn >> 7) & 1) == 0) && (((insn >> 3) & 1) == 0))
		{
		  RETURN(OP_MULSU,insn);
		}    
	      else if ((((insn >> 7) & 1) == 0) && (((insn >> 3) & 1) == 1))
		{
		  RETURN(OP_FMUL,insn);
		}     /* 0000 0011 1xxx 0xxx */
	      else if ((((insn >> 7) & 1) == 1) && (((insn >> 3) & 1) == 0))
		{
		  RETURN(OP_FMULS,insn);
		}     /* 0000 0011 1xxx 1xxx */
	      else if ((((insn >> 7) & 1) == 1) && (((insn >> 3) & 1) == 1))
		{
		  RETURN(OP_FMULSU,insn);
		}
	      else
		{
		  UNKNOWN_OPCODE(insn); 
		}
	      break;
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x1: /* 0000 01xx CD */ RETURN(OP_CPC,insn);
	case 0x2: /* 0000 10xx CD */ RETURN(OP_SBC,insn);
	case 0x3: /* 0000 11xx CD */ RETURN(OP_ADD,insn); /* OP_LSL r=d */
	default: UNKNOWN_OPCODE(insn); break;
	}
      break;

      /* ============================================================ */
      /* == 0001 BCD ================================================ */
      /* ============================================================ */
    case 0x1:
      switch ((insn >> 10 ) & 0x3)
	{
	case 0x0: /* 0001 00xx CD */ RETURN(OP_CPSE,insn);
	case 0x1: /* 0001 01xx CD */ RETURN(OP_CP,insn);
	case 0x2: /* 0001 10xx CD */ RETURN(OP_SUB,insn);
	case 0x3: /* 0001 11xx CD */ RETURN(OP_ADC,insn); /* OP_ROL c=d */
	default: UNKNOWN_OPCODE(insn); break;
	}
      break;
      
      /* ============================================================ */
      /* == 0010 BCD ================================================ */
      /* ============================================================ */
    case 0x2: 
      switch ((insn >> 10 ) & 0x3)
	{
	case 0: /* 0010 00xx CD */ RETURN(OP_AND,insn); /* OP_TST if dst == src */
	case 1: /* 0010 01xx CD */ RETURN(OP_EOR,insn); /* OP_CLR if dst == src */
	case 2: /* 0010 10xx CD */ RETURN(OP_OR,insn);
	case 3: /* 0010 11xx CD */ RETURN(OP_MOV,insn);
	default: UNKNOWN_OPCODE(insn); break;
	}
      break;

      /* ============================================================ */
      /* == 0011 BCD ================================================ */
      /* ============================================================ */
    case 0x3: 
      RETURN(OP_CPI,insn);

      /* ============================================================ */
      /* == 0100 BCD ================================================ */
      /* ============================================================ */
    case 0x4:
      RETURN(OP_SBCI,insn);

      /* ============================================================ */
      /* == 0101 BCD ================================================ */
      /* ============================================================ */
    case 0x5:
      RETURN(OP_SUBI,insn);

      /* ============================================================ */
      /* == 0110 BCD ================================================ */
      /* ============================================================ */
    case 0x6: 
      RETURN(OP_ORI,insn);  /* OP_SBR */

      /* ============================================================ */
      /* == 0111 BCD ================================================ */
      /* ============================================================ */
    case 0x7:
      RETURN(OP_ANDI,insn);
      
      /* ============================================================ */
      /* == 1000 BCD ================================================ */
      /* ============================================================ */
    case 0x8:
      switch ((insn >> 8) & 0xf)
	{
	case 0x0: /* 1000 0000 CD */ 
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1000 0000 C 0000 */ RETURN(OP_LDD,insn);  /* LDD Z (i)   v0 */
	    case 0x8: /* 1000 0000 C 1000 */ RETURN(OP_LDD,insn);  /* LDD Y (i)   v0 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x1: /* 1000 0001 CD */ 
	  switch (insn & 0xf) 
	    {
	    case 0x0: /* 1000 0001 C 0000 */ RETURN(OP_LDD,insn);  /* LDD Z (i)   v1 */
	    case 0x8: /* 1000 0001 C 1000 */ RETURN(OP_LDD,insn);  /* LDD Y (i)   v1 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x2:
	  switch (insn & 0xf) 
	    {
	    case 0x0: /* 1000 0010 C 0000 */ RETURN(OP_STD,insn);  /* LDD Z (i)   v1 */
	    case 0x8: /* 1000 0010 C 1000 */ RETURN(OP_STD,insn);  /* LDD Y (i)   v1 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x3:
	  switch (insn & 0xf) 
	    {
	    case 0x0: /* 1000 0010 C 0000 */ RETURN(OP_STD,insn);  /* LDD Z (i)   v1 */
	    case 0x8: /* 1000 0010 C 1000 */ RETURN(OP_STD,insn);  /* LDD Y (i)   v1 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	default: 
	  if (BIT(insn,9) == 0)
	    {
	       if (BIT(insn,3) == 1)
		 RETURN(OP_LDD,insn);  /* LDD Y (iv)   v0 */
	       else
		 RETURN(OP_LDD,insn);  /* LDD Z (iv)   v0 */
	    }
	  else
	    {
	       if (BIT(insn,3) == 1)
		 RETURN(OP_STD,insn);  /* STD Y (iv)   v0 */
	       else
		 RETURN(OP_STD,insn);  /* STD Z (iv)   v0 */
	    }
	  break;
	}
      break;

      /* ============================================================ */
      /* == 1001 BCD ================================================ */
      /* ============================================================ */
    case 0x9: 
      switch ((insn >> 8) & 0xf)
	{
	case 0x0: /* 1001 0000 CD */
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0000 C 0000 */ RETURN(OP_LDS,insn);  /* LDS         v0 */
	    case 0x1: /* 1001 0000 C 0001 */ RETURN(OP_LDD,insn);  /* LDD Z (ii)  v0 */
	    case 0x2: /* 1001 0000 C 0010 */ RETURN(OP_LDD,insn);  /* LDD Z (iii) v0 */
	    case 0x4: /* 1001 0000 C 0100 */ RETURN(OP_LPM,insn);  /* LPM   (ii)  v0 */
	    case 0x5: /* 1001 0000 C 0101 */ RETURN(OP_LPM,insn);  /* LPM   (iii) v0 */
	    case 0x6: /* 1001 0000 C 0110 */ RETURN(OP_ELPM,insn); /* ELPM  (ii)  v0 */
	    case 0x7: /* 1001 0000 C 0111 */ RETURN(OP_ELPM,insn); /* ELPM  (iii) v0 */ 
	    case 0x8: /* 1001 0000 C 1001 */ RETURN(OP_LDD,insn);  /* LDD Y (ii)  v0 */
	    case 0xa: /* 1001 0000 C 1010 */ RETURN(OP_LDD,insn);  /* LDD Y (iii) v0 */
	    case 0xc: /* 1001 0000 C 1100 */ RETURN(OP_LD,insn);   /* LD    (i)   v0 */
	    case 0xd: /* 1001 0000 C 1101 */ RETURN(OP_LD,insn);   /* LD    (ii)  v0 */
	    case 0xe: /* 1001 0000 C 1110 */ RETURN(OP_LD,insn);   /* LD    (iii) v0 */
	    case 0xf: /* 1001 0000 C 1111 */ RETURN(OP_POP,insn);  /* POP         v0 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x1: /* 1001 0001 CD */
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0001 C 0000 */ RETURN(OP_LDS,insn);  /* LDS         v1 */
	    case 0x1: /* 1001 0001 C 0001 */ RETURN(OP_LDD,insn);  /* LDD Z (ii)  v1 */
	    case 0x2: /* 1001 0001 C 0010 */ RETURN(OP_LDD,insn);  /* LDD Z (iii) v1 */
	    case 0x4: /* 1001 0001 C 0100 */ RETURN(OP_LPM,insn);  /* LPM   (ii)  v1 */
	    case 0x5: /* 1001 0001 C 0101 */ RETURN(OP_LPM,insn);  /* LPM   (iii) v1 */
	    case 0x6: /* 1001 0001 C 0110 */ RETURN(OP_ELPM,insn); /* ELPM  (ii)  v1 */
	    case 0x7: /* 1001 0001 C 0111 */ RETURN(OP_ELPM,insn); /* ELPM  (iii) v1 */
	    case 0x8: /* 1001 0001 C 1001 */ RETURN(OP_LDD,insn);  /* LDD Y (ii)  v1 */
	    case 0xa: /* 1001 0001 C 1010 */ RETURN(OP_LDD,insn);  /* LDD Y (iii) v1 */
	    case 0xc: /* 1001 0001 C 1100 */ RETURN(OP_LD,insn);   /* LD    (i)   v1 */
	    case 0xd: /* 1001 0001 C 1101 */ RETURN(OP_LD,insn);   /* LD    (ii)  v1 */
	    case 0xe: /* 1001 0001 C 1110 */ RETURN(OP_LD,insn);   /* LD    (iii) v1 */
	    case 0xf: /* 1001 0001 C 1111 */ RETURN(OP_POP,insn);  /* POP         v1 */
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x2: /* 1001 0010 CD */
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0010 C 0000 */ RETURN(OP_STS,insn);  /* v0 */
	    case 0x1: /* 1001 0010 C 0001 */ RETURN(OP_STD,insn);  /* STD Z */ 
	    case 0x2: /* 1001 0010 C 0010 */ RETURN(OP_STD,insn);  /* STD Z */ 
	    case 0x8: /* 1001 0010 C 1000 */ RETURN(OP_STD,insn);  /* STD Y */ 
	    case 0xa: /* 1001 0010 C 1010 */ RETURN(OP_STD,insn);  /* STD Y */ 
	    case 0xc: /* 1001 0010 C 1100 */ RETURN(OP_ST,insn);   /* v0 */
	    case 0xd: /* 1001 0010 C 1101 */ RETURN(OP_ST,insn);   /* v0 */
	    case 0xe: /* 1001 0010 C 1110 */ RETURN(OP_ST,insn);   /* v0 */
	    case 0xf: /* 1001 001C C 1111 */ RETURN(OP_PUSH,insn); 
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x3: /* 1001 0011 CD */
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0011 C 0000 */ RETURN(OP_STS,insn);  /* v1 */
	    case 0x1: /* 1001 0011 C 0001 */ RETURN(OP_STD,insn);  /* STD Z */ 
	    case 0x2: /* 1001 0011 C 0010 */ RETURN(OP_STD,insn);  /* STD Z */ 
	    case 0x8: /* 1001 0011 C 1000 */ RETURN(OP_STD,insn);  /* STD Y */ 
	    case 0xa: /* 1001 0011 C 1010 */ RETURN(OP_STD,insn);  /* STD Y */ 
	    case 0xc: /* 1001 0011 C 1100 */ RETURN(OP_ST,insn);   /* v1 */
	    case 0xd: /* 1001 0011 C 1101 */ RETURN(OP_ST,insn);   /* v1 */
	    case 0xe: /* 1001 0011 C 1110 */ RETURN(OP_ST,insn);   /* v1 */
	    case 0xf: /* 1001 001C C 1111 */ RETURN(OP_PUSH,insn);
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x4: /* 1001 0100 CD */ 
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0100 C 0000 */ RETURN(OP_COM,insn);
	    case 0x1: /* 1001 0100 C 0001 */ RETURN(OP_NEG,insn);
	    case 0x2: /* 1001 0100 C 0010 */ RETURN(OP_SWAP,insn);
	    case 0x3: /* 1001 0100 C 0011 */ RETURN(OP_INC,insn);
	    case 0x5: /* 1001 0100 C 0101 */ RETURN(OP_ASR,insn);
	    case 0x6: /* 1001 0100 C 0110 */ RETURN(OP_LSR,insn);
	    case 0x7: /* 1001 0100 C 0111 */ RETURN(OP_ROR,insn);
	    case 0x8: /* 1001 0100 C 1000 */ 
	      switch ((insn >> 4) & 0xf)
		{
		  /* ** 0xxx return OP_BSET ** */
		case 0x0: RETURN(OP_SEC,insn);    /* 0000 fall */
		case 0x1: RETURN(OP_SEZ,insn);    /* 0001 fall */
		case 0x2: RETURN(OP_SEN,insn);    /* 0010 fall */
		case 0x3: RETURN(OP_SEV,insn);    /* 0011 fall */
		case 0x4: RETURN(OP_SES,insn);    /* 0100 fall */
		case 0x5: RETURN(OP_SEH,insn);    /* 0101 fall */
		case 0x6: RETURN(OP_SET,insn);    /* 0110 fall */
		case 0x7: RETURN(OP_SEI,insn);    /* 0111 = BSET */
		  
		  /* ** 1xxx RETURN(OP_BCLR,insn); ** */
		case 0x8: RETURN(OP_CLC,insn);    /* 1000 */
		case 0x9: RETURN(OP_CLZ,insn);    /* 1001 */
		case 0xa: RETURN(OP_CLN,insn);    /* 1010 */
		case 0xb: RETURN(OP_CLV,insn);    /* 1011 */
		case 0xc: RETURN(OP_CLS,insn);    /* 1100 */
		case 0xd: RETURN(OP_CLH,insn);    /* 1101 */
		case 0xe: RETURN(OP_CLT,insn);    /* 1110 */
		case 0xf: RETURN(OP_CLI,insn);    /* 1111 */
		}
	      break;
	    case 0x9: /* 1001 0100 C 1001 */
	      switch ((insn >> 4) & 0xf)
		{
		case 0x0: RETURN(OP_IJMP,insn);   /* 0000 */
		case 0x1: RETURN(OP_EIJMP,insn);  /* 0001 */
		default:  UNKNOWN_OPCODE(insn); break;
		}
	      break;
	    case 0xa: /* 1001 0100 C 1010 */ RETURN(OP_DEC,insn);
	    case 0xc: /* 1001 0100 C 1100 */ /* fall */
	    case 0xd: /* 1001 0100 C 1101 */ RETURN(OP_JMP,insn);
	    case 0xe: /* 1001 0100 C 1110 */ /* fall */
	    case 0xf: /* 1001 0100 C 1111 */ RETURN(OP_CALL,insn);
	    default: UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x5: /* 1001 0101 CD */
	  switch (insn & 0xf)
	    {
	    case 0x0: /* 1001 0101 C 0000 */ RETURN(OP_COM,insn);
	    case 0x1: /* 1001 0100 C 0001 */ RETURN(OP_NEG,insn);
	    case 0x2: /* 1001 0100 C 0010 */ RETURN(OP_SWAP,insn);
	    case 0x3: /* 1001 0101 C 0011 */ RETURN(OP_INC,insn);
	    case 0x5: /* 1001 0101 C 0101 */ RETURN(OP_ASR,insn);
	    case 0x6: /* 1001 0101 C 0110 */ RETURN(OP_LSR,insn);
	    case 0x7: /* 1001 0101 C 0111 */ RETURN(OP_ROR,insn);
	    case 0x8: /* 1001 0101 C 1000 */ 
	      switch ((insn >> 4) & 0xf)
		{
		case 0x0: RETURN(OP_RET,insn);    /* 0000 */
		case 0x1: RETURN(OP_RETI,insn);   /* 0001 */
		case 0x8: RETURN(OP_SLEEP,insn);  /* 1000 */
		case 0x9: RETURN(OP_BREAK,insn);  /* 1001 */
		case 0xa: RETURN(OP_WDR,insn);    /* 1010 */
		case 0xc: RETURN(OP_LPM,insn);    /* 1100 */ /* LPM (i)  */
		case 0xd: RETURN(OP_ELPM,insn);   /* 1101 */ /* ELPM (i) */
		case 0xe: RETURN(OP_SPM,insn);    /* 1110 */
		default : UNKNOWN_OPCODE(insn); break;
		}
	      break;
	    case 0x9: /* 1001 0101 C 1001 */ 
	      switch ((insn >> 4) & 0xf)
		{
		case 0x0: RETURN(OP_ICALL,insn);  /* 0000 */
		case 0x1: RETURN(OP_EICALL,insn); /* 0001 */
		default:  UNKNOWN_OPCODE(insn); break;
		}
	      break;
	    case 0xa: /* 1001 0101 C 1010 */ RETURN(OP_DEC,insn);
	    case 0xc: /* 1001 0101 C 1100 */ /* fall */
	    case 0xd: /* 1001 0101 C 1101 */ RETURN(OP_JMP,insn);
	    case 0xe: /* 1001 0101 C 1110 */ /* fall */
	    case 0xf: /* 1001 0101 C 1111 */ RETURN(OP_CALL,insn);
	    default:  UNKNOWN_OPCODE(insn); break;
	    }
	  break;
	case 0x6: /* 1001 0110 CD */ RETURN(OP_ADIW,insn);
	case 0x7: /* 1001 0111 CD */ RETURN(OP_SBIW,insn);
	case 0x8: /* 1001 1000 CD */ RETURN(OP_CBI,insn);
	case 0x9: /* 1001 1001 CD */ RETURN(OP_SBIC,insn);
	case 0xa: /* 1001 1010 CD */ RETURN(OP_SBI,insn);
	case 0xb: /* 1001 1011 CD */ RETURN(OP_SBIS,insn);
	case 0xc: /* 1001 1100 CD */ /* fall */
	case 0xd: /* 1001 1101 CD */ /* fall */
	case 0xe: /* 1001 1110 CD */ /* fall */
	case 0xf: /* 1001 11xx CD */ RETURN(OP_MUL,insn);
	default: UNKNOWN_OPCODE(insn); break;
	}
      break;

      /* ============================================================ */
      /* == 1010 BCD ================================================ */
      /* ============================================================ */
    case 0xa:
      if (BIT(insn,9) == 0)
	{
	  if (BIT(insn,3) == 1)
	    RETURN(OP_LDD,insn);  /* LDD Y (iv)   v1  */
	  else
	    RETURN(OP_LDD,insn);  /* LDD Z (iv)   v1 */
	}
      else
	{
	  if (BIT(insn,3) == 1)
	    RETURN(OP_STD,insn);  /* STD Y (iv)   v1 */
	  else
	    RETURN(OP_STD,insn);  /* STD Z (iv)   v1 */
	}
      break;

      /* ============================================================ */
      /* == 1011 BCD ================================================ */
      /* ============================================================ */
    case 0xb:
      if (((insn >> 11) & 1) == 0)
	{
	  RETURN(OP_IN,insn);
	}
      else
	{
	  RETURN(OP_OUT,insn);
	}
      break;

      /* ============================================================ */
      /* == 1100 BCD ================================================ */
      /* ============================================================ */
    case 0xc:
      RETURN(OP_RJMP,insn);

      /* ============================================================ */
      /* == 1101 BCD ================================================ */
      /* ============================================================ */
    case 0xd:
      RETURN(OP_RCALL,insn);

      /* ============================================================ */
      /* == 1110 BCD ================================================ */
      /* ============================================================ */
    case 0xe:
      RETURN(OP_LDI,insn); /* = OP_SER */

      /* ============================================================ */
      /* == 1111 BCD ================================================ */
      /* ============================================================ */
    case 0xf: 
      switch ((insn >> 9) & 0x7) /* 3b */
	{
	case 0x0: /* 1111 000x CD */ /* fall */
	case 0x1: /* 1111 001x CD */ 
	  switch (insn & 0x7)
	    {
	    case 0x0: RETURN(OP_BRCS,insn); /* BRLO */
	    case 0x1: RETURN(OP_BREQ,insn);
	    case 0x2: RETURN(OP_BRMI,insn);
	    case 0x3: RETURN(OP_BRVS,insn);
	    case 0x4: RETURN(OP_BRLT,insn);
	    case 0x5: RETURN(OP_BRHS,insn);
	    case 0x6: RETURN(OP_BRTS,insn);
	    case 0x7: RETURN(OP_BRIE,insn);
	    default:  RETURN(OP_BRBS,insn);
	    }
	  break;
	case 0x2: /* 1111 010x CD */ /* fall */
	case 0x3: /* 1111 011x CD */ 
	  switch (insn & 0x7)
	    {
	    case 0x0: RETURN(OP_BRCC,insn); /* BRSH */
	    case 0x1: RETURN(OP_BRNE,insn);
	    case 0x2: RETURN(OP_BRPL,insn);
	    case 0x3: RETURN(OP_BRVC,insn);
	    case 0x4: RETURN(OP_BRGE,insn);
	    case 0x5: RETURN(OP_BRHC,insn);
	    case 0x6: RETURN(OP_BRTC,insn);
	    case 0x7: RETURN(OP_BRID,insn);
	    default:  RETURN(OP_BRBC,insn);
	    }
	  break;
	case 0x4: /* 1111 100x CD */ RETURN(OP_BLD,insn);
	case 0x5: /* 1111 101x CD */ 
	  if (BIT(insn,3) == 0)
	    RETURN(OP_BST,insn);
	  else
	    UNKNOWN_OPCODE(insn);
	  break;
	case 0x6: /* 1111 110x CD */
	  if (BIT(insn,3) == 0)
	    RETURN(OP_SBRC,insn);
	  else
	    UNKNOWN_OPCODE(insn);
	  break;
	case 0x7: /* 1111 111x CD */
	  if (BIT(insn,3) == 0)
	    RETURN(OP_SBRS,insn);
	  else
	    UNKNOWN_OPCODE(insn);
	  break;
	default: UNKNOWN_OPCODE(insn); break;
	}
      break;

      /* ============================================================ */
      /* ============================================================ */
      /* ============================================================ */
    default: UNKNOWN_OPCODE(insn); break;
    }
  return 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void atmega128_mcu_update(unsigned int UNUSED cycles)
{
  /* time update from cycles     */
  /* internal devices update (1) */
  /* external devices update     */
  /* internal devices update (2) */
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void atmega128_mcu_run_insn(void)
{
  do 
    {
      uint16_t insn       = 0;
      unsigned int opcode = 0;
      unsigned int cycles = 0;

      MCU_ALU.pc = MCU_ALU.next_pc;
      insn = atmega128_flash_read_short(MCU_ALU.pc << 1);
      opcode = extract_opcode(insn);
      MCU_INSN_CPT  += 1;
#if defined(ETRACE)
      /* eslot */
#endif
      atmega128_mcu_update(cycles);
    }
  while (MCU_ALU.signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

static void atmega128_mcu_run_lpm(void)
{
#define LPM_UPDATE_CYCLES 2
  do {
    atmega128_mcu_update(LPM_UPDATE_CYCLES);
  } while (MCU_ALU.signal == 0);
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void mcu_run(void)
{
  int curr_run_mode;
  int prev_run_mode;

  curr_run_mode = RUNNING_MODE(); 
  //MCU_CLOCK_SYSTEM_SPEED_TRACER(); 

  do {
    if (1==1) /* current run mode == active */ 
      {
	HW_DMSG_FD("----------------------------------------------\n");
	atmega128_mcu_run_insn();
	HW_DMSG_FD("\n");
      }
    else
      {
	atmega128_mcu_run_lpm();
      }

    prev_run_mode = curr_run_mode; 
    curr_run_mode = RUNNING_MODE();

    /*
     * update rmode after a couple of instructions
     */
    if (MCU_ALU.signal & SIG_MCU_LPM_CHANGE)
      {
	HW_DMSG_LPM("atmega128:lpm: Low power mode [%s] changed to [%s] at [%" PRId64 "]\n",
		    atmega128_lpm_names[prev_run_mode],          
		    atmega128_lpm_names[curr_run_mode], 
		    MACHINE_TIME_GET_NANO());
	//TRACER_TRACE_LPM(curr_run_mode);
	//MCU_CLOCK_SYSTEM_SPEED_TRACER();
	MCU_ALU.signal &= ~SIG_MCU_LPM_CHANGE; 
	
	if (((prev_run_mode & 1) == 0) && ((curr_run_mode & 1) != 0))
	  {
	    etracer_slot_set_ns();
	  }
	
      }

  } while (MCU_ALU.signal == 0);

}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

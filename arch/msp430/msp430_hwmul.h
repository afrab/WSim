
/**
 *  \file   msp430_hwmul.h
 *  \brief  MSP430 Hardware Multiplier 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_HWMUL_H
#define MSP430_HWMUL_H

#ifdef __msp430_have_hwmul

#define HWMUL_START    0x130
#define HWMUL_END      0x13e

#define HWMUL_MPY      0x130
#define HWMUL_MPYS     0x132
#define HWMUL_MAC      0x134
#define HWMUL_MACS     0x136

#define HWMUL_OP2      0x138 
#define HWMUL_RESLO    0x13a
#define HWMUL_RESHI    0x13c
#define HWMUL_SUMEXT   0x13e

struct msp430_hwmul_t {
  int16_t op1;
  int16_t op2;
  int16_t sumext;
  int16_t reshi;
  int16_t reslo;

  uint16_t mode; /* last op1 write address */

  union {
    int32_t  s;
    uint32_t u;
  } mult_out;

  union {
    int32_t  s;
    uint32_t u;
  } add_out;
};

void    msp430_hwmul_create  ();
void    msp430_hwmul_reset   ();

int16_t msp430_hwmul_read16  (uint16_t addr);
void    msp430_hwmul_write16 (uint16_t addr, int16_t val);

int8_t  msp430_hwmul_read8   (uint16_t addr);
void    msp430_hwmul_write8  (uint16_t addr, int8_t val);

#else
#define msp430_hwmul_create() do { } while (0)
#endif // __have_hwmul

#endif // MSP430

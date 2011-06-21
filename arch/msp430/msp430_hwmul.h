
/**
 *  \file   msp430_hwmul.h
 *  \brief  MSP430 Hardware Multiplier 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_HWMUL_H
#define MSP430_HWMUL_H

#ifdef __msp430_have_hwmul

struct msp430_hwmul_t {
  int16_t op1;
  int16_t op2;
  int16_t sumext;
  int16_t reshi;
  int16_t reslo;

  uint16_t mode; /* last op1 write address */
};

void    msp430_hwmul_create  ();
void    msp430_hwmul_reset   ();
#define msp430_hwmul_update() do { } while (0)

int16_t msp430_hwmul_read16  (uint16_t addr);
void    msp430_hwmul_write16 (uint16_t addr, int16_t val);

int8_t  msp430_hwmul_read8   (uint16_t addr);
void    msp430_hwmul_write8  (uint16_t addr, int8_t val);

#else
#define msp430_hwmul_create() do { } while (0)
#define msp430_hwmul_reset()  do { } while (0)
#define msp430_hwmul_update() do { } while (0)
#endif // __have_hwmul

#endif // MSP430

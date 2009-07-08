/**
 *  \file   msp430_adc10.h
 *  \brief  MSP430 ADC10 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_ADC10_H
#define MSP430_ADC10_H
#if defined(__msp430_have_adc10)

enum adc10_addr_t {
  ADC10AE   = 0x04A,  /*  8 */
  ADC10CTL0 = 0x01B0, /* 16 */
  ADC10CTL1 = 0x01B2, /* 16 */
  ADC10MEM  = 0x01B4, /* 16 */
  ADC10DTC0 = 0x048,  /*  8 */
  ADC10DTC1 = 0x049,  /*  8 */
  ADC10SA   = 0x01BC  /* 16 */
};

struct msp430_adc10_t {
  
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_adc10_reset ();
void    msp430_adc10_update();
int16_t msp430_adc10_read16 (uint16_t addr);
void    msp430_adc10_write16(uint16_t addr, int16_t val);
int8_t  msp430_adc10_read8  (uint16_t addr);
void    msp430_adc10_write8 (uint16_t addr, int8_t val);
#define msp430_adc10_chkifg()   0

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_adc10 */
#endif

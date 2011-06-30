/**
 *  \file   msp430_adc10.c
 *  \brief  MSP430 Adc10 controller
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

#include <stdlib.h>
#include <string.h>
#include "arch/common/hardware.h"
#include "msp430.h"
#include "src/options.h"

#if defined(__msp430_have_adc10)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DMSG_ADC(x...) HW_DMSG_MCUDEV(x)

#define ADC_DEBUG_LEVEL_2 0

#if ADC_DEBUG_LEVEL_2 != 0
#define HW_DMSG_2_DBG(x...) HW_DMSG_ADC(x)
#else
#define HW_DMSG_2_DBG(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if !defined(ADC10_BASE)
#define ADC10_BASE 0
#endif

enum adc10_addr_t {
  ADC10AE   = ADC10_BASE + 0x004A, /*  8 */
  ADC10DTC0 = ADC10_BASE + 0x0048, /*  8 */
  ADC10DTC1 = ADC10_BASE + 0x0049, /*  8 */
  ADC10CTL0 = ADC10_BASE + 0x01B0, /* 16 */
  ADC10CTL1 = ADC10_BASE + 0x01B2, /* 16 */
  ADC10MEM  = ADC10_BASE + 0x01B4, /* 16 */
  ADC10SA   = ADC10_BASE + 0x01BC  /* 16 */
};

tracer_id_t MSP430_TRACER_ADC10STATE;
tracer_id_t MSP430_TRACER_ADC10INPUT[ADC10_CHANNELS];

#define ADC10_TRACER_STATE(v)   tracer_event_record(MSP430_TRACER_ADC10STATE, v)
#define ADC10_TRACER_INPUT(i,v) tracer_event_record(MSP430_TRACER_ADC10INPUT[i], v)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_create()
{
  msp430_io_register_addr8 (ADC10AE  ,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC0,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC1,msp430_adc10_read8 ,msp430_adc10_write8);

  msp430_io_register_addr16(ADC10CTL0,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10CTL1,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10MEM ,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10SA  ,msp430_adc10_read16,msp430_adc10_write16);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_reset()
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_update()
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc10_read16 (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write16 (uint16_t addr, int16_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%04x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc10_read8  (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write8 (uint16_t addr, int8_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%02x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

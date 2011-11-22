/**
 *  \file   msp430_portmap.h
 *  \brief  MSP430 Portmap dummy(!) definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_PORTMAP_H
#define MSP430_PORTMAP_H

#if defined(__msp430_have_portmap)
#define PORTMAP_IOMEM_BEGIN     PORTMAP_BASE
#define PORTMAP_IOMEM_END       (PORTMAP_BASE + 0x1f)

/**
 * Portmap Data Structure
 **/
struct msp430_portmap_t {
  int tmp;
};

void msp430_pormtap_create();
int msp430_portmap_reset();
void msp430_portmap_update();
int8_t msp430_portmap_read8(uint16_t addr);
int16_t msp430_portmap_read(uint16_t addr);
void msp430_portmap_write8(uint16_t addr, int8_t val);
void msp430_portmap_write(uint16_t addr, int16_t val);

#else
#define msp430_portmap_create() do { } while (0)
#define msp430_portmap_reset() do { } while (0)
#define msp430_portmap_update() do { } while (0)
#endif
#endif

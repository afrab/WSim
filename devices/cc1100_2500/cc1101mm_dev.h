
/**
 *  \file   cc1101mm_dev.h
 *  \brief  MSP430 CC1101 interface definition for CC430
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_CC1101_H
#define MSP430_CC1101_H
//#include "arch/common/hardware.h" // included in hardware.h
//#include "arch/msp430/msp430.h"

#if defined(CC1101MM)
// include original cc1100 structure
#include "cc1100_2500_internals.h"

#define CC1101_IOMEM_OFFSET     0x0F00
#define CC1101_IOMEM_END        CC1101_IOMEM_OFFSET + 0x3f

#define RF1AIFCTL0          CC1101_IOMEM_OFFSET
#define RF1AIFCTL1          CC1101_IOMEM_OFFSET + 0x02
#define RF1AIFERR           CC1101_IOMEM_OFFSET + 0x06
#define RF1AIFERRV          CC1101_IOMEM_OFFSET + 0x0c
#define RF1AIFIV            CC1101_IOMEM_OFFSET + 0x0e
#define RF1AINSTRW          CC1101_IOMEM_OFFSET + 0x10
#define RF1AINSTR1W         CC1101_IOMEM_OFFSET + 0x12
#define RF1AINSTR2W         CC1101_IOMEM_OFFSET + 0x14
#define RF1ADINW            CC1101_IOMEM_OFFSET + 0x16
#define RF1ASTATW           CC1101_IOMEM_OFFSET + 0x20
#define RF1ASTAT1W          CC1101_IOMEM_OFFSET + 0x22
#define RF1ASTAT2W          CC1101_IOMEM_OFFSET + 0x24
#define RF1ADOUTW           CC1101_IOMEM_OFFSET + 0x28
#define RF1ADOUT1W          CC1101_IOMEM_OFFSET + 0x2a
#define RF1ADOUT2W          CC1101_IOMEM_OFFSET + 0x2c
#define RF1AIN              CC1101_IOMEM_OFFSET + 0x30
#define RF1AIFG             CC1101_IOMEM_OFFSET + 0x32
#define RF1AIES             CC1101_IOMEM_OFFSET + 0x34
#define RF1AIE              CC1101_IOMEM_OFFSET + 0x36
#define RF1AIV              CC1101_IOMEM_OFFSET + 0x38


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) rf1aifctl0_t
{
  uint16_t
    reserved : 14,
    rfendian : 1,
    reserved2 : 1;
};
#else
struct __attribute__ ((packed)) rf1aifctl0_t
{
  uint16_t
    reserved2 : 1,
    rfendian : 1,
    eserved : 14;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) rf1aifctl1_t
{
  uint16_t
    rfdoutie : 1,
    rfstatie : 1,
    rfdinie : 1,
    rfinstrie : 1,
    unused : 1,
    rferrie : 1,
    unused1 : 2,
    rfdoutifg : 1,
    rfstatifg : 1,
    rfdinifg : 1,
    rfinstrifg : 1,
    unused2 : 1,
    rferrifg : 1,
    unused3 : 2;
};
#else
struct __attribute__ ((packed)) rf1aifctl1_t
{
  uint16_t
    unused3: 2,
    rferrifg:1,
    unused2:1,
    rfinstrifg:1,
    rfdinifg:1,
    rfstatifg:1,
    rfdoutifg:1,
    unused1:2,
    rferrie:1,
    unused:1,
    rfinstrie:1,
    rfdinie:1,
    rfstatie:1,
    rfdoutie:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) rf1aiferr_t
{
  uint16_t
    unused:12,
    opoverr:1,
    outerr:1,
    operr:1,
    lverr:1;
};
#else
struct __attribute__ ((packed)) rf1aiferr_t
{
  uint16_t
    lverr:1,
    operr:1,
    outerr:1,
    opoverr:1,
    unused:12;
};
#endif

/**
 * Statemachine Structure
 */
struct _cc1101mm_statemachine_t {
  uint8_t instruction;
  uint8_t data;
  uint8_t parameter;
  uint8_t state;
  uint8_t rxfifo;
};

/**
 * CC1101 Data Structure
 **/
struct _msp430_cc1101_t
{
  /* state */
  struct _cc1101mm_statemachine_t statemachine;

  /* registers */
  union {
    struct rf1aifctl0_t  b;
    uint16_t             s;
  } rf1aifctl0;
  union {
    struct rf1aifctl1_t  b;
    uint16_t             s;
  } rf1aifctl1;
  union {
    struct rf1aiferr_t  b;
    uint16_t             s;
  } rf1aiferr;
  uint16_t rf1aiferrv;
  uint16_t rf1aifiv;
  uint16_t rf1ainstrw;
  uint8_t rf1ainstr1b;
  uint8_t rf1ainstr2b;
  uint16_t rf1adinw;
  uint16_t rf1astatw;
  uint16_t rf1astat1w; //auto read, do we need duplication?
  uint16_t rf1astat2w;
  uint16_t rf1adoutw;
  uint16_t rf1adout1w;
  uint16_t rf1adout2w;
  uint16_t rf1ain;
  uint16_t rf1aifg;
  uint16_t rf1aies;
  uint16_t rf1aie;
  uint16_t rf1aiv;
  // full cc1100 structure used by common code
  struct _cc1100_t *cc1100;
  struct _cc1100_t cc1100_struct;
  
  /* for the ifgs */
  uint8_t last_GO0_pin;
  uint8_t last_GO1_pin;
  uint8_t last_GO2_pin;
  
  uint8_t send_intr;

}; //TODO make this better

/**
 * Mnemonic Bitmasks
 */
#define CC1101MM_SRES           0x30
#define CC1101MM_SFSTXON        0x31
#define CC1101MM_SXOFF          0x32
#define CC1101MM_SCAL           0x33
#define CC1101MM_SRX            0x34
#define CC1101MM_STX            0x35
#define CC1101MM_SIDLE          0x36
#define CC1101MM_SWOR           0x38
#define CC1101MM_SPWD           0x39
#define CC1101MM_SFRX           0x3A
#define CC1101MM_SFTX           0x3B
#define CC1101MM_SWORRST        0x3C
#define CC1101MM_SNOP           0x3D
#define CC1101MM_SNGLREGRD      0x80
#define CC1101MM_SNGLREGWR      0x00
#define CC1101MM_REGRD          0xC0
#define CC1101MM_REGWR          0x40
#define CC1101MM_STATREGRD      0xC1
#define CC1101MM_SNGLPATABRD    0xBE
#define CC1101MM_SNGLPATABWR    0x3E
#define CC1101MM_PATABRD        0xFE
#define CC1101MM_PATABWR        0x7E
#define CC1101MM_SNGLRXRD       0xBF
#define CC1101MM_SNGLTXWR       0x3F
#define CC1101MM_RXFIFORD       0xFF
#define CC1101MM_TXFIFOWR       0x7F

/**
 * Statemachine for instructions
 */
#define CC1101MM_WAIT_INSTR         0
#define CC1101MM_WAIT_DATA          1
#define CC1101MM_WAIT_INSTR_DATA    2

void cc1101mm_create (void);
int cc1101mm_reset (void);
void cc1101mm_update (void);
int8_t cc1101mm_read8 (uint16_t addr);
int16_t cc1101mm_read16 (uint16_t addr);
void cc1101mm_write8 (uint16_t addr, int8_t val);
void cc1101mm_write16 (uint16_t addr, int16_t val);
int cc1101mm_chkifg (void);
void cc1101mm_assert_gdo(uint8_t event, int assert);

#else
#define cc1101mm_create() do { } while (0)
#define cc1101mm_reset() do { } while (0)
#define cc1101mm_update() do { } while (0)
#endif
#endif

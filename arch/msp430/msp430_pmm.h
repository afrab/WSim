/**
 *  \file   msp430_pmm.h
 *  \brief  MSP430 PMM definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_PMM_H
#define MSP430_PMM_H

#if defined(__msp430_have_pmm)

#define PMM_IOMEM_BEGIN     PMM_BASE
#define PMM_IOMEM_END       (PMM_BASE + 0x11)

#define PMMCTL0   PMM_BASE
#define PMMCTL1   (PMM_BASE + 0x02)
#define SVSMHCTL  (PMM_BASE + 0x04)
#define SVSMLCTL  (PMM_BASE + 0x06)
/*#define SVSMIO    (PMM_BASE + 0x08) 
 * defined in cc430 UG, but not in the cc430f613x datasheet*/
#define PMMIFG    (PMM_BASE + 0x0c) /* different offset in cc430f613x datasheet then in cc430 UG */
#define PMMRIE    (PMM_BASE + 0x0e)
#define PM5CTL0   (PMM_BASE + 0x10)

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) pmmctl0_t
{
  uint16_t
  pmmpw:8,
    pmmhpmre:1,
    reserved:2,
    pmmregoff:1,
    pmmswpor:1,
    pmmswbor:1,
    pmmcorev:2;
};
#else

struct __attribute__((packed)) pmmctl0_t
{
  uint16_t
  pmmcorev:2,
    pmmswbor:1,
    pmmswpor:1,
    pmmregoff:1,
    reserved:2,
    pmmhpmre:1,
    pmmpw:8;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) pmmctl1_t
{
  uint16_t
  reserved0:8,
    reserved1:2,
    reserved2:2,
    reserved3:2,
    reserved4:1,
    reserved5:1;
};
#else

struct __attribute__((packed)) pmmctl1_t
{
  uint16_t
  reserved5:1,
    reserved4:1,
    reserved3:2,
    reserved2:2,
    reserved1:2,
    reserverd0:8;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) svsmhctl_t
{
  uint16_t
  svmhfp:1,
    svmhe:1,
    reserved0:1,
    svmhovpe:1,
    svshfp:1,
    svshe:1,
    svshrvl:2,
    svsmhace:1,
    svsmhevm:1,
    reserved1:1,
    svshmd:1,
    svsmhdlyst:1,
    svsmhrrl:3;
};
#else

struct __attribute__((packed)) svsmhctl_t
{
  uint16_t
  svsmhrrl:3,
    svsmhdlyst:1,
    svshmd:1,
    reserved1:1,
    svsmhevm:1,
    svsmhace:1,
    svshrvl:2,
    svshe:1,
    svshfp:1,
    svmhovpe:1,
    reserved0:1,
    svmhe:1,
    svmhfp:1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) svsmlctl_t
{
  uint16_t
  svmlfp:1,
    svmle:1,
    reserved0:1,
    svmlovpe:1,
    svslfp:1,
    svsle:1,
    svslrvl:2,
    svsmlace:1,
    svsmlevm:1,
    reserved1:1,
    svslmd:1,
    svsmldlyst:1,
    svsmlrrl:3;
};
#else

struct __attribute__((packed)) svsmlctl_t
{
  uint16_t
  svsmlrrl:3,
    svsmldlyst:1,
    svslmd:1,
    reserved1:1,
    svsmlevm:1,
    svsmlace:1,
    svslrvl:2,
    svsle:1,
    svslfp:1,
    svmlovpe:1,
    reserved0:1,
    svmle:1,
    svmlfp:1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) svsmio_t
{
  uint16_t
  reserved0:3,
    svmhvlroe:1,
    svmhoe:1,
    reserved1:5,
    svmoutpol:1,
    svmlvlroe:1,
    svmloe:1,
    reserved2:3;
};
#else

struct __attribute__((packed)) svsmio_t
{
  uint16_t
  reserved2:3,
    svmloe:1,
    svmlvlroe:1,
    svmoutpol:1,
    reserved1:5,
    svmhoe:1,
    svmhvlroe:1,
    reserved0:3;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) pmmifg_t
{
  uint16_t
  pmmlpm5ifg:1,
    reserved0:1,
    svslifg:1,
    svshifg:1,
    reserved1:1,
    pmmporifg:1,
    pmmrstifg:1,
    pmmborifg:1,
    reserved2:1,
    svmhvlrifg:1,
    svmhifg:1,
    svsmhdlyifg:1,
    reserved3:1,
    svmlvlrifg:1,
    svmlifg:1,
    svsmldlyifg:1;
};
#else

struct __attribute__((packed)) pmmifg_t
{
  uint16_t
  svsmldlyifg:1,
    svmlifg:1,
    svmlvlrifg:1,
    reserved3:1,
    svsmhdlyifg:1,
    svmhifg:1,
    svmhvlrifg:1,
    reserved2:1,
    pmmborifg:1,
    pmmrstifg:1,
    pmmporifg:1,
    reserved1:1,
    svshifg:1,
    svslifg:1,
    reserved0:1,
    pmmlpm5ifg:1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) pmmrie_t
{
  uint16_t
  reserved0:2,
    svmhvlrpe:1,
    svshpe:1,
    reserved1:2,
    svmlvlrpe:1,
    svslpe:1,
    reserved2:1,
    svmhvlrie:1,
    svmhie:1,
    svsmhdlyie:1,
    reserved3:1,
    svmlvlrie:1,
    svmlie:1,
    svsmldlyie:1;
};
#else

struct __attribute__((packed)) pmmrie_t
{
  uint16_t
  svsmldlyie:1,
    svmlie:1,
    svmlvlrie:1,
    reserved3:1,
    svsmhdlyie:1,
    svmhie:1,
    svmhvlrie:1,
    reserverd2:1,
    svslpe:1,
    svmlvlrpe:1,
    reserved1:2,
    svshpe:1,
    svmhvlrpe:1,
    reserved0:2;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) pm5ctl0_t
{
  uint16_t
  reserved0:15,
    locklpm5:1;
};
#else

struct __attribute__((packed)) pm5ctl0_t
{
  uint16_t
  locklpm5:1,
    reserved0:15;
};
#endif
/**
 * PMM Data Structure
 **/
struct msp430_pmm_t { 
  union {
    struct pmmctl0_t b;
    uint16_t s;
  } pmmctl0;
  union {
    struct pmmctl1_t b;
    uint16_t s;
  } pmmctl1;
  union {
    struct svsmhctl_t b;
    uint16_t s;
  } svsmhctl;
  union {
    struct svsmlctl_t b;
    uint16_t s;
  } svsmlctl;
  union {
    struct svsmio_t b;
    uint16_t s;
  } svsmio;
  union {
    struct pmmifg_t b;
    uint16_t s;
  } pmmifg;
  union {
    struct pmmrie_t b;
    uint16_t s;
  } pmmrie;
  union {
    struct pm5ctl0_t b;
    uint16_t s;
  } pm5ctl0;
  
  int unlocked;
};

void msp430_pmm_create();
int msp430_pmm_reset();
void msp430_pmm_update();
int8_t msp430_pmm_read8(uint16_t addr);
int16_t msp430_pmm_read(uint16_t addr);
void msp430_pmm_write8(uint16_t addr, int8_t val);
void msp430_pmm_write(uint16_t addr, int16_t val);

#else
#define msp430_pmm_create() do { } while (0)
#define msp430_pmm_reset() do { } while (0)
#define msp430_pmm_update() do { } while (0)
#endif
#endif

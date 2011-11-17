/**
 *  \file   msp430_ucs_clock.h
 *  \brief  MSP430x5xx / MSP430x6xx UCS clock module (based on msp430_fll_clock.c)
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_UCS_CLOCK_H
#define MSP430_UCS_CLOCK_H

#if defined(__msp430_have_ucs)

#define UCS_START     UCS_BASE
#define UCS_END       (UCS_BASE + 0x10)

#define UCSCTL0       UCS_BASE
#define UCSCTL1       (UCS_BASE + 0x02)
#define UCSCTL2       (UCS_BASE + 0x04)
#define UCSCTL3       (UCS_BASE + 0x06)
#define UCSCTL4       (UCS_BASE + 0x08)
#define UCSCTL5       (UCS_BASE + 0x0A)
#define UCSCTL6       (UCS_BASE + 0x0C)
#define UCSCTL7       (UCS_BASE + 0x0E)
#define UCSCTL8       (UCS_BASE + 0x10)

/**
 register definition
 */
#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl0_t
{
  uint16_t
  reserved : 3,
    dco : 5,
    mod : 5,
    reserved1 : 3;
};
#else

struct __attribute__((packed)) ucsctl0_t
{
  uint16_t
  reserved1 : 3,
    mod : 5,
    dco : 5,
    reserved : 3;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl1_t
{
  uint16_t
  reserved : 9,
    dcorsel : 3,
    reserved1 : 3,
    dismod : 1;
};
#else

struct __attribute__((packed)) ucsctl1_t
{
  uint16_t
  dismod : 1,
    reserved1 : 3,
    dcorsel : 3,
    reserved : 9;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl2_t
{
  uint16_t
  reserved : 1,
    flld : 3,
    reserved1 : 2,
    flln : 10;
};
#else

struct __attribute__((packed)) ucsctl2_t
{
  uint16_t
  flln : 10,
    reserved1 : 2,
    flld : 3,
    reserved : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl3_t
{
  uint16_t
  reserved : 9,
    selref : 3,
    reserved1 : 1,
    fllrefdiv : 3;
};
#else

struct __attribute__((packed)) ucsctl3_t
{
  uint16_t
  fllrefdiv : 3,
    reserved1 : 1,
    selref : 3,
    reserved : 9;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl4_t
{
  uint16_t
  reserved : 5,
    sela : 3,
    reserved1 : 1,
    sels : 3,
    reserved2 : 1,
    selm : 3;
};
#else

struct __attribute__((packed)) ucsctl4_t
{
  uint16_t
  selm : 3,
    reserved2 : 1,
    sels : 3,
    reserved1 : 1,
    sela : 3,
    reserved : 5;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl5_t
{
  uint16_t
  reserved : 1,
    divpa : 3,
    reserved1 : 1,
    diva : 3,
    reserved2 : 1,
    divs : 3,
    reserved3 : 1,
    divm : 3;
};
#else

struct __attribute__((packed)) ucsctl5_t
{
  uint16_t
  divm : 3,
    reserved3 : 1,
    divs : 3,
    reserved2 : 1,
    diva : 3,
    reserved1 : 1,
    divpa : 3,
    reserved : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl6_t
{
  uint16_t
  reserved : 7,
    xt2off : 1,
    xt1drive : 2,
    xts : 1,
    xt1bypass : 1,
    xcap : 2,
    smclkoff : 1,
    xt1off : 1;
};
#else

struct __attribute__((packed)) ucsctl6_t
{
  uint16_t
  xt1off : 1,
    smclkoff : 1,
    xcap : 2,
    xt1bypass : 1,
    xts : 1,
    xt1drive : 2,
    xt2off : 1,
    reserved : 7;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl7_t
{
  uint16_t
  reserved : 12,
    xt2offg : 1,
    xt1hfoffg : 1,
    xt1lfoffg : 1,
    dcoffg : 1;
};
#else

struct __attribute__((packed)) ucsctl7_t
{
  uint16_t
  dcoffg : 1,
    xt1lfoffg : 1,
    xt1hfoffg : 1,
    xt2offg : 1,
    reserved : 12;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) ucsctl8_t
{
  uint16_t
  reserved : 12,
    modoscreqen : 1,
    smclkreqen : 1,
    mclkreqen : 1,
    aclkreqen : 1;
};
#else

struct __attribute__((packed)) ucsctl8_t
{
  uint16_t
  aclkreqen : 1,
    mclkreqen : 1,
    smclkreqen : 1,
    modoscreqen : 1,
    reserved : 1;
};
#endif

/**
 * UCS Data Structure
 **/
struct msp430_ucs_t {

  union {
    struct ucsctl0_t b;
    uint16_t s;
  } ucsctl0;

  union {
    struct ucsctl1_t b;
    uint16_t s;
  } ucsctl1;

  union {
    struct ucsctl2_t b;
    uint16_t s;
  } ucsctl2;

  union {
    struct ucsctl3_t b;
    uint16_t s;
  } ucsctl3;

  union {
    struct ucsctl4_t b;
    uint16_t s;
  } ucsctl4;

  union {
    struct ucsctl5_t b;
    uint16_t s;
  } ucsctl5;

  union {
    struct ucsctl6_t b;
    uint16_t s;
  } ucsctl6;

  union {
    struct ucsctl7_t b;
    uint16_t s;
  } ucsctl7;

  union {
    struct ucsctl8_t b;
    uint16_t s;
  } ucsctl8;

  /* internal clocks */

  uint64_t MCLK_counter;
  int MCLK_increment;

  uint64_t ACLK_counter;
  int ACLK_temp;
  int ACLK_increment;
  uint32_t ACLK_bitmask;

  uint64_t ACLKn_counter;
  int ACLKn_temp;
  int ACLKn_increment;
  uint32_t ACLKn_bitmask;

  uint64_t SMCLK_counter;
  int SMCLK_temp;
  int SMCLK_increment;
  uint64_t SMCLK_bitmask;

  /* external clocks */

  uint32_t lfxt1_freq;
  uint32_t lfxt1_cycle_nanotime;
  uint64_t lfxt1_counter;
  int lfxt1_temp;
  int lfxt1_increment;

  uint32_t xt2_freq;
  uint32_t xt2_cycle_nanotime;
  uint64_t xt2_counter;
  int xt2_temp;
  int xt2_increment;

  uint32_t vlo_freq;
  uint32_t vlo_cycle_nanotime;
  uint64_t vlo_counter;
  int vlo_temp;
  int vlo_increment;

  uint32_t refo_freq;
  uint32_t refo_cycle_nanotime;
  uint64_t refo_counter;
  int refo_temp;
  int refo_increment;

  uint32_t dcoclk_freq;
  uint32_t dcoclk_cycle_nanotime;
  uint64_t dcoclk_counter;
  int dcoclk_temp;
  int dcoclk_increment;

};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_ucs_create();
void msp430_ucs_reset();

int msp430_ucs_update(int clock_add);
void msp430_ucs_update_done();

int16_t msp430_ucs_read(uint16_t addr);
void msp430_ucs_write(uint16_t addr, int16_t val);

#define MCU_CLOCK_SYSTEM_UPDATE(n)       msp430_ucs_update(n)
#define MCU_CLOCK_SYSTEM_UPDATE_DONE()   msp430_ucs_update_done()
#define MCU_CLOCK_SYSTEM_SPEED_TRACER()  do { } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else
#define msp430_ucs_create() do { } while (0)
#define msp430_ucs_reset() do { } while (0)
#define msp430_ucs_update() do { } while (0)
#endif
#endif /* _H_ */

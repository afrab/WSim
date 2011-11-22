/**
 *  \file   msp430_rtc.h
 *  \brief  MSP430 RTC definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_RTC_H
#define MSP430_RTC_H

#include "msp430_models.h"
#include <time.h>

#if defined(__msp430_have_rtc)

#define RTC_IOMEM_BEGIN         RTC_BASE
#define RTC_IOMEM_END           (RTC_BASE + 0x1b)

#define RTC_RTCCTL0             RTC_BASE
#define RTC_RTCCTL1             (RTC_BASE + 0x01)
#define RTC_RTCCTL2             (RTC_BASE + 0x02)
#define RTC_RTCCTL3             (RTC_BASE + 0x03)
#define RTC_RTCPS0CTL           (RTC_BASE + 0x08)
#define RTC_RTCPS1CTL           (RTC_BASE + 0x0a)
#define RTC_RTCPS0              (RTC_BASE + 0x0c)
#define RTC_RTCPS1              (RTC_BASE + 0x0d)
#define RTC_RTCIV               (RTC_BASE + 0x0e)
#define RTC_RTCSEC              (RTC_BASE + 0x10)
#define RTC_RTCMIN              (RTC_BASE + 0x11)
#define RTC_RTCHOUR             (RTC_BASE + 0x12)
#define RTC_RTCDOW              (RTC_BASE + 0x13)
#define RTC_RTCDAY              (RTC_BASE + 0x14)
#define RTC_RTCMON              (RTC_BASE + 0x15)
#define RTC_RTCYEARL            (RTC_BASE + 0x16)
#define RTC_RTCYEARH            (RTC_BASE + 0x17)
#define RTC_RTCAMIN             (RTC_BASE + 0x18)
#define RTC_RTCAHOUR            (RTC_BASE + 0x19)
#define RTC_RTCADOW             (RTC_BASE + 0x1a)
#define RTC_RTCADAY             (RTC_BASE + 0x1b)

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcctl0_t
{
  uint8_t
  reserved : 1,
    rtctevie : 1,
    rtcaie : 1,
    rtcrdyie : 1,
    reserved1 : 1,
    rtctevifg : 1,
    rtcaifg : 1,
    rtcrdyifg : 1;

};
#else

struct __attribute__((packed)) rtcctl0_t
{
  uint8_t
  rtcrdyifg : 1,
    rtcaifg : 1,
    rtctevifg : 1,
    reserved1 : 1,
    rtcrdyie : 1,
    rtcaie : 1,
    rtctevie : 1,
    reserved : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcctl1_t
{
  uint8_t
  rtcbcd : 1,
    rtchold : 1,
    rtcmode : 1,
    rtcrdy : 1,
    rtcssel : 2,
    rtctev : 2;
};
#else

struct __attribute__((packed)) rtcctl1_t
{
  uint8_t
  rtcev : 2,
    rtcssel : 2,
    rtcrdy : 1,
    rtcmode : 1,
    rtchold : 1,
    rtcbcd : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcctl2_t
{
  uint8_t
  rtccal : 6,
    reserved : 1,
    rtccals : 1;
};
#else

struct __attribute__((packed)) rtcctl2_t
{
  uint8_t
  rtccals : 1,
    reserved : 1,
    rtccal : 6;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcctl3_t
{
  uint8_t
  rtccalf : 2,
    reserved : 6;
};
#else

struct __attribute__((packed)) rtcctl3_t
{
  uint8_t
  reserved : 6,
    rtccalf : 2;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcps0ctl_t
{
  uint16_t
  rt0psifg : 1,
    rt0psie : 1,
    rt0ip : 3,
    reserved2 : 3,
    rt0pshold : 1,
    reserved1 : 2,
    rt0psdiv : 3,
    rt0ssel : 1,
    reserved : 1;
};
#else

struct __attribute__((packed)) rtcps0ctl_t
{
  uint16_t
  reserved : 1,
    rt0ssel : 1,
    rt0psdiv : 3,
    reserved1 : 2,
    rt0pshold : 1,
    reserved2 : 3,
    rt0ip : 3,
    rt0psie : 1,
    rt0psifg : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtcps1ctl_t
{
  uint16_t
  rt1psifg : 1,
    rt1psie : 1,
    rt1op : 3,
    reserved2 : 3,
    rt1pshold : 1,
    reserved1 : 2,
    rt1psdiv : 3,
    rt1ssel : 1,
    reserved : 1;

};
#else

struct __attribute__((packed)) rtcps1ctl_t
{
  uint16_t
  reserved : 1,
    rt1ssel : 1,
    rt1psdiv : 3,
    reserved1 : 2,
    rt1pshold : 1,
    reserved2 : 3,
    rt1ip : 3,
    rt1psie : 1,
    rt1psifg : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) rtciv_t
{
  uint16_t
  reserved1 : 1,
    rtciv : 4,
    reserved : 11;

};
#else

struct __attribute__((packed)) rtciv_t
{
  uint16_t
  reserved : 11,
    rtciv : 4,
    reserved1 : 1;
};
#endif

/**
 * RTC Data Structure
 **/
struct msp430_rtc_t {

  union {
    struct rtcctl0_t b;
    uint8_t s;
  } rtcctl0;

  union {
    struct rtcctl1_t b;
    uint8_t s;
  } rtcctl1;

  union {
    struct rtcctl2_t b;
    uint8_t s;
  } rtcctl2;

  union {
    struct rtcctl3_t b;
    uint8_t s;
  } rtcctl3;

  union {
    struct rtcps0ctl_t b;
    uint16_t s;
  } rtcps0ctl;

  union {
    struct rtcps1ctl_t b;
    uint16_t s;
  } rtcps1ctl;

  union {
    struct rtciv_t b;
    uint16_t s;
  } rtciv;
  uint8_t rtcps0;
  uint8_t rtcps1;
  uint8_t rtcamin;
  uint8_t rtcahour;
  uint8_t rtcadow;
  uint8_t rtcaday;

  time_t rtctime;
  time_t lasttime;
};

void msp430_rtc_create();
int msp430_rtc_reset();
void msp430_rtc_update();
int8_t msp430_rtc_read8(uint16_t addr);
int16_t msp430_rtc_read(uint16_t addr);
void msp430_rtc_write8(uint16_t addr, int8_t val);
void msp430_rtc_write(uint16_t addr, int16_t val);

#else
#define msp430_rtc_create() do { } while (0)
#define msp430_rtc_reset() do { } while (0)
#define msp430_rtc_update() do { } while (0)
#endif
#endif

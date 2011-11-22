/**
 *  \file   msp430_rtc.c
 *  \brief  MSP430 RTC definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <time.h>

#include "arch/common/hardware.h"
#include "msp430.h"


#if defined(__msp430_have_rtc)
/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_rtc_create()
{
  msp430_io_register_range8(RTC_IOMEM_BEGIN, RTC_IOMEM_END, msp430_rtc_read8, msp430_rtc_write8);
  msp430_io_register_range16(RTC_IOMEM_BEGIN, RTC_IOMEM_END, msp430_rtc_read, msp430_rtc_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_rtc_reset()
{
  MCU.rtc.rtcctl0.s = 0x40;
  MCU.rtc.rtcctl1.s = 0x00;
  MCU.rtc.rtcctl2.s = 0x00;
  MCU.rtc.rtcctl3.s = 0x00;
  MCU.rtc.rtcps0ctl.s = 0x0100;
  MCU.rtc.rtcps1ctl.s = 0x0100;
  MCU.rtc.rtciv.s = 0x0000;
  MCU.rtc.rtctime = time(NULL); //get current timestamp
  MCU.rtc.lasttime = MCU.rtc.rtctime;
  return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_rtc_update()
{
  //count up seconds since last update
  time_t curtime;
  curtime = time(NULL);
  MCU.rtc.rtctime += (curtime - MCU.rtc.lasttime);
  if (MCU.rtc.lasttime != curtime) {
    MCU.rtc.rtcctl1.b.rtcrdy = 1;
  }
  MCU.rtc.lasttime = curtime;

  MCU.rtc.rtcctl1.b.rtcrdy = 1;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t msp430_rtc_read(uint16_t addr)
{
  uint16_t res;
  struct tm *rtctime_tm = localtime(&MCU.rtc.rtctime);
  if (addr == RTC_RTCCTL0) {
    res = (MCU.rtc.rtcctl1.s << 8) | MCU.rtc.rtcctl0.s;
  } else if (addr == RTC_RTCYEARL) {
    res = rtctime_tm->tm_year + 1900;
  } else {
    ERROR("msp430:rtc: bad word read address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_rtc_read8(uint16_t addr)
{
  int8_t res;
  struct tm *rtctime_tm = localtime(&MCU.rtc.rtctime);
  if (addr == RTC_RTCSEC) {
    res = rtctime_tm->tm_sec;
  } else if (addr == RTC_RTCMIN) {
    res = rtctime_tm->tm_min;
  } else if (addr == RTC_RTCHOUR) {
    res = rtctime_tm->tm_hour;
  } else if (addr == RTC_RTCDAY) {
    res = rtctime_tm->tm_mday;
  } else if (addr == RTC_RTCMON) {
    res = rtctime_tm->tm_mon + 1;
  } else {
    ERROR("msp430:rtc: bad byte read address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_rtc_write(uint16_t addr, int16_t val)
{
  if (addr == RTC_RTCCTL0) {
    MCU.rtc.rtcctl0.s = (val & 0x00ff);
    MCU.rtc.rtcctl1.s = (val & 0xff00) >> 8;
  } else if (addr == RTC_RTCYEARL) {
    struct tm *rtctime_tm = localtime(&MCU.rtc.rtctime);
    rtctime_tm->tm_year = val - 1900;
    MCU.rtc.rtctime = mktime(rtctime_tm);
  } else {
    ERROR("msp430:rtc: bad write address 0x%04x = 0x%04x\n", addr, val & 0xffff);
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_rtc_write8(uint16_t addr, int8_t val)
{
  struct tm *rtctime_tm = localtime(&MCU.rtc.rtctime);
  if (addr == RTC_RTCSEC) {
    rtctime_tm->tm_sec = val;
  } else if (addr == RTC_RTCMIN) {
    rtctime_tm->tm_min = val;
  } else if (addr == RTC_RTCHOUR) {
    rtctime_tm->tm_hour = val;
  } else if (addr == RTC_RTCDAY) {
    rtctime_tm->tm_mday = val;
  } else if (addr == RTC_RTCMON) {
    rtctime_tm->tm_mon = val - 1;
  } else {
    ERROR("msp430:rtc: bad write address 0x%04x = 0x%02x\n", addr, val & 0xff);
  }
  MCU.rtc.rtctime = mktime(rtctime_tm);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // _have_rtc

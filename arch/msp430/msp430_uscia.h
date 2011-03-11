
/**
 *  \file   msp430_uscia.h
 *  \brief  MSP430 USCIA definition (based on "msp430_usart.h" UART MODE)
 *  \author Julien Carpentier
 *  \date   2011
 **/

#ifndef MSP430_USCIA_H
#define MSP430_USCIA_H

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxctl0_t {
  uint8_t
    ucpen:1,
    ucpar:1,    
    ucmsb:1,    
    uc7bit:1,  
    ucspb:1, 
    ucmode:2,   
    ucsync:1;   
};
#else
struct __attribute__ ((packed)) ucaxctl0_t {
  uint8_t
    ucsync:1,
    ucmode:2,
    ucspb:1,
    uc7bit:1,
    ucmsb:1,
    ucpar:1,
    ucpen:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxctl1_t {
  uint8_t
    ucssel:2,
    ucrxeie:1,
    ucbrkie:1,  
    ucdorm:1,  
    uctxaddr:1, 
    uctxbrk:1,   
    ucswrst:1;  
};
#else
struct __attribute__ ((packed)) ucaxctl1_t {
  uint8_t
    ucswrst:1,
    uctxbrk:1, 
    uctxaddr:1,
    ucdorm:1,
    ucbrkie:1, 
    ucrxeie:1,
    ucssel:2;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxmctl_t {
  uint8_t
    ucbrf:4,
    ucbrs:3,
    ucos16:1;
};
#else
struct __attribute__ ((packed)) ucaxmctl_t {
  uint8_t
    ucos16:1,
    ucbrs:3,
    ucbrf:4;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxstat_t {
  uint8_t
    uclisten:1,
    ucfe:1, 
    ucoe:1,
    ucpe:1,
    ucbrk:1, 
    ucrxerr:1,
    ucaddr:1,
    ucbusy:1;
};
#else
struct __attribute__ ((packed)) ucaxstat_t {
  uint8_t
    ucbusy:1,
    ucaddr:1,
    ucrxerr:1,
    ucbrk:1,
    ucpe:1,
    ucoe:1,
    ucfe:1,
    uclisten:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxirtctl_t {
  uint8_t
    ucirtxpl:6,
    ucirtxclk:1,
    uciren:1;
};
#else
struct __attribute__ ((packed)) ucaxirtctl_t {
  uint8_t
    uciren:1,
    ucirtxclk:1,
    ucirtxpl:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxirrctl_t {
  uint8_t
    ucirrxfl:6,
    ucirrxpl:1,
    ucirrxfe:1;
};
#else
struct __attribute__ ((packed)) ucaxirrctl_t {
  uint8_t
    ucirrxfe:1,
    ucirrxpl:1,
    ucirrxfl:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucaxabctl_t {
  uint8_t
    reserved0:2,
    ucdelim:2,
    ucstoe:1,
    ucbtoe:1,
    reserved1:1,
    ucabden:1;
};
#else
struct __attribute__ ((packed)) ucaxabctl_t {
  uint8_t
    ucabden:1,
    reserved1:1,
    ucbtoe:1,
    ucstoe:1,
    ucdelim:2,
    reserved0:2;
};
#endif



  
struct msp430_uscia_t
{
  union {
    struct ucaxctl0_t    b;
    int8_t               s;
  } ucaxctl0;
  union {
    struct ucaxctl1_t    b;
    int8_t               s;
  } ucaxctl1;
  union {
    struct ucaxmctl_t    b;
    int8_t               s;
  } ucaxmctl;
  union {
    struct ucaxstat_t    b;
    int8_t               s;
  } ucaxstat;
  union {
    struct ucaxabctl_t   b;
    int8_t               s;
  } ucaxabctl;
  union {
    struct ucaxirtctl_t  b;
    int8_t               s;
  } ucaxirtctl;
  union {
    struct ucaxirrctl_t  b;
    int8_t               s;
  } ucaxirrctl;

  uint8_t   ucaxbr0;
  uint8_t   ucaxbr1;
  uint8_t   ucaxrxbuf;
  uint8_t   ucaxtxbuf;
  uint32_t  ucaxbr_div;
  
  uint8_t   ucaxrxbuf_full;
  uint8_t   ucaxrx_shift_buf;
  uint8_t   ucaxrx_shift_empty;
  uint8_t   ucaxrx_shift_ready;
  int32_t   ucaxrx_shift_delay;
  uint8_t   ucaxrx_slave_rx_done;

  uint8_t   ucaxtxbuf_full;
  uint8_t   ucaxtx_shift_buf;
  uint8_t   ucaxtx_shift_empty;
  uint8_t   ucaxtx_shift_ready;
  int32_t   ucaxtx_shift_delay;
  int32_t   ucaxtx_full_delay;  /* delay between tx_buff and shifter */
  
};





/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_uscia0)
#define USCIA0_START  0x060
#define USCIA0_END    0x067

#define UCA0CTL0      0x060
#define UCA0CTL1      0x061
#define UCA0BR0       0x062
#define UCA0BR1       0x063
#define UCA0MCTL      0x064
#define UCA0STAT      0x065
#define UCA0RXBUF     0x056
#define UCA0TXBUF     0x067
#define UCA0ABCTL     0x05D
#define UCA0IRTCTL    0x05E
#define UCA0IRRCTL    0x05F


void   msp430_uscia0_reset();
void   msp430_uscia0_update();
int8_t msp430_uscia0_read (uint16_t addr);
void   msp430_uscia0_write(uint16_t addr, int8_t val);
int    msp430_uscia0_chkifg();


int    msp430_uscia0_dev_read_uart      (uint8_t *val);
void   msp430_uscia0_dev_write_uart     (uint8_t val);
int    msp430_uscia0_dev_write_uart_ok  ();


#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


